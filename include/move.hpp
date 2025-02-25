#pragma once

#include <cstdint>
#include <string>

#include "types.hpp"

namespace TungstenChess
{
  class Moves
  {
  public:
    /**
     * @brief Returns a UCI string representation of the move
     * @param move The move to convert
     */
    static std::string getUCI(Move move)
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

    /**
     * @brief Creates a move
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param promotionPieceType The piece type to promote to (if any)
     * @return The created move
     */
    static Move createMove(Square from, Square to, PieceType promotionPieceType = NO_TYPE)
    {
      return from | (to << 6) | (promotionPieceType << 12);
    }

    /**
     * Returns the move flags for a move
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param pieceType The type of the piece moving
     * @param capturedPiece The piece being captured (if any)
     */
    static uint8_t getMoveFlags(Square from, Square to, PieceType pieceType, Piece capturedPiece)
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

    /**
     * @brief Checks if a move is a promotion
     * @param to The square the piece is moving to
     * @param pieceType The type of the piece moving (only matters if it's a pawn)
     */
    static bool isPromotion(Square to, PieceType pieceType)
    {
      return (pieceType == PAWN && (to <= 7 || to >= 56));
    }
  };
}