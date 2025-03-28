#include "bot/engine.hpp"

#include "bot/piece_eval_tables.hpp"
#include "core/moves_lookup/magic.hpp"

namespace TungstenChess
{
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

      int mobility = Bitboards::countBits(m_board.getPseudoLegalPieceMovesBitboard(i));

      Bitboard pawns = m_board.bitboard(((piece & COLOR) ^ COLOR) | PAWN) | m_board.bitboard((piece & COLOR) | PAWN);
      Bitboard invertedFriendlyPiecesMask = ~m_board.bitboard(piece & COLOR);

      if (pieceType & 1) // odd piece types - bishops and queens (diagonal sliders)
        mobility += Bitboards::countBits(MagicMoveGen::getBishopMoves(i, pawns) & invertedFriendlyPiecesMask) / 2;
      if (pieceType >= ROOK) // rooks and queens (orthogonal sliders)
        mobility += Bitboards::countBits(MagicMoveGen::getRookMoves(i, pawns) & invertedFriendlyPiecesMask) / 2;

      mobilityEvaluation += mobility * (m_board[i] & WHITE ? 1 : -1);
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

      if (rank == 0 && m_board[i] == WHITE_KING)
      {
        evaluationBonus += KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i - 8] == WHITE_PAWN);
        if (file > 0)
          evaluationBonus += KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i - 9] == WHITE_PAWN);
        if (file < 7)
          evaluationBonus += KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i - 7] == WHITE_PAWN);
        continue;
      }
      if (rank == 7 && m_board[i] == BLACK_KING)
      {
        evaluationBonus -= KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i + 8] == BLACK_PAWN);
        if (file > 0)
          evaluationBonus -= KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i + 7] == BLACK_PAWN);
        if (file < 7)
          evaluationBonus -= KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS * (m_board[i + 9] == BLACK_PAWN);
        continue;
      }
    }

    return evaluationBonus;
  }
}