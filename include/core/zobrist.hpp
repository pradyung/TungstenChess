#pragma once

#include "utils/types.hpp"
#include "utils/utils.hpp"

namespace TungstenChess
{
  typedef uint64_t ZobristKey;

  class Zobrist
  {
  public:
    /**
     * @brief Populates the pieceKeys, castlingKeys, enPassantKeys, and sideKey vectors with random keys
     */
    static void init();

  private:
    /**
     * @brief Get the combined Zobrist key for two pieces on the same square
     *        (used for updating the hash for a single square)
     * @param square The square to get the key for
     * @param before The piece that was on the square before
     * @param after The piece that is on the square now
     */
    static ZobristKey getPieceCombinationKey(Square square, Square before, Square after);

    static inline utils::array2d<ZobristKey, PIECE_NUMBER, 64> pieceKeys = {};
    static inline std::array<ZobristKey, 16> castlingKeys = {};
    static inline std::array<ZobristKey, 9> enPassantKeys = {};
    static inline ZobristKey sideKey = 0;

    static inline std::array<ZobristKey, 64 * 32 * 32> precomputedPieceCombinationKeys = {};

    friend class Board;
    friend class OpeningBook;
  };
}