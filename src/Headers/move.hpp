#pragma once
#include <string>
#include "piece.hpp"

namespace Chess
{
  const int NO_EN_PASSANT = 8;

  enum Flags
  {
    NORMAL = 0,
    CAPTURE = 1,
    PAWN_DOUBLE = 2,
    EP_CAPTURE = 4,
    PROMOTION = 8,
    KSIDE_CASTLE = 16,
    QSIDE_CASTLE = 32,
    CASTLE = KSIDE_CASTLE | QSIDE_CASTLE
  };

  class Move
  {
  public:
    Move();

    /**
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param movePiece The piece that is moving
     * @param capturedPiece The piece that is being captured, if any
     * @param enPassantFile The current state of the en passant file, used to restore it when the move is unmade
     * @param castlingRights The current state of the castling rights, used to restore them when the move is unmade
     * @param promotionPiece The piece that the moving piece is being promoted to, if any
     */
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

    /**
     * Returns an integer representation of the move
     */
    int toInt();

    /**
     * Returns a string representation (UCI) of the move
     */
    std::string toString();
  };
}