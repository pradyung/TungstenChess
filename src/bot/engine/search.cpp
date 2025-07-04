#include "bot/engine.hpp"

#include <iostream>
#include <algorithm>

#include "bot/piece_eval_tables.hpp"

namespace TungstenChess
{
  void Bot::startSearchTimerThread()
  {
    m_searchTimerThread = std::thread(
        [this]()
        {
          std::unique_lock<std::mutex> lock(m_searchTimerMutex);
          m_searchTimerEvent.wait(lock);
          while (!m_searchTimerTerminated)
          {
            m_searchTimerReset = false;

            if (m_searchTimerEvent.wait_for(lock, std::chrono::milliseconds(m_maxSearchTime), [this]
                                            { return m_searchTimerReset.load(); }))
              continue;

            m_searchCancelled = true;
          }
        });

    m_searchTimerThread.detach();
  }

  void Bot::stopSearchTimerThread()
  {
    m_searchCancelled = true;
    m_searchTimerTerminated = true;
    m_searchTimerReset = true;
    if (m_searchTimerThread.joinable())
      m_searchTimerThread.join();
  }

  Move Bot::generateBotMove(int maxSearchTime)
  {
    if (m_botSettings.useOpeningBook &&
        m_onceOpeningBookLoaded.peek() &&
        m_openingBook.updateMoveHistory(m_board.moveHistory()))
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

    Move bestMove = iterativeDeepeningSearch(maxSearchTime == -1 ? m_botSettings.maxSearchTime : maxSearchTime);

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
                       "Move: {:<9} Time: {:>6} ms    Depth: {:<12} Positions evaluated: {:>9}   Transpositions used: {:>7}   Occupied: {:>12}   Evaluation: {}",
                       m_botSettings.logPGNMoves ? m_board.getMovePGN(bestMove) : Moves::getUCI(bestMove),
                       std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count(),
                       std::format("{} + {}/{}", m_previousSearchInfo.depthSearched, m_previousSearchInfo.nextDepthNumMovesSearched, m_previousSearchInfo.nextDepthTotalMoves),
                       m_previousSearchInfo.positionsEvaluated,
                       m_previousSearchInfo.transpositionsUsed,
                       m_transpositionTable.occupancy(),
                       evalString)
                << std::endl;
    }

    return bestMove;
  }

  int Bot::negamax(int depth, int alpha, int beta, bool quiesce)
  {
    if (m_searchCancelled)
      return 0;

    bool found;
    const TranspositionTable::Entry &entry = m_transpositionTable.retrieve(m_board.zobristKey(), found);

    if (found && entry.quiesce() == quiesce && entry.depth() >= depth)
    {
      bool isTerminal = abs(entry.evaluation()) == INF_EVAL;

      // Ignore transposition table entry if it is a terminal position evaluated in a previous search
      // to avoid premature mate detection
      if (!(isTerminal && entry.searchId() < m_currentSearchId && entry.depth() > depth))
      {
        m_previousSearchInfo.transpositionsUsed++;
        return entry.evaluation();
      }
    }

    int standPat;

    if (quiesce)
    {
      standPat = getStaticEvaluation();

      if (depth == 0)
        return standPat;

      if (standPat > alpha)
        alpha = standPat;

      if (alpha >= beta)
        return beta;
    }
    else
    {
      if (depth == 0)
        return negamax(m_botSettings.quiesceDepth, alpha, beta, true);
    }

    if (m_board.countRepetitions(m_board.zobristKey()) >= 3 || m_board.halfmoveClock() >= 100)
      return -CONTEMPT;

    MoveAllocation legalMoves(m_moveStack);
    int legalMovesCount = getSortedLegalMoves(legalMoves, quiesce);

    if (legalMovesCount == 0)
    {
      bool isStalemate = !m_board.isInCheck(m_board.sideToMove());
      if (isStalemate)
        return -CONTEMPT;
      else
        return -INF_EVAL + (quiesce ? (INF_EVAL / 10) : 0);
    }

    if (legalMovesCount == 1)
      depth++;

    for (Move &move : legalMoves)
    {
      Board::UnmoveData unmoveData = m_board.makeMove(move);
      int evaluation = -negamax(depth - 1, -beta, -alpha, quiesce);
      m_board.unmakeMove(move, unmoveData);

      if (m_searchCancelled)
        return 0;

      if (evaluation > alpha)
      {
        alpha = evaluation;

        if (alpha >= beta)
          return beta;

        if (!quiesce && alpha >= INF_EVAL)
          break;
      }
    }

    if (!m_searchCancelled)
    {
      m_transpositionTable.store(m_board.zobristKey(), m_currentSearchId, alpha, depth, quiesce);
    }

    return alpha;
  }

  Move Bot::generateBestMove(int depth, Move bestMoveSoFar)
  {
    MoveAllocation legalMoves(m_moveStack);
    int legalMovesCount = getSortedLegalMoves(legalMoves, false, bestMoveSoFar);

    if (legalMovesCount == 0)
      return NULL_MOVE;

    Move bestMove = legalMoves[0];

    int alpha = -INF_EVAL;
    int numMovesSearched = 0;

    m_previousSearchInfo.nextDepthNumMovesSearched = 0;
    m_previousSearchInfo.nextDepthTotalMoves = legalMovesCount;

    for (Move &move : legalMoves)
    {
      Board::UnmoveData unmoveData = m_board.makeMove(move);
      int evaluation = -negamax(depth - 1, -INF_EVAL, -alpha, false);
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

        if (alpha >= INF_EVAL)
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

  Move Bot::iterativeDeepeningSearch(int time)
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

      if (m_previousSearchInfo.evaluation <= -INF_EVAL)
      {
        m_previousSearchInfo.lossFound = true;
        m_previousSearchInfo.mateIn = (depth - 1) / 2;
        break;
      }
    }

    return bestMove;
  }

  void Bot::heuristicSortMoves(MoveAllocation &moves, Move bestMove)
  {
    std::sort(moves.begin(), moves.end(),
              [this, bestMove](Move a, Move b)
              { return heuristicEvaluation(a, bestMove) > heuristicEvaluation(b, bestMove); });
  }

  int Bot::heuristicEvaluation(Move move, Move bestMove)
  {
    if (move == bestMove)
      return INF_EVAL;

    int evaluation = 0;

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    PieceType promotionPieceType = move >> 12;

    evaluation += PIECE_VALUES[m_board[to] & TYPE];
    evaluation += PIECE_VALUES[promotionPieceType];

    evaluation += getPiecePositionalEvaluation(to, true) - getPiecePositionalEvaluation(from, true);

    return evaluation;
  }

  int Bot::getSortedLegalMoves(MoveAllocation &moves, bool onlyCaptures, Move bestMove)
  {
    int legalMovesCount = m_board.getLegalMoves(moves, onlyCaptures);
    heuristicSortMoves(moves, bestMove);
    return legalMovesCount;
  }
}