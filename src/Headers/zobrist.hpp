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
     * @brief Get the instance of the Zobrist singleton
     * @return Zobrist&
     */
    static Zobrist &getInstance()
    {
      static Zobrist instance;
      return instance;
    }

    std::array<std::array<ZobristKey, PIECE_NUMBER>, 64> pieceKeys;
    std::array<ZobristKey, 16> castlingKeys;
    std::array<ZobristKey, 9> enPassantKeys;
    ZobristKey sideKey;

  private:
    /**
     * @brief Populates the pieceKeys, castlingKeys, enPassantKeys, and sideKey vectors with random keys
     */
    Zobrist()
    {
      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

      for (int i = 0; i < 64; i++)
        for (int j = 0; j < PIECE_NUMBER; j++)
          pieceKeys[i][j] = dis(gen);

      for (int i = 0; i < 16; i++)
        castlingKeys[i] = dis(gen);

      for (int i = 0; i < 9; i++)
        enPassantKeys[i] = dis(gen);

      sideKey = dis(gen);
    }
  };
}