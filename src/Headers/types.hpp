#pragma once

namespace Chess
{
  typedef unsigned long long BitboardInt;
  typedef unsigned long long ZobristKey;

  enum CastlingRights
  {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8,
    KINGSIDE = 16,
    QUEENSIDE = 32,
    CASTLING = KINGSIDE | QUEENSIDE,
    WHITE_CASTLING = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_CASTLING = BLACK_KINGSIDE | BLACK_QUEENSIDE,
  };

  enum EvaluationBonus
  {
    BISHOP_PAIR_BONUS = 100,
    CASTLED_KING_BONUS = 50,
    CAN_CASTLE_BONUS = 25,
    ROOK_ON_OPEN_FILE_BONUS = 50,
    ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
    KNIGHT_OUTPOST_BONUS = 50,
    PASSED_PAWN_BONUS = 50,
    DOUBLED_PAWN_PENALTY = 50,
    ISOLATED_PAWN_PENALTY = 25,
    BACKWARDS_PAWN_PENALTY = 50,
    KING_SAFETY_PAWN_SHIELD_BONUS = 50,
  };

  enum GameStatus
  {
    NO_MATE,
    STALEMATE,
    LOSE
  };

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
}