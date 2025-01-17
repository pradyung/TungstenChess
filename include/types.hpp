#pragma once

#include <cstdint>
#include <mutex>

#define NULL_MOVE 0

#define DEBUG_MODE false

#define DEF_USE_OPENING_BOOK !DEBUG_MODE
#define DEF_GUI_THREADING !DEBUG_MODE
#define DEF_USE_ITERATIVE_DEEPENING true
#define DEF_PLAYER_COLOR DEBUG_MODE ? EMPTY : WHITE

namespace TungstenChess
{
  typedef uint8_t Piece;
  typedef uint8_t PieceColor;
  typedef uint8_t PieceType;

  typedef uint16_t Move;

  typedef uint8_t Square;

  typedef uint64_t Bitboard;

  typedef uint64_t ZobristKey;

  typedef uint64_t Magic;
  typedef uint8_t Shift;

  template <bool B>
  struct once
  {
    operator bool()
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (value == B)
      {
        value = !B;
        return B;
      }
      return !B;
    }

    void trigger()
    {
      std::lock_guard<std::mutex> lock(mtx);
      value = !B;
    }

  private:
    bool value = B;
    std::mutex mtx;
  };

  enum CastlingRights : uint8_t
  {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8,
    KINGSIDE = 16,
    QUEENSIDE = 32,
    BOTHSIDES = KINGSIDE | QUEENSIDE,
    WHITE_CASTLING = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_CASTLING = BLACK_KINGSIDE | BLACK_QUEENSIDE,
  };

  enum GameStatus : uint8_t
  {
    NO_MATE = 0,
    STALEMATE = 1,
    LOSE = 2,
  };

  enum MoveFlags : uint8_t
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

  enum MoveMasks : uint16_t
  {
    FROM = 0x3F,
    TO = 0xFC0,
    BASE = FROM | TO,
    PROMOTION_PIECE = 0x7000
  };

  enum FenParts : uint8_t
  {
    FEN_BOARD = 0,
    FEN_SIDE_TO_MOVE = 1,
    FEN_CASTLING_RIGHTS = 2,
    FEN_EN_PASSANT = 3,
    FEN_HALFMOVE_CLOCK = 4,
    FEN_FULLMOVE_NUMBER = 5
  };

  enum PieceMasks : Piece
  {
    TYPE = 7,
    COLOR = 24
  };

  enum PieceTypes : PieceType
  {
    EMPTY = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    PIECE_TYPE_NUMBER = 7
  };

  enum MovePromotions : uint16_t
  {
    KNIGHT_PROMOTION = KNIGHT << 12,
    BISHOP_PROMOTION = BISHOP << 12,
    ROOK_PROMOTION = ROOK << 12,
    QUEEN_PROMOTION = QUEEN << 12
  };

  enum PieceColors : PieceColor
  {
    WHITE = 8,
    BLACK = 16,
    BOTH = WHITE | BLACK
  };

  enum Pieces : Piece
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
    BLACK_KING = BLACK | KING,

    PIECE_NUMBER = 23,
    ALL_PIECES = 24
  };

  const Piece validPieces[13] = {
      EMPTY,
      WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
      BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING};

  enum Squares : uint8_t
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
}