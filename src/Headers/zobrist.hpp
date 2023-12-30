#pragma once

#include <random>
#include <vector>

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

    std::vector<std::vector<ZobristKey>> pieceKeys;
    std::vector<ZobristKey> castlingKeys;
    std::vector<ZobristKey> enPassantKeys;
    ZobristKey sideKey;

  private:
    static const int PIECE_INDICES[12];
  };
}