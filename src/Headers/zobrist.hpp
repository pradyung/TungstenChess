#pragma once

#include <random>
#include <array>

#include "types.hpp"

namespace Chess
{
  class Zobrist
  {
  public:
    /**
     * @brief Populates the pieceKeys, castlingKeys, enPassantKeys, and sideKey vectors with random keys
     */
    Zobrist()
    {
      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

      for (Square i = 0; i < 64; i++)
        for (Piece j = 0; j < PIECE_NUMBER; j++)
          pieceKeys[i][j] = dis(gen);

      for (uint8_t i = 0; i < 16; i++)
        castlingKeys[i] = dis(gen);

      for (uint8_t i = 0; i < 9; i++)
        enPassantKeys[i] = dis(gen);

      sideKey = dis(gen);
    }

    std::array<std::array<ZobristKey, PIECE_NUMBER>, 64> pieceKeys;
    std::array<ZobristKey, 16> castlingKeys;
    std::array<ZobristKey, 9> enPassantKeys;
    ZobristKey sideKey;
  };
}