#include "bot.hpp"

namespace TungstenChess
{
  Move Bot::generateBotMove()
  {
    if (m_botSettings.useOpeningBook && m_openingBookLoaded && m_openingBook.updateMoveHistory(m_board.moveHistory()))
    {
      Move moveInt = m_openingBook.getNextMove();

      if (moveInt != NULL_MOVE)
      {
        Move bestMove = moveInt & BASE;

        if (m_botSettings.logSearchInfo)
          std::cout << "Book: " << (m_botSettings.logPGNMoves ? m_board.getMovePGN(bestMove) : Moves::getUCI(bestMove)) << std::endl;

        return bestMove;
      }
    }

    m_previousSearchInfo.positionsEvaluated = 0;

    auto start = std::chrono::high_resolution_clock::now();

    Move bestMove = m_botSettings.fixedDepthSearch ? generateBestMove(m_botSettings.maxSearchDepth) : iterativeDeepening(m_botSettings.maxSearchTime);

    if (m_botSettings.logSearchInfo)
      std::cout << "Move: " << (m_botSettings.logPGNMoves ? m_board.getMovePGN(bestMove) : Moves::getUCI(bestMove)) << "\t"
                << "Depth: " << m_previousSearchInfo.depthSearched << "\t"
                << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << " ms\t"
                << "Positions evaluated: " << m_previousSearchInfo.positionsEvaluated << "\t"
                << "Evaluation: " << (m_previousSearchInfo.evaluation * (m_board.sideToMove() == WHITE ? 1 : -1)) << std::endl;

    return bestMove;
  }

  int Bot::getStaticEvaluation()
  {
    m_previousSearchInfo.positionsEvaluated++;

    GameStatus gameStatus = m_board.getGameStatus(m_board.sideToMove());

    if (gameStatus != NO_MATE)
    {
      if (gameStatus == LOSE)
        return NEGATIVE_INFINITY;
      else
        return -STALEMATE_PENALTY;
    }

    int staticEvaluation = getMaterialEvaluation() + getPositionalEvaluation() + getEvaluationBonus();

    return m_board.sideToMove() == WHITE ? staticEvaluation : -staticEvaluation;
  }

  int Bot::getMaterialEvaluation() const
  {
    int materialEvaluation = 0;

    materialEvaluation += Bitboards::countBits(m_board.bitboard(WHITE_PAWN)) * PIECE_VALUES[PAWN];
    materialEvaluation += Bitboards::countBits(m_board.bitboard(WHITE_KNIGHT)) * PIECE_VALUES[KNIGHT];
    materialEvaluation += Bitboards::countBits(m_board.bitboard(WHITE_BISHOP)) * PIECE_VALUES[BISHOP];
    materialEvaluation += Bitboards::countBits(m_board.bitboard(WHITE_ROOK)) * PIECE_VALUES[ROOK];
    materialEvaluation += Bitboards::countBits(m_board.bitboard(WHITE_QUEEN)) * PIECE_VALUES[QUEEN];

    materialEvaluation -= Bitboards::countBits(m_board.bitboard(BLACK_PAWN)) * PIECE_VALUES[PAWN];
    materialEvaluation -= Bitboards::countBits(m_board.bitboard(BLACK_KNIGHT)) * PIECE_VALUES[KNIGHT];
    materialEvaluation -= Bitboards::countBits(m_board.bitboard(BLACK_BISHOP)) * PIECE_VALUES[BISHOP];
    materialEvaluation -= Bitboards::countBits(m_board.bitboard(BLACK_ROOK)) * PIECE_VALUES[ROOK];
    materialEvaluation -= Bitboards::countBits(m_board.bitboard(BLACK_QUEEN)) * PIECE_VALUES[QUEEN];

    return materialEvaluation;
  }

  int Bot::getPositionalEvaluation() const
  {
    int positionalEvaluation = 0;

    Bitboard whitePieces = m_board.bitboard(WHITE_KNIGHT) | m_board.bitboard(WHITE_BISHOP) | m_board.bitboard(WHITE_ROOK) | m_board.bitboard(WHITE_QUEEN);
    Bitboard blackPieces = m_board.bitboard(BLACK_KNIGHT) | m_board.bitboard(BLACK_BISHOP) | m_board.bitboard(BLACK_ROOK) | m_board.bitboard(BLACK_QUEEN);

    Bitboard allPieces = whitePieces | m_board.bitboard(WHITE_PAWN) | blackPieces | m_board.bitboard(BLACK_PAWN);

    while (allPieces)
    {
      Square pieceIndex = Bitboards::popBit(allPieces);

      positionalEvaluation += getPiecePositionalEvaluation(pieceIndex);
    }

    {
      Square whiteKingIndex = m_board.kingIndex(WHITE_KING);

      Bitboard enemyPieces = blackPieces | m_board.bitboard(BLACK_PAWN);
      Bitboard friendlyPieces = whitePieces;

      float endgameScore = Bitboards::countBits(enemyPieces) / 16.0;

      positionalEvaluation += KING_EVAL_TABLE[whiteKingIndex] * endgameScore;

      positionalEvaluation += KING_ENDGAME_EVAL_TABLE[whiteKingIndex] * (1 - endgameScore);

      if (Bitboards::countBits(friendlyPieces) <= 3 && Bitboards::countBits(friendlyPieces) >= 1)
      {
        int kingsDistance = abs(whiteKingIndex % 8 - m_board.kingIndex(BLACK_KING) % 8) + abs(whiteKingIndex / 8 - m_board.kingIndex(BLACK_KING) / 8);

        positionalEvaluation += KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
      }
    }

    {
      Square blackKingIndex = m_board.kingIndex(BLACK_KING);

      Bitboard enemyPieces = whitePieces | m_board.bitboard(WHITE_PAWN);
      Bitboard friendlyPieces = blackPieces;

      float endgameScore = Bitboards::countBits(enemyPieces) / 16.0;

      positionalEvaluation -= KING_EVAL_TABLE[63 - blackKingIndex] * endgameScore;

      positionalEvaluation -= KING_ENDGAME_EVAL_TABLE[63 - blackKingIndex] * (1 - endgameScore);

      if (Bitboards::countBits(friendlyPieces) <= 3 && Bitboards::countBits(friendlyPieces) >= 1)
      {
        int kingsDistance = abs(blackKingIndex % 8 - m_board.kingIndex(WHITE_KING) % 8) + abs(blackKingIndex / 8 - m_board.kingIndex(WHITE_KING) / 8);

        positionalEvaluation -= KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
      }
    }

    return positionalEvaluation;
  }

  int Bot::getEvaluationBonus() const
  {
    int evaluationBonus = 0;

    if (Bitboards::countBits(m_board.bitboard(WHITE_BISHOP)) >= 2)
      evaluationBonus += BISHOP_PAIR_BONUS;
    if (Bitboards::countBits(m_board.bitboard(BLACK_BISHOP)) >= 2)
      evaluationBonus -= BISHOP_PAIR_BONUS;

    if (m_board.castlingRights() & WHITE_KINGSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (m_board.castlingRights() & BLACK_KINGSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;
    if (m_board.castlingRights() & WHITE_QUEENSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (m_board.castlingRights() & BLACK_QUEENSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;

    if (m_board.hasCastled() & WHITE)
      evaluationBonus += CASTLED_KING_BONUS;
    if (m_board.hasCastled() & BLACK)
      evaluationBonus -= CASTLED_KING_BONUS;

    for (Square i = 0; i < 64; i++)
    {
      Square file = i % 8;
      Square rank = i / 8;

      if (rank == 0)
      {
        if (Bitboards::countBits(Bitboards::file(m_board.bitboard(WHITE_PAWN), file)) > 1)
          evaluationBonus -= DOUBLED_PAWN_PENALTY;
        if (Bitboards::countBits(Bitboards::file(m_board.bitboard(BLACK_PAWN), file)) > 1)
          evaluationBonus += DOUBLED_PAWN_PENALTY;

        if (Bitboards::file(m_board.bitboard(WHITE_PAWN), file))
        {
          if (!Bitboards::file(m_board.bitboard(BLACK_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(BLACK_PAWN), file + 1))
            evaluationBonus += PASSED_PAWN_BONUS;
          if (!Bitboards::file(m_board.bitboard(WHITE_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(WHITE_PAWN), file + 1))
            evaluationBonus -= ISOLATED_PAWN_PENALTY;
        }
        if (Bitboards::file(m_board.bitboard(BLACK_PAWN), file))
        {
          if (!Bitboards::file(m_board.bitboard(WHITE_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(WHITE_PAWN), file + 1))
            evaluationBonus -= PASSED_PAWN_BONUS;
          if (!Bitboards::file(m_board.bitboard(BLACK_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(BLACK_PAWN), file + 1))
            evaluationBonus += ISOLATED_PAWN_PENALTY;
        }
      }

      if (!m_board[i])
        continue;

      if (m_board[i] == WHITE_ROOK)
      {
        Bitboard pawns = m_board.bitboard(WHITE_PAWN) | m_board.bitboard(BLACK_PAWN);

        if (!Bitboards::file(pawns, file))
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (!Bitboards::file(m_board.bitboard(BLACK_PAWN), file))
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (m_board[i] == BLACK_ROOK)
      {
        Bitboard pawns = m_board.bitboard(WHITE_PAWN) | m_board.bitboard(BLACK_PAWN);

        if (!Bitboards::file(pawns, file))
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (!Bitboards::file(m_board.bitboard(WHITE_PAWN), file))
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (m_board[i] == WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && !Bitboards::file(m_board.bitboard(BLACK_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(BLACK_PAWN), file + 1))
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (m_board[i] == BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && !Bitboards::file(m_board.bitboard(WHITE_PAWN), file - 1) && !Bitboards::file(m_board.bitboard(WHITE_PAWN), file + 1))
          evaluationBonus -= KNIGHT_OUTPOST_BONUS;
        continue;
      }
    }

    return evaluationBonus;
  }

  int Bot::negamax(int depth, int alpha, int beta, bool quiesce)
  {
    if (m_searchCancelled)
      return 0;

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
      return -STALEMATE_PENALTY;

    std::vector<Move> legalMoves = getSortedLegalMoves(m_board.sideToMove(), quiesce);

    if (legalMoves.empty())
      return quiesce ? standPat : (m_board.isInCheck(m_board.sideToMove()) ? NEGATIVE_INFINITY : -STALEMATE_PENALTY);

    if (legalMoves.size() == 1)
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

        if (!quiesce && alpha >= POSITIVE_INFINITY)
          break;
      }
    }

    return alpha;
  }

  Move Bot::generateBestMove(int depth)
  {
    if (m_searchCancelled)
      return NULL_MOVE;

    std::vector<Move> legalMoves = getSortedLegalMoves(m_board.sideToMove());

    Move &bestMove = legalMoves[0];
    int alpha = NEGATIVE_INFINITY;

    for (Move &move : legalMoves)
    {
      Board::UnmoveData unmoveData = m_board.makeMove(move);
      int evaluation = -negamax(depth - 1, NEGATIVE_INFINITY, -alpha, false);
      m_board.unmakeMove(move, unmoveData);

      if (m_searchCancelled)
        return NULL_MOVE;

      if (evaluation > alpha)
      {
        alpha = evaluation;
        bestMove = move;

        if (alpha >= POSITIVE_INFINITY)
          break;
      }
    }

    m_previousSearchInfo.evaluation = alpha;
    m_previousSearchInfo.depthSearched = depth;

    return bestMove;
  }

  Move Bot::iterativeDeepening(int time)
  {
    m_searchCancelled = false;

    int depth = m_botSettings.minSearchDepth;

    m_maxSearchTime = time;
    m_searchTimerEvent.notify_one();

    Move bestMove = generateBestMove(depth);

    while (true)
    {
      depth++;
      Move newMove = generateBestMove(depth);

      if (!m_searchCancelled)
        bestMove = newMove;
      else
        break;
    }

    return bestMove;
  }

  void Bot::heuristicSortMoves(std::vector<Move> &moves)
  {
    std::sort(moves.begin(), moves.end(), [this](Move a, Move b)
              { return heuristicEvaluation(a) > heuristicEvaluation(b); });
  }

  int Bot::heuristicEvaluation(Move move)
  {
    int evaluation = 0;

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    PieceType promotionPieceType = move >> 12;

    evaluation += PIECE_VALUES[m_board[to] & TYPE];
    evaluation += PIECE_VALUES[promotionPieceType];

    evaluation += getPiecePositionalEvaluation(to, true) - getPiecePositionalEvaluation(from, true);

    return evaluation;
  }
}