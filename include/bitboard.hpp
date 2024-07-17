#pragma once

#include "types.hpp"

namespace TungstenChess
{
  namespace Bitboards
  {
    static void addBit(Bitboard &bitboard, int index) { bitboard |= (1ULL << index); }
    static void removeBit(Bitboard &bitboard, int index) { bitboard &= ~(1ULL << index); }
    static bool isEmpty(Bitboard bitboard) { return bitboard == 0; }
    static bool hasBit(Bitboard bitboard, int index) { return bitboard & (1ULL << index); }
    static int countBits(Bitboard bitboard) { return __builtin_popcountll(bitboard); }
    static Bitboard file(Bitboard bitboard, int file) { return bitboard & (0x0101010101010101ULL << file); }
    static Bitboard rank(Bitboard bitboard, int rank) { return bitboard & (0xFFULL << (rank * 8)); }
    static int popBit(Bitboard &bitboard)
    {
      int index = __builtin_ctzll(bitboard);
      removeBit(bitboard, index);
      return index;
    }
  };
}