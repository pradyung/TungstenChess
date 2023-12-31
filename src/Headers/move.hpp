#pragma once

#include "piece.hpp"

namespace Chess
{
  const int NO_EN_PASSANT = 8;

  class Move
  {
  public:
    Move() : from(0), to(0), piece(0), capturedPiece(0), enPassantFile(NO_EN_PASSANT), castlingRights(15), flags(NORMAL) {}

    /**
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param movePiece The piece that is moving
     * @param capturedPiece The piece that is being captured, if any
     * @param enPassantFile The current state of the en passant file, used to restore it when the move is unmade
     * @param castlingRights The current state of the castling rights, used to restore them when the move is unmade
     * @param promotionPiece The piece that the moving piece is being promoted to, if any
     */
    Move(int from, int to, int movePiece, int capturedPiece, int enPassantFile, int castlingRights, int promotionPiece = EMPTY)
        : from(from), to(to), piece(movePiece), capturedPiece(capturedPiece), enPassantFile(enPassantFile), castlingRights(castlingRights), promotionPiece(promotionPiece), flags(NORMAL)
    {
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
    int toInt() { return from ^ 0x38 | ((to ^ 0x38) << 6); }
  };
}