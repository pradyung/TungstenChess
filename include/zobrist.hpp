#pragma once

#include <random>
#include <array>

#include "types.hpp"
#include "utils.hpp"

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

      for (Piece piece : validPieces)
        for (Square square = 0; square < 64; square++)
          pieceKeys[piece, square] = dis(gen);

      for (int i = 0; i < 16; i++)
        castlingKeys[i] = dis(gen);

      for (int i = 0; i < 9; i++)
        enPassantKeys[i] = dis(gen);

      sideKey = dis(gen);

      for (Piece piece1 : validPieces)
        for (Piece piece2 : validPieces)
          for (Square square = 0; square < 64; square++)
            precomputedPieceCombinationKeys[square | (piece1 << 6) | (piece2 << 11)] = pieceKeys[piece1, square] ^ pieceKeys[piece2, square];
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

    static inline array2d<ZobristKey, PIECE_NUMBER, 64> pieceKeys = {};
    static inline std::array<ZobristKey, 16> castlingKeys = {};
    static inline std::array<ZobristKey, 9> enPassantKeys = {};
    static inline ZobristKey sideKey = 0;

    static inline std::array<ZobristKey, 64 * 32 * 32> precomputedPieceCombinationKeys = {};

    friend class Board;
  };
}