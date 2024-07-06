#pragma once

#include <cstdint>
#include <string>

#include "types.hpp"

namespace TungstenChess
{
  struct Move
  {
    uint8_t from;
    uint8_t to;
    Piece piece;
    Piece capturedPiece;
    PieceType promotionPieceType;

    uint8_t castlingRights_enPassantFile; // 0xF0: castling rights, 0x0F: en passant file, grouped together to save space and fit Move object into 8 bytes

    uint8_t halfmoveClock;

    uint8_t flags;

    Move() = default;

    /**
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param piece The piece that is moving
     * @param capturedPiece The piece that is being captured, if any
     * @param enPassantFile The current state of the en passant file, used to restore it when the move is unmade
     * @param castlingRights The current state of the castling rights, used to restore them when the move is unmade
     * @param promotionPieceType The piece that the moving piece is being promoted to, if any (only piece type)
     */
    Move(int from, int to, Piece piece, Piece capturedPiece, int castlingRights, int enPassantFile, int halfmoveClock, PieceType promotionPieceType = EMPTY)
        : from(from), to(to), piece(piece), capturedPiece(capturedPiece), castlingRights_enPassantFile((castlingRights << 4) | enPassantFile), halfmoveClock(halfmoveClock), promotionPieceType(promotionPieceType), flags(NORMAL)
    {
      PieceType pieceType = piece & TYPE;

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

    /**
     * @param move The move to copy
     * @param promotionPieceType The new promotion piece type
     */
    Move(const Move &move, PieceType promotionPieceType) : from(move.from), to(move.to), piece(move.piece), capturedPiece(move.capturedPiece), castlingRights_enPassantFile(move.castlingRights_enPassantFile), halfmoveClock(move.halfmoveClock), promotionPieceType(promotionPieceType), flags(move.flags) {}

    /**
     * Returns an integer representation of the move
     */
    MoveInt toInt() const { return from | (to << 6); }

    /**
     * Returns a UCI string representation of the move
     */
    std::string getUCI() const
    {
      std::string uci = "";

      uci += 'a' + (from % 8);
      uci += '8' - (from / 8);
      uci += 'a' + (to % 8);
      uci += '8' - (to / 8);

      if (promotionPieceType != EMPTY)
        uci += ".pnbrqk"[promotionPieceType];

      return uci;
    }
  };
}