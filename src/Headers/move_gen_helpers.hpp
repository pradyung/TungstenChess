#pragma once

#include <string>

#include "bitboard.hpp"

namespace Chess
{
  class MovesLookup
  {
  public:
    static BitboardInt KNIGHT_MOVES[64];
    static BitboardInt KING_MOVES[64];
  };
}