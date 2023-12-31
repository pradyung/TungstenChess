#pragma once

#include <cctype>

#include "types.hpp"

namespace Chess
{
  class Piece
  {
  public:
    /**
     * @param piece The integer representation of the piece
     */
    Piece(int piece = EMPTY) : piece(piece) {}

    /**
     * @param type The type of the piece
     * @param color The color of the piece
     */
    Piece(int type, int color) : piece(type | color) {}

    /**
     * @param fen The character representing the piece in FEN notation
     */
    Piece(char fen) : piece(std::string("PNBRQK..pnbrqk").find(fen) + 9) {}

    int piece;

    /**
     * @brief Returns the type of the piece - see enum PieceTypes
     */
    inline int getPieceType() { return piece & 7; }

    /**
     * @brief Returns the color of the piece - see enum PieceColors
     */
    inline int getPieceColor() { return piece & 24; }

    /**
     * @brief Returns whether the piece is EMPTY
     */
    inline bool isEmpty() { return piece == EMPTY; }
  };
}