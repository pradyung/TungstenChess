#include "../../Headers/move.hpp"

namespace Chess
{
  Move::Move()
  {
    this->from = 0;
    this->to = 0;
    this->piece = 0;
    this->capturedPiece = 0;

    this->enPassantFile = NO_EN_PASSANT;
    this->castlingRights = 15;

    this->flags = NORMAL;
  }

  Move::Move(int from, int to, int movePiece, int capturedPiece, int enPassantFile, int castlingRights, int promotionPiece)
  {
    this->from = from;
    this->to = to;
    this->piece = movePiece;
    this->capturedPiece = capturedPiece;
    this->promotionPiece = promotionPiece;

    this->enPassantFile = enPassantFile;
    this->castlingRights = castlingRights;

    this->flags = NORMAL;

    int movePieceType = movePiece & 7;
    int movePieceColor = movePiece & 24;

    if (movePieceType == KING && from - to == -2)
    {
      this->flags |= KSIDE_CASTLE;
      return;
    }

    if (movePieceType == KING && from - to == 2)
    {
      this->flags |= QSIDE_CASTLE;
      return;
    }

    if (movePieceType == PAWN && (from - to == 16 || from - to == -16))
    {
      this->flags |= PAWN_DOUBLE;
      return;
    }

    if (movePieceType == PAWN && capturedPiece == EMPTY && (to - from) % 8)
    {
      this->flags |= EP_CAPTURE;
      return;
    }

    if (capturedPiece != EMPTY)
    {
      this->flags |= CAPTURE;
    }

    if (movePieceType == PAWN && (to <= 7 || to >= 56))
    {
      this->flags |= PROMOTION;
    }
  }

  std::string Move::toString()
  {
    std::string result = "";

    int fromX = from % 8;
    int fromY = 8 - from / 8;

    int toX = to % 8;
    int toY = 8 - to / 8;

    result += (char)(fromX + 'a');
    result += (char)(fromY + '0');

    result += (char)(toX + 'a');
    result += (char)(toY + '0');

    if (flags & PROMOTION)
    {
      switch (promotionPiece)
      {
      case QUEEN:
        result += "q";
        break;
      case ROOK:
        result += "r";
        break;
      case BISHOP:
        result += "b";
        break;
      case KNIGHT:
        result += "n";
        break;
      }
    }

    return result;
  }

  int Move::toInt()
  {
    return from ^ 0x38 | ((to ^ 0x38) << 6);
  }
}