#pragma once

#include "types.hpp"

namespace Chess
{
  class Move
  {
  public:
    Move() : from(0), to(0), piece(0), capturedPiece(0), castlingRights(0), enPassantFile(0), flags(NORMAL) {}

    /**
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param piece The piece that is moving
     * @param capturedPiece The piece that is being captured, if any
     * @param enPassantFile The current state of the en passant file, used to restore it when the move is unmade
     * @param castlingRights The current state of the castling rights, used to restore them when the move is unmade
     * @param promotionPieceType The piece that the moving piece is being promoted to, if any (only piece type)
     */
    Move(int from, int to, int piece, int capturedPiece, int castlingRights, int enPassantFile, int promotionPieceType = EMPTY)
        : from(from), to(to), piece(piece), capturedPiece(capturedPiece), castlingRights(castlingRights), enPassantFile(enPassantFile), promotionPieceType(promotionPieceType), flags(NORMAL)
    {
      int pieceType = piece & TYPE;

      if (pieceType == KING && from - to == -2)
      {
        this->flags |= KSIDE_CASTLE;
        return;
      }

      if (pieceType == KING && from - to == 2)
      {
        this->flags |= QSIDE_CASTLE;
        return;
      }

      if (pieceType == PAWN && (from - to == 16 || from - to == -16))
      {
        this->flags |= PAWN_DOUBLE;
        return;
      }

      if (pieceType == PAWN && capturedPiece == EMPTY && (to - from) % 8)
      {
        this->flags |= EP_CAPTURE;
        return;
      }

      if (capturedPiece != EMPTY)
      {
        this->flags |= CAPTURE;
      }

      if (pieceType == PAWN && (to <= 7 || to >= 56))
      {
        this->flags |= PROMOTION;
      }
    }

    int from;
    int to;
    int piece;
    int capturedPiece;
    int promotionPieceType;

    int castlingRights;
    int enPassantFile;

    int flags;

    /**
     * Returns an integer representation of the move
     */
    int toInt() const { return from | (to << 6); }

    /**
     * Returns a UCI string representation of the move
     */
    std::string getUCI() const
    {
      std::string uci = "";

      uci += 'a' + (from & TYPE);
      uci += '8' - (from / 8);
      uci += 'a' + (to & TYPE);
      uci += '8' - (to / 8);

      if (promotionPieceType != EMPTY)
      {
        uci += ".pnbrqk"[promotionPieceType];
      }

      return uci;
    }
  };
}