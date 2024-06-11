#pragma once

#include <random>
#include <array>

#include "types.hpp"

namespace TungstenChess
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

    /**
     * @brief Get the combined Zobrist key for two pieces on the same square (used for updating the hash for a single square)
     * @param square The square to get the key for
     * @param before The piece that was on the square before
     * @param after The piece that is on the square now
     */
    ZobristKey getPieceCombinationKey(int square, int before, int after)
    {
      return precomputedPieceCombinationKeys[square | (before << 6) | (after << 11)];
    }

    std::array<std::array<ZobristKey, PIECE_NUMBER>, 64> pieceKeys;
    std::array<ZobristKey, 16> castlingKeys;
    std::array<ZobristKey, 9> enPassantKeys;
    ZobristKey sideKey;

    std::array<ZobristKey, 64 * 32 * 32> precomputedPieceCombinationKeys;

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
            precomputedPieceCombinationKeys[i | (j << 6) | (k << 11)] = pieceKeys[i][j] ^ pieceKeys[i][k];
    }
  };
}