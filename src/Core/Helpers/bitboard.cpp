#include "../../Headers/bitboard.hpp"

namespace Chess
{
  const BitboardInt Bitboard::fileMasks[8] = {
      0x8080808080808080ULL,
      0x4040404040404040ULL,
      0x2020202020202020ULL,
      0x1010101010101010ULL,
      0x0808080808080808ULL,
      0x0404040404040404ULL,
      0x0202020202020202ULL,
      0x0101010101010101ULL};

  const BitboardInt Bitboard::rankMasks[8] = {
      0xFF00000000000000ULL,
      0x00FF000000000000ULL,
      0x0000FF0000000000ULL,
      0x000000FF00000000ULL,
      0x00000000FF000000ULL,
      0x0000000000FF0000ULL,
      0x000000000000FF00ULL,
      0x00000000000000FFULL};

  Bitboard::Bitboard()
  {
    this->bitboard = 0;
  }

  Bitboard::Bitboard(BitboardInt bitboard)
  {
    this->bitboard = bitboard;
  }

  void Bitboard::addBit(int index)
  {
    bitboard |= (1ULL << index);
  }

  void Bitboard::removeBit(int index)
  {
    bitboard &= ~(1ULL << index);
  }

  bool Bitboard::hasBit(int index)
  {
    return bitboard & (1ULL << index);
  }

  int Bitboard::countBits()
  {
    return __builtin_popcountll(bitboard);
  }

  Bitboard Bitboard::file(int file)
  {
    return *this & fileMasks[file];
  }

  Bitboard Bitboard::rank(int rank)
  {
    return *this & rankMasks[rank];
  }

  bool Bitboard::isEmpty()
  {
    return bitboard == 0;
  }
}