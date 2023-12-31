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
    inline void addBit(int index) { bitboard |= (1ULL << index); }

    /**
     * @brief Removes a bit from the bitboard
     * @param index The index of the bit to remove, 0-63
     */
    inline void removeBit(int index) { bitboard &= ~(1ULL << index); }

    /**
     * @brief Checks if the bitboard is empty
     */
    inline bool isEmpty() { return bitboard == 0; }

    /**
     * @brief Checks if the bitboard has a bit at the given index
     * @param index The index of the bit to check, 0-63
     */
    inline bool hasBit(int index) { return bitboard & (1ULL << index); }

    /**
     * @brief Counts the number of bits in the bitboard
     */
    inline int countBits() { return __builtin_popcountll(bitboard); }

    /**
     * @brief Returns a bitboard with the bits in the given file
     */
    inline Bitboard file(int file) { return *this & (0x8080808080808080ULL >> file); }

    /**
     * @brief Returns a bitboard with the bits in the given rank
     */
    inline Bitboard rank(int rank) { return *this & (0xFFULL << (rank * 8)); }

    /**
     * @brief Overloads the bool constructor to check if the bitboard is not empty
     */
    inline operator bool() const { return bitboard != 0; }

    /**
     * @brief Overloads the & operator to return the intersection of two bitboards
     */
    inline Bitboard operator&(const Bitboard &other) const { return Bitboard(bitboard & other.bitboard); }

    /**
     * @brief Overloads the & operator to return the intersection of two bitboards
     */
    inline Bitboard operator&(const BitboardInt &other) const { return Bitboard(bitboard & other); }

    /**
     * @brief Overloads the | operator to return the union of two bitboards
     */
    inline Bitboard operator|(const Bitboard &other) const { return Bitboard(bitboard | other.bitboard); }

    /**
     * @brief Overloads the | operator to return the union of two bitboards
     */
    inline Bitboard operator|(const BitboardInt &other) const { return Bitboard(bitboard | other); }
  };
}