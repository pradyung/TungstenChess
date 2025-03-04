#include "core/move.hpp"

namespace TungstenChess
{
  std::string Moves::getUCI(Move move)
  {
    std::string uci = "";

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    uint8_t promotionPieceType = move >> 12;

    uci += 'a' + (from % 8);
    uci += '8' - (from / 8);
    uci += 'a' + (to % 8);
    uci += '8' - (to / 8);

    if (promotionPieceType != NO_TYPE)
      uci += ".pnbrqk"[promotionPieceType];

    return uci;
  }

  Move Moves::createMove(Square from, Square to, PieceType promotionPieceType)
  {
    return from | (to << 6) | (promotionPieceType << 12);
  }

  uint8_t Moves::getMoveFlags(Square from, Square to, PieceType pieceType, Piece capturedPiece)
  {
    if (pieceType == KING)
    {
      if (from - to == -2)
        return KSIDE_CASTLE;
      if (from - to == 2)
        return QSIDE_CASTLE;
    }
    else if (pieceType == PAWN)
    {
      if (from - to == 16 || from - to == -16)
        return PAWN_DOUBLE;
      else if (capturedPiece == NO_PIECE && (to - from) % 8)
        return EP_CAPTURE;
      else if (to <= 7 || to >= 56)
        return PROMOTION | bool(capturedPiece);
    }

    return bool(capturedPiece);
  }

  bool Moves::isPromotion(Square to, PieceType pieceType)
  {
    return (pieceType == PAWN && (to <= 7 || to >= 56));
  }
}