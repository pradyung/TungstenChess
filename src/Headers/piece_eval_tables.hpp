#pragma once

namespace Chess
{
  class PieceEvalTables
  {
  public:
    static const int PAWN_EVAL_TABLE[64];
    static const int KNIGHT_EVAL_TABLE[64];
    static const int BISHOP_EVAL_TABLE[64];
    static const int ROOK_EVAL_TABLE[64];
    static const int QUEEN_EVAL_TABLE[64];
    static const int KING_EVAL_TABLE[64];
    static const int KING_ENDGAME_EVAL_TABLE[64];
    static const int KINGS_DISTANCE_EVAL_TABLE[16];
  };
}