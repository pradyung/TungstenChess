#pragma once

#include <random>
#include <array>

#include "types.hpp"

namespace Chess
{
  typedef uint64_t ZobristKey;

  const int validPieces[13] = {
      EMPTY,
      WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
      BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING};

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

    std::array<std::array<std::array<ZobristKey, PIECE_NUMBER>, PIECE_NUMBER>, 64> precomputedPieceCombinationKeys;

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
        for (int j : validPieces)
          pieceKeys[i][j] = dis(gen);

      for (int i = 0; i < 16; i++)
        castlingKeys[i] = dis(gen);

      for (int i = 0; i < 9; i++)
        enPassantKeys[i] = dis(gen);

      sideKey = dis(gen);

      for (int i = 0; i < 64; i++)
        for (int j : validPieces)
          for (int k : validPieces)
            precomputedPieceCombinationKeys[i][j][k] = pieceKeys[i][j] ^ pieceKeys[i][k];
    }
  };
}