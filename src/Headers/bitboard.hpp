#pragma once

#include <iostream>

namespace Chess
{
  class Bitboard
  {
  public:
    Bitboard();
    Bitboard(unsigned long long bitboard);

    void addBit(int index);
    void removeBit(int index);

    bool isEmpty();

    Bitboard operator|(Bitboard other);
    Bitboard operator|(unsigned long long other);
    Bitboard operator&(Bitboard other);
    Bitboard operator&(unsigned long long other);

    unsigned long long bitboard;

    bool hasBit(int index);

    int countBits();

    Bitboard file(int file);
    Bitboard rank(int rank);

    operator bool() const { return bitboard != 0; }

  private:
    static const unsigned long long fileMasks[8];
    static const unsigned long long rankMasks[8];

    static const unsigned long long MAX_BITBOARD = 0xFFFFFFFFFFFFFFFF;
  };
}