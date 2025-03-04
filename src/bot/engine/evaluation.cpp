#include "bot/engine.hpp"

#include "bot/piece_eval_tables.hpp"

namespace TungstenChess
{
  enum EvaluationConstants : int
  {
    BISHOP_PAIR_BONUS = 100,
    CASTLED_KING_BONUS = 25,
    CAN_CASTLE_BONUS = 25,
    ROOK_ON_OPEN_FILE_BONUS = 50,
    ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
    KNIGHT_OUTPOST_BONUS = 50,
    PASSED_PAWN_BONUS = 50,
    DOUBLED_PAWN_PENALTY = 50,
    ISOLATED_PAWN_PENALTY = 25,
    BACKWARDS_PAWN_PENALTY = 50,
    KING_SAFETY_PAWN_SHIELD_BONUS = 50,
    CONTEMPT = 100,
  };

  int Bot::getStaticEvaluation()
  {
    m_previousSearchInfo.positionsEvaluated++;

    Board::GameStatus gameStatus = m_board.getGameStatus(m_board.sideToMove());

    if (gameStatus != Board::NO_MATE)
    {
      if (gameStatus == Board::LOSE)
        return NEGATIVE_INFINITY;
      else
        return -CONTEMPT;
    }

    int staticEvaluation = getMaterialEvaluation() + getPositionalEvaluation() + getMobilityEvaluation() + getEvaluationBonus();

    return m_board.sideToMove() == WHITE ? staticEvaluation : -staticEvaluation;
  }

  int Bot::getMaterialEvaluation() const
  {
    int whiteMaterial = 0;
    int blackMaterial = 0;

    for (PieceType pieceType = PAWN; pieceType <= QUEEN; pieceType++)
    {
      whiteMaterial += PIECE_VALUES[pieceType] * m_board.pieceCount(WHITE | pieceType);
      blackMaterial += PIECE_VALUES[pieceType] * m_board.pieceCount(BLACK | pieceType);
    }

    whiteMaterial -= (whiteMaterial * whiteMaterial) >> MATERIAL_DIMINISH_SHIFT;
    blackMaterial -= (blackMaterial * blackMaterial) >> MATERIAL_DIMINISH_SHIFT;

    return whiteMaterial - blackMaterial;
  }

  int Bot::getPiecePositionalEvaluation(Square pieceIndex, bool absolute) const
  {
    int positionalEvaluation = 0;

    positionalEvaluation = PIECE_EVAL_TABLES[m_board[pieceIndex]][pieceIndex];

    if (!absolute && (m_board[pieceIndex] & BLACK))
      positionalEvaluation = -positionalEvaluation;

    return positionalEvaluation;
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

  int Bot::getMobilityEvaluation() const
  {
    int mobilityEvaluation = 0;

    for (Square i = 0; i < 64; i++)
    {
      Piece piece = m_board[i];
      PieceType pieceType = piece & TYPE;

      if (pieceType <= PAWN || pieceType == KING)
        continue;

      mobilityEvaluation += Bitboards::countBits(m_board.getPseudoLegalPieceMovesBitboard(i)) * (m_board[i] & WHITE ? 1 : -1);
    }

    return mobilityEvaluation;
  }

  int Bot::getEvaluationBonus() const
  {
    int evaluationBonus = 0;

    evaluationBonus += BISHOP_PAIR_BONUS * ((m_board.pieceCount(WHITE_BISHOP) >= 2) - (m_board.pieceCount(BLACK_BISHOP) >= 2));

    evaluationBonus += CAN_CASTLE_BONUS * CASTLING_BONUS_MULTIPLIERS[m_board.castlingRights()];

    evaluationBonus += CASTLED_KING_BONUS * (bool(m_board.hasCastled() & WHITE) - bool(m_board.hasCastled() & BLACK));

    std::array<uint, 8> whitePawnsOnFiles = {0};
    std::array<uint, 8> blackPawnsOnFiles = {0};

    std::array<bool, 8> whitePawnsOnNeighboringFiles = {false};
    std::array<bool, 8> blackPawnsOnNeighboringFiles = {false};

    for (File file = 0; file < 8; file++)
    {
      whitePawnsOnFiles[file] = Bitboards::countBits(Bitboards::file(m_board.bitboard(WHITE_PAWN), file));
      blackPawnsOnFiles[file] = Bitboards::countBits(Bitboards::file(m_board.bitboard(BLACK_PAWN), file));

      if (file > 0)
      {
        whitePawnsOnNeighboringFiles[file - 1] |= bool(whitePawnsOnFiles[file]);
        blackPawnsOnNeighboringFiles[file - 1] |= bool(blackPawnsOnFiles[file]);
      }
      if (file < 7)
      {
        whitePawnsOnNeighboringFiles[file + 1] |= bool(whitePawnsOnFiles[file]);
        blackPawnsOnNeighboringFiles[file + 1] |= bool(blackPawnsOnFiles[file]);
      }
    }

    for (Square i = 0; i < 64; i++)
    {
      File file = i % 8;
      Rank rank = i / 8;

      if (rank == 0)
      {
        evaluationBonus -= DOUBLED_PAWN_PENALTY * ((whitePawnsOnFiles[file] > 1) - (blackPawnsOnFiles[file] > 1));

        if (whitePawnsOnFiles[file])
        {
          if (!blackPawnsOnNeighboringFiles[file])
            evaluationBonus += PASSED_PAWN_BONUS;

          if (!whitePawnsOnNeighboringFiles[file])
            evaluationBonus -= ISOLATED_PAWN_PENALTY;
        }
        if (blackPawnsOnFiles[file])
        {
          if (!whitePawnsOnNeighboringFiles[file])
            evaluationBonus -= PASSED_PAWN_BONUS;

          if (!blackPawnsOnNeighboringFiles[file])
            evaluationBonus += ISOLATED_PAWN_PENALTY;
        }
      }

      if (!m_board[i])
        continue;

      if (m_board[i] == WHITE_ROOK)
      {
        if (!(blackPawnsOnFiles[file] || whitePawnsOnFiles[file]))
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (!blackPawnsOnFiles[file])
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (m_board[i] == BLACK_ROOK)
      {
        if (!(blackPawnsOnFiles[file] || whitePawnsOnFiles[file]))
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (!whitePawnsOnFiles[file])
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (m_board[i] == WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && !blackPawnsOnNeighboringFiles[file])
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (m_board[i] == BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && !whitePawnsOnNeighboringFiles[file])
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

    bool found;
    const TranspositionTable::Entry &entry = m_transpositionTable.retrieve(m_board.zobristKey(), found);

    if (found && entry.quiesce() == quiesce && entry.depth() >= depth)
    {
      if (!((entry.evaluation() == POSITIVE_INFINITY || entry.evaluation() == NEGATIVE_INFINITY) && entry.searchId() != m_currentSearchId && entry.depth() != depth))
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

    MoveArray legalMoves;
    int legalMovesCount = getSortedLegalMoves(legalMoves, m_board.sideToMove(), quiesce);

    if (legalMovesCount == 0)
      return (m_board.isInCheck(m_board.sideToMove()) ? NEGATIVE_INFINITY + (quiesce ? 10000 : 0) : -CONTEMPT);

    if (legalMovesCount == 1)
      depth++;

    for (Move &move : legalMoves)
    {
      if (move == NULL_MOVE)
        break;

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

    if (!m_searchCancelled)
    {
      m_transpositionTable.store(m_board.zobristKey(), m_currentSearchId, alpha, depth, quiesce);
    }

    return alpha;
  }
}