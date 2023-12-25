#pragma once
#include <string>
#include "piece.hpp"

namespace Chess
{
  enum Flags
  {
    NORMAL = 0,
    CAPTURE = 1,
    PAWN_DOUBLE = 2,
    EP_CAPTURE = 4,
    PROMOTION = 8,
    KSIDE_CASTLE = 16,
    QSIDE_CASTLE = 32
  };

  class Move
  {
  public:
    Move();
    Move(int from, int to, int movePiece, int capturedPiece, int enPassantFile, int castlingRights, int promotionPiece = EMPTY);

    int from;
    int to;
    int piece;
    int capturedPiece;
    int promotionPiece;

    int enPassantFile;
    int castlingRights;

    int flags;

    int computedEvaluation;

    int toInt();
    std::string toString();
  };
}