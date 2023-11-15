#pragma once

#include <iostream>
#include <cctype>

namespace Chess
{
  class Piece
  {
  public:
    Piece();
    Piece(int type, int color);
    Piece(char fen);
    Piece(int piece);

    static Piece fromChar(char fen);

    int piece;

    int getPieceType();
    int getPieceColor();
    char getPieceChar();

    bool isEmpty();

    static const int PIECE_VALUES[7];

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

    static const int PIECE_INDICES[12];

    static int *generateIntArray(Piece board[64]);
  };
}