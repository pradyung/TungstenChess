#include "core/bitboard.hpp"

#include <print>

namespace TungstenChess
{
  namespace Bitboards
  {
    void addBit(Bitboard &bitboard, Square index) { bitboard |= bit(index); }

    void removeBit(Bitboard &bitboard, Square index) { bitboard &= ~bit(index); }

    bool hasBit(const Bitboard &bitboard, Square index) { return bitboard & bit(index); }

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
        std::print("{:d} ", hasBit(bitboard, i));
        if (i % 8 == 7)
          std::print("\n");
      }
    }
  }
}