#pragma once

#include <iostream>

namespace Chess
{
  typedef unsigned long long BitboardInt;

  class Bitboard
  {
  public:
    BitboardInt bitboard;

    Bitboard();
    Bitboard(BitboardInt bitboard);

    /**
     * @brief Adds a bit to the bitboard
     * @param index The index of the bit to add, 0-63
     */
    void addBit(int index);

    /**
     * @brief Removes a bit from the bitboard
     * @param index The index of the bit to remove, 0-63
     */
    void removeBit(int index);

    /**
     * @brief Checks if the bitboard is empty
     * @return bool
     */
    bool isEmpty();

    /**
     * @brief Checks if the bitboard has a bit at the given index
     * @param index The index of the bit to check, 0-63
     * @return bool
     */
    bool hasBit(int index);

    /**
     * @brief Counts the number of bits in the bitboard
     * @return int
     */
    int countBits();

    /**
     * @brief Returns a bitboard with the bits in the given file
     * @return Bitboard
     */
    Bitboard file(int file);

    /**
     * @brief Returns a bitboard with the bits in the given rank
     * @return Bitboard
     */
    Bitboard rank(int rank);

    /**
     * @brief Overloads the bool constructor to check if the bitboard is not empty
     * @return bool
     */
    operator bool() const { return bitboard != 0; }

  private:
    static const BitboardInt fileMasks[8];
    static const BitboardInt rankMasks[8];

    static const BitboardInt MAX_BITBOARD = 0xFFFFFFFFFFFFFFFF;
  };
}