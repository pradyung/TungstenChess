#include "bot/engine.hpp"

#include <iostream>
#include <algorithm>

#include "bot/piece_eval_tables.hpp"

namespace TungstenChess
{
  Move Bot::generateBotMove(int maxSearchTime)
  {
    if (m_botSettings.useOpeningBook && m_openingBookLoaded.peek() && m_openingBook.updateMoveHistory(m_board.moveHistory()))
    {
      Move moveInt = m_openingBook.getNextMove();

      if (moveInt != NULL_MOVE)
      {
        Move bestMove = moveInt & FROM_TO;

        if (m_botSettings.logSearchInfo)
          std::cout << "Book: " << (m_botSettings.logPGNMoves ? m_board.getMovePGN(bestMove) : Moves::getUCI(bestMove)) << std::endl;

        return bestMove;
      }
    }

    m_previousSearchInfo = SearchInfo();

    auto start = std::chrono::high_resolution_clock::now();

    Move bestMove = iterativeDeepening(maxSearchTime == -1 ? m_botSettings.maxSearchTime : maxSearchTime);

    if (m_botSettings.logSearchInfo)
    {
      std::string evalString;
      if (m_previousSearchInfo.lossFound)
        evalString = std::format("Loss in {}", m_previousSearchInfo.mateIn);
      else if (m_previousSearchInfo.mateFound)
        evalString = m_previousSearchInfo.mateIn == 0 ? "Mate" : std::format("Mate in {}", m_previousSearchInfo.mateIn);
      else
        evalString = std::to_string(m_previousSearchInfo.evaluation * (m_board.sideToMove() == WHITE ? 1 : -1));

      std::cout << std::format(
                       "Move: {:<9} Depth: {:<12} Time: {:<11} Positions evaluated: {:<11} Transpositions used: {:<9} Occupied: {:<14} Evaluation: {}",
                       m_botSettings.logPGNMoves ? m_board.getMovePGN(bestMove) : Moves::getUCI(bestMove),
                       std::format("{} + {}/{}", m_previousSearchInfo.depthSearched, m_previousSearchInfo.nextDepthNumMovesSearched, m_previousSearchInfo.nextDepthTotalMoves),
                       std::format("{} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count()),
                       m_previousSearchInfo.positionsEvaluated,
                       m_previousSearchInfo.transpositionsUsed,
                       m_transpositionTable.occupancy(),
                       evalString)
                << std::endl;
    }

    return bestMove;
  }

  Move Bot::generateBestMove(int depth, Move bestMoveSoFar)
  {
    MoveArray legalMoves;
    int legalMovesCount = getSortedLegalMoves(legalMoves, m_board.sideToMove(), false, bestMoveSoFar);

    if (legalMovesCount == 0)
      return NULL_MOVE;

    Move bestMove = legalMoves[0];

    int alpha = NEGATIVE_INFINITY;
    int numMovesSearched = 0;

    m_previousSearchInfo.nextDepthNumMovesSearched = 0;
    m_previousSearchInfo.nextDepthTotalMoves = legalMovesCount;

    for (Move &move : legalMoves)
    {
      if (move == NULL_MOVE)
        break;

      Board::UnmoveData unmoveData = m_board.makeMove(move);
      int evaluation = -negamax(depth - 1, NEGATIVE_INFINITY, -alpha, false);
      m_board.unmakeMove(move, unmoveData);

      if (m_searchCancelled)
      {
        if (numMovesSearched > 0)
          break;
        else
          return NULL_MOVE;
      }

      numMovesSearched++;

      if (evaluation > alpha)
      {
        alpha = evaluation;
        bestMove = move;

        if (alpha >= POSITIVE_INFINITY)
        {
          m_previousSearchInfo.mateFound = true;
          break;
        }
      }
    }

    if (!m_searchCancelled && !m_previousSearchInfo.mateFound)
    {
      m_previousSearchInfo.evaluation = alpha;
      m_previousSearchInfo.depthSearched = depth;
    }

    m_previousSearchInfo.nextDepthNumMovesSearched = numMovesSearched;

    return bestMove;
  }

  Move Bot::iterativeDeepening(int time)
  {
    m_searchCancelled = false;

    int depth = 1;

    m_maxSearchTime = time;
    m_searchTimerReset = true;
    m_searchTimerEvent.notify_one();

    m_currentSearchId++;

    Move bestMove = generateBestMove(depth);

    while (!m_searchCancelled)
    {
      depth++;

      Move newMove = generateBestMove(depth, bestMove);

      if (newMove == NULL_MOVE)
        break;

      bestMove = newMove;

      if (m_previousSearchInfo.mateFound)
      {
        m_previousSearchInfo.mateIn = (depth - 1) / 2;
        break;
      }

      if (m_previousSearchInfo.evaluation <= NEGATIVE_INFINITY)
      {
        m_previousSearchInfo.lossFound = true;
        m_previousSearchInfo.mateIn = (depth - 1) / 2;
        break;
      }
    }

    return bestMove;
  }

  void Bot::heuristicSortMoves(MoveArray &moves, int movesCount, Move bestMove)
  {
    std::sort(moves.begin(), moves.begin() + movesCount,
              [this, bestMove](Move a, Move b)
              { return heuristicEvaluation(a, bestMove) > heuristicEvaluation(b, bestMove); });
  }

  int Bot::heuristicEvaluation(Move move, Move bestMove)
  {
    if (move == bestMove)
      return POSITIVE_INFINITY;

    int evaluation = 0;

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    PieceType promotionPieceType = move >> 12;

    evaluation += PIECE_VALUES[m_board[to] & TYPE];
    evaluation += PIECE_VALUES[promotionPieceType];

    evaluation += getPiecePositionalEvaluation(to, true) - getPiecePositionalEvaluation(from, true);

    return evaluation;
  }

  int Bot::getSortedLegalMoves(MoveArray &moves, PieceColor color, bool onlyCaptures, Move bestMove)
  {
    int legalMovesCount = m_board.getLegalMoves(moves, color, onlyCaptures);
    heuristicSortMoves(moves, legalMovesCount, bestMove);
    return legalMovesCount;
  }
}