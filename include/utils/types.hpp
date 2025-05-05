#pragma once

#include <cstdint>

#define DEBUG_MODE false

namespace TungstenChess
{
  typedef uint32_t uint;

  typedef uint8_t Piece;
  typedef uint8_t PieceColor;
  typedef uint8_t PieceType;

  typedef uint8_t Square;
  typedef uint8_t Rank;
  typedef uint8_t File;

  enum PieceMasks : Piece
  {
    TYPE = 7,
    COLOR = 24
  };

  enum PieceTypes : PieceType
  {
    NO_TYPE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    PIECE_TYPE_NUMBER = 7
  };

  enum PieceColors : PieceColor
  {
    NO_COLOR = 0,
    WHITE = 8,
    BLACK = 16,
    BOTH = 24 // WHITE | BLACK
  };

  enum Pieces : Piece
  {
    NO_PIECE = 0,
    WHITE_PAWN = 9,    // WHITE | PAWN
    WHITE_KNIGHT = 10, // WHITE | KNIGHT
    WHITE_BISHOP = 11, // WHITE | BISHOP
    WHITE_ROOK = 12,   // WHITE | ROOK
    WHITE_QUEEN = 13,  // WHITE | QUEEN
    WHITE_KING = 14,   // WHITE | KING

    BLACK_PAWN = 17,   // BLACK | PAWN,
    BLACK_KNIGHT = 18, // BLACK | KNIGHT,
    BLACK_BISHOP = 19, // BLACK | BISHOP,
    BLACK_ROOK = 20,   // BLACK | ROOK,
    BLACK_QUEEN = 21,  // BLACK | QUEEN,
    BLACK_KING = 22,   // BLACK | KING,

    PIECE_NUMBER = 23,
    ALL_PIECES = 24
  };

  const Piece validPieces[13] = {
      NO_PIECE,
      WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
      BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING};

  enum Squares : Square
  {
    A8 = 0,
    B8 = 1,
    C8 = 2,
    D8 = 3,
    E8 = 4,
    F8 = 5,
    G8 = 6,
    H8 = 7,
    A7 = 8,
    B7 = 9,
    C7 = 10,
    D7 = 11,
    E7 = 12,
    F7 = 13,
    G7 = 14,
    H7 = 15,
    A6 = 16,
    B6 = 17,
    C6 = 18,
    D6 = 19,
    E6 = 20,
    F6 = 21,
    G6 = 22,
    H6 = 23,
    A5 = 24,
    B5 = 25,
    C5 = 26,
    D5 = 27,
    E5 = 28,
    F5 = 29,
    G5 = 30,
    H5 = 31,
    A4 = 32,
    B4 = 33,
    C4 = 34,
    D4 = 35,
    E4 = 36,
    F4 = 37,
    G4 = 38,
    H4 = 39,
    A3 = 40,
    B3 = 41,
    C3 = 42,
    D3 = 43,
    E3 = 44,
    F3 = 45,
    G3 = 46,
    H3 = 47,
    A2 = 48,
    B2 = 49,
    C2 = 50,
    D2 = 51,
    E2 = 52,
    F2 = 53,
    G2 = 54,
    H2 = 55,
    A1 = 56,
    B1 = 57,
    C1 = 58,
    D1 = 59,
    E1 = 60,
    F1 = 61,
    G1 = 62,
    H1 = 63
  };

  enum Files : File
  {
    FILE_A = 0,
    FILE_B = 1,
    FILE_C = 2,
    FILE_D = 3,
    FILE_E = 4,
    FILE_F = 5,
    FILE_G = 6,
    FILE_H = 7
  };

  enum Ranks : Rank
  {
    RANK_1 = 0,
    RANK_2 = 1,
    RANK_3 = 2,
    RANK_4 = 3,
    RANK_5 = 4,
    RANK_6 = 5,
    RANK_7 = 6,
    RANK_8 = 7
  };
}