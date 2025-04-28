#pragma once

#include <array>

#include "core/bitboard.hpp"
#include "utils/utils.hpp"

namespace TungstenChess
{
  class MovesLookup
  {
  public:
    /**
     * @brief Initializes the move lookup tables
     */
    static void init();

  private:
    static inline std::array<Bitboard, 64> KNIGHT_MOVES = {};
    static inline std::array<Bitboard, 64> KING_MOVES = {};
    static inline std::array<Bitboard, 64> BISHOP_MASKS = {};
    static inline std::array<Bitboard, 64> ROOK_MASKS = {};

    static inline utils::array2d<Bitboard, BLACK_PAWN + 1, 64> PAWN_CAPTURE_MOVES = {};
    static inline utils::array2d<Bitboard, BLACK_PAWN + 1, 64> PAWN_REVERSE_SINGLE_MOVES = {};
    static inline utils::array2d<Bitboard, BLACK_PAWN + 1, 64> PAWN_REVERSE_DOUBLE_MOVES = {};

    friend class Board;
    friend class MagicMoveGen;

    /**
     * @brief Initializes the knight move lookup table
     */
    static void initKnightMoves();

    /**
     * @brief Initializes the king move lookup table
     */
    static void initKingMoves();

    /**
     * @brief Initializes the pawn move lookup tables
     */
    static void initPawnMoves();

    /**
     * @brief Initializes the bishop mask lookup tables
     */
    static void initBishopMasks();

    /**
     * @brief Initializes the rook mask lookup table
     */
    static void initRookMasks();
  };
}