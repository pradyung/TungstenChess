#pragma once

#include <random>
#include <array>

typedef unsigned long long ZobristKey;

namespace Chess
{
  class Zobrist
  {
  public:
    /**
     * @brief Populates the pieceKeys, castlingKeys, enPassantKeys, and sideKey vectors with random keys
     */
    Zobrist();

    std::array<std::array<ZobristKey, 23>, 64> pieceKeys;
    std::array<ZobristKey, 16> castlingKeys;
    std::array<ZobristKey, 9> enPassantKeys;
    ZobristKey sideKey;

  private:
    static const int PIECE_INDICES[12];
  };
}