#pragma once

#include <iostream>

namespace Chess
{
  typedef unsigned long long BitboardInt;

  class Bitboard
  {
  public:
    Bitboard();
    Bitboard(BitboardInt bitboard);

    void addBit(int index);
    void removeBit(int index);

    bool isEmpty();

    Bitboard operator|(Bitboard other);
    Bitboard operator|(BitboardInt other);
    Bitboard operator&(Bitboard other);
    Bitboard operator&(BitboardInt other);

    BitboardInt bitboard;

    bool hasBit(int index);

    int countBits();

    Bitboard file(int file);
    Bitboard rank(int rank);

    operator bool() const { return bitboard != 0; }

  private:
    static const BitboardInt fileMasks[8];
    static const BitboardInt rankMasks[8];

    static const BitboardInt MAX_BITBOARD = 0xFFFFFFFFFFFFFFFF;
  };
}