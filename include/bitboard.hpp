#pragma once

#include <iostream>

#include "types.hpp"

namespace TungstenChess
{
  namespace Bitboards
  {
    static void addBit(Bitboard &bitboard, int index) { bitboard |= (1ULL << index); }
    static void removeBit(Bitboard &bitboard, int index) { bitboard &= ~(1ULL << index); }
    static bool isEmpty(const Bitboard &bitboard) { return bitboard == 0; }
    static bool hasBit(const Bitboard &bitboard, int index) { return bitboard & (1ULL << index); }
    static int countBits(const Bitboard &bitboard) { return __builtin_popcountll(bitboard); }
    static Bitboard file(const Bitboard &bitboard, int file) { return bitboard & (0x0101010101010101ULL << file); }
    static Bitboard rank(const Bitboard &bitboard, int rank) { return bitboard & (0xFFULL << (rank * 8)); }
    static int popBit(Bitboard &bitboard)
    {
      int index = __builtin_ctzll(bitboard);
      removeBit(bitboard, index);
      return index;
    }
    static void printBitboard(const Bitboard &bitboard)
    {
      for (int i = 0; i < 64; i++)
      {
        if (i % 8 == 0)
          std::cout << std::endl;
        std::cout << (hasBit(bitboard, i) ? "1" : "0") << " ";
      }
      std::cout << std::endl;
    }
  };
}