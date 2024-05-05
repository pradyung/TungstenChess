#pragma once

#include <cstdint>

#define FEN_LENGTH 6
#define NO_EP 8
#define INVALID -1
#define INVALID_MOVE 0
#define POSITIVE_INFINITY 1000000
#define NEGATIVE_INFINITY -1000000

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

namespace Chess
{
  typedef unsigned long long Bitboard;
  typedef unsigned long long Magic;
  typedef unsigned long long ZobristKey;
  typedef uint8_t Piece;
  typedef uint8_t Shift;
  typedef uint16_t MoveInt;

  struct BotSettings
  {
    int maxSearchTime; // In milliseconds, not a hard limit
    int minSearchDepth;
    int maxSearchDepth;
    int quiesceDepth;
    bool useOpeningBook;
    bool logSearchInfo;
    bool logPGNMoves; // as opposed to UCI moves
    bool fixedDepthSearch;
  };

  enum CastlingRights
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

  enum EvaluationBonus
  {
    BISHOP_PAIR_BONUS = 100,
    CASTLED_KING_BONUS = 25,
    CAN_CASTLE_BONUS = 25,
    ROOK_ON_OPEN_FILE_BONUS = 50,
    ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
    KNIGHT_OUTPOST_BONUS = 50,
    PASSED_PAWN_BONUS = 50,
    DOUBLED_PAWN_PENALTY = 50,
    ISOLATED_PAWN_PENALTY = 25,
    BACKWARDS_PAWN_PENALTY = 50,
    KING_SAFETY_PAWN_SHIELD_BONUS = 50,
    STALEMATE_PENALTY = 150,
  };

  enum GameStatus
  {
    NO_MATE = 0,
    STALEMATE = 1,
    LOSE = 2,
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

  enum PieceMasks
  {
    TYPE = 7,
    COLOR = 24
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
    BLACK = 16,
    BOTH = WHITE | BLACK
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

  const int validPieces[12] = {WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING};

  enum PieceCounts
  {
    PIECE_TYPE_NUMBER = 7,
    PIECE_NUMBER = 23
  };

  enum Squares
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

  enum FenParts
  {
    BOARD = 0,
    SIDE_TO_MOVE = 1,
    CASTLING_RIGHTS = 2,
    EN_PASSANT = 3,
    HALFMOVE_CLOCK = 4,
    FULLMOVE_NUMBER = 5
  };
}