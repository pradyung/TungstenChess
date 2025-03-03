#pragma once

#include <iostream>

#include "utils/types.hpp"

namespace TungstenChess
{
  typedef uint64_t Bitboard;

  namespace Bitboards
  {
    void addBit(Bitboard &bitboard, Square index);
    void removeBit(Bitboard &bitboard, Square index);

    bool hasBit(const Bitboard &bitboard, Square index);

    Square countBits(const Bitboard &bitboard);

    Bitboard file(const Bitboard &bitboard, File file);
    Bitboard rank(const Bitboard &bitboard, Rank rank);

    Square popBit(Bitboard &bitboard);

    void printBitboard(const Bitboard &bitboard);
  };
}