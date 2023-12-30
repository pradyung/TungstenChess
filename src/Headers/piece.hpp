#pragma once

#include <iostream>
#include <cctype>

namespace Chess
{
  enum PieceTypes
  {
    EMPTY = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
  };

  enum PieceColors
  {
    WHITE = 8,
    BLACK = 16
  };

  enum Pieces
  {
    WHITE_PAWN = WHITE | PAWN,
    WHITE_KNIGHT = WHITE | KNIGHT,
    WHITE_BISHOP = WHITE | BISHOP,
    WHITE_ROOK = WHITE | ROOK,
    WHITE_QUEEN = WHITE | QUEEN,
    WHITE_KING = WHITE | KING,

    BLACK_PAWN = BLACK | PAWN,
    BLACK_KNIGHT = BLACK | KNIGHT,
    BLACK_BISHOP = BLACK | BISHOP,
    BLACK_ROOK = BLACK | ROOK,
    BLACK_QUEEN = BLACK | QUEEN,
    BLACK_KING = BLACK | KING
  };

  enum Numbers
  {
    PIECE_TYPE_NUMBER = 7,
    PIECE_NUMBER = 23
  };

  class Piece
  {
  public:
    Piece();

    /**
     * @param type The type of the piece
     * @param color The color of the piece
     */
    Piece(int type, int color);

    /**
     * @param fen The character representing the piece in FEN notation
     */
    Piece(char fen);

    /**
     * @param piece The integer representation of the piece
     */
    Piece(int piece);

    int piece;

    /**
     * @brief Returns the type of the piece - see enum PieceTypes
     */
    int getPieceType();

    /**
     * @brief Returns the color of the piece - see enum PieceColors
     */
    int getPieceColor();

    /**
     * @brief Returns the character representing the piece in FEN notation
     */
    char getPieceChar();

    /**
     * @brief Returns whether the piece is EMPTY
     */
    bool isEmpty();

    static const int PIECE_VALUES[7];
  };
}