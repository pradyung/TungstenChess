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
     */
    bool isEmpty();

    /**
     * @brief Checks if the bitboard has a bit at the given index
     * @param index The index of the bit to check, 0-63
     */
    bool hasBit(int index);

    /**
     * @brief Counts the number of bits in the bitboard
     */
    int countBits();

    /**
     * @brief Returns a bitboard with the bits in the given file
     */
    Bitboard file(int file);

    /**
     * @brief Returns a bitboard with the bits in the given rank
     */
    Bitboard rank(int rank);

    /**
     * @brief Overloads the bool constructor to check if the bitboard is not empty
     */
    operator bool() const { return bitboard != 0; }

    /**
     * @brief Overloads the & operator to return the intersection of two bitboards
     */
    Bitboard operator&(const Bitboard &other) const { return Bitboard(bitboard & other.bitboard); }

    /**
     * @brief Overloads the & operator to return the intersection of two bitboards
     */
    Bitboard operator&(const BitboardInt &other) const { return Bitboard(bitboard & other); }

    /**
     * @brief Overloads the | operator to return the union of two bitboards
     */
    Bitboard operator|(const Bitboard &other) const { return Bitboard(bitboard | other.bitboard); }

    /**
     * @brief Overloads the | operator to return the union of two bitboards
     */
    Bitboard operator|(const BitboardInt &other) const { return Bitboard(bitboard | other); }

  private:
    static const BitboardInt fileMasks[8];
    static const BitboardInt rankMasks[8];

    static const BitboardInt MAX_BITBOARD = 0xFFFFFFFFFFFFFFFF;
  };
}