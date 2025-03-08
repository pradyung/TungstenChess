#include "core/bitboard.hpp"

#include <iostream>

namespace TungstenChess
{
  namespace Bitboards
  {
    void addBit(Bitboard &bitboard, Square index) { bitboard |= (1ULL << index); }

    void removeBit(Bitboard &bitboard, Square index) { bitboard &= ~(1ULL << index); }

    bool hasBit(const Bitboard &bitboard, Square index) { return bitboard & (1ULL << index); }

    Square countBits(const Bitboard &bitboard) { return __builtin_popcountll(bitboard); }

    Bitboard file(const Bitboard &bitboard, File file) { return bitboard & (0x0101010101010101ULL << file); }

    Bitboard rank(const Bitboard &bitboard, Rank rank) { return bitboard & (0xFFULL << (rank * 8)); }

    Square popBit(Bitboard &bitboard)
    {
      Square index = __builtin_ctzll(bitboard);
      bitboard &= bitboard - 1;
      return index;
    }

    void printBitboard(const Bitboard &bitboard)
    {
      for (Square i = 0; i < 64; i++)
      {
        if (i % 8 == 0)
          std::cout << std::endl;
        std::cout << (hasBit(bitboard, i) ? "1" : "0") << " ";
      }
      std::cout << std::endl;
    }
  }
}