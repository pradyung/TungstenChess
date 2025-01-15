#pragma once

#include <random>
#include <array>

#include "types.hpp"

namespace TungstenChess
{
  class Zobrist
  {
  public:
    /**
     * @brief Populates the pieceKeys, castlingKeys, enPassantKeys, and sideKey vectors with random keys
     */
    static void init()
    {
      static once<false> initialized;
      if (initialized)
        return;

      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

      for (Square i = 0; i < 64; i++)
        for (Piece j : validPieces)
          pieceKeys[i][j] = dis(gen);

      for (int i = 0; i < 16; i++)
        castlingKeys[i] = dis(gen);

      for (int i = 0; i < 9; i++)
        enPassantKeys[i] = dis(gen);

      sideKey = dis(gen);

      for (Square i = 0; i < 64; i++)
        for (Piece j : validPieces)
          for (Piece k : validPieces)
            precomputedPieceCombinationKeys[i | (j << 6) | (k << 11)] = pieceKeys[i][j] ^ pieceKeys[i][k];
    }

  private:
    /**
     * @brief Get the combined Zobrist key for two pieces on the same square (used for updating the hash for a single square)
     * @param square The square to get the key for
     * @param before The piece that was on the square before
     * @param after The piece that is on the square now
     */
    static ZobristKey getPieceCombinationKey(Square square, Square before, Square after)
    {
      return precomputedPieceCombinationKeys[square | (before << 6) | (after << 11)];
    }

    static inline std::array<std::array<ZobristKey, PIECE_NUMBER>, 64> pieceKeys = {};
    static inline std::array<ZobristKey, 16> castlingKeys = {};
    static inline std::array<ZobristKey, 9> enPassantKeys = {};
    static inline ZobristKey sideKey = 0;

    static inline std::array<ZobristKey, 64 * 32 * 32> precomputedPieceCombinationKeys = {};

    friend class Board;
  };
}