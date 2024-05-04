#pragma once

#include "types.hpp"

namespace Chess
{
  class Bitboard
  {
  public:
    BitboardInt bitboard;

    Bitboard(BitboardInt bitboard = 0) : bitboard(bitboard) {}

    /**
     * @brief Adds a bit to the bitboard
     * @param index The index of the bit to add, 0-63
     */
    void addBit(int index) { bitboard |= (1ULL << index); }

    /**
     * @brief Removes a bit from the bitboard
     * @param index The index of the bit to remove, 0-63
     */
    void removeBit(int index) { bitboard &= ~(1ULL << index); }

    /**
     * @brief Checks if the bitboard is empty
     */
    bool isEmpty() const { return bitboard == 0; }

    /**
     * @brief Checks if the bitboard has a bit at the given index
     * @param index The index of the bit to check, 0-63
     */
    bool hasBit(int index) const { return bitboard & (1ULL << index); }

    /**
     * @brief Counts the number of bits in the bitboard
     */
    int countBits() const { return __builtin_popcountll(bitboard); }

    /**
     * @brief Returns a bitboard with the bits in the given file
     */
    Bitboard file(int file) const { return *this & (0x0101010101010101ULL >> file); }

    /**
     * @brief Returns a bitboard with the bits in the given rank
     */
    Bitboard rank(int rank) const { return *this & (0xFFULL << (rank * 8)); }

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

    /**
     * @brief Overloads the ~ operator to return the complement of the bitboard
     */
    Bitboard operator~() const { return Bitboard(~bitboard); }
  };
}