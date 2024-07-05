#pragma once

#include <array>

#include "bitboard.hpp"

namespace TungstenChess
{
  class MovesLookup
  {
  public:
    static inline std::array<Bitboard, 64> KNIGHT_MOVES = {};
    static inline std::array<Bitboard, 64> KING_MOVES = {};
    static inline std::array<Bitboard, 64> BISHOP_MASKS = {};
    static inline std::array<Bitboard, 64> ROOK_MASKS = {};

    static inline std::array<std::array<Bitboard, 64>, BLACK_PAWN + 1> PAWN_CAPTURE_MOVES = {};
    static inline std::array<std::array<Bitboard, 64>, BLACK_PAWN + 1> PAWN_REVERSE_SINGLE_MOVES = {};
    static inline std::array<std::array<Bitboard, 64>, BLACK_PAWN + 1> PAWN_REVERSE_DOUBLE_MOVES = {};

    /**
     * @brief Initializes the move lookup tables
     */
    static void init()
    {
      static bool initialized = false;

      if (initialized)
        return;
      initialized = true;

      initKnightMoves();
      initKingMoves();
      initPawnMoves();
      initBishopMasks();
      initRookMasks();
    }

  private:
    /**
     * @brief Initializes the knight move lookup table
     */
    static void initKnightMoves()
    {
      for (int square = 0; square < 64; square++)
      {
        Bitboard position = 1ULL << square;

        KNIGHT_MOVES[square] = 0ULL;

        if (square < 48 && square % 8 > 0)
          KNIGHT_MOVES[square] |= position << 15;
        if (square < 48 && square % 8 < 7)
          KNIGHT_MOVES[square] |= position << 17;
        if (square < 56 && square % 8 > 1)
          KNIGHT_MOVES[square] |= position << 6;
        if (square < 56 && square % 8 < 6)
          KNIGHT_MOVES[square] |= position << 10;
        if (square > 15 && square % 8 > 0)
          KNIGHT_MOVES[square] |= position >> 17;
        if (square > 15 && square % 8 < 7)
          KNIGHT_MOVES[square] |= position >> 15;
        if (square > 7 && square % 8 > 1)
          KNIGHT_MOVES[square] |= position >> 10;
        if (square > 7 && square % 8 < 6)
          KNIGHT_MOVES[square] |= position >> 6;
      }
    }

    /**
     * @brief Initializes the king move lookup table
     */
    static void initKingMoves()
    {
      for (int square = 0; square < 64; square++)
      {
        Bitboard position = 1ULL << square;

        KING_MOVES[square] = 0ULL;

        int offsets[8] = {-1, 0, 1};
        int rank = square / 8;
        int file = square % 8;

        for (int dr = 0; dr < 3; dr++)
        {
          for (int df = 0; df < 3; df++)
          {
            int toRank = rank + offsets[dr];
            int toFile = file + offsets[df];

            if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7 || (dr == 1 && df == 1))
              continue;

            KING_MOVES[square] |= 1ULL << (toRank * 8 + toFile);
          }
        }
      }
    }

    /**
     * @brief Initializes the pawn move lookup tables
     */
    static void initPawnMoves()
    {
      for (int square = 0; square < 64; square++)
      {
        Bitboard position = 1ULL << square;

        PAWN_CAPTURE_MOVES[WHITE_PAWN][square] = 0ULL;
        PAWN_CAPTURE_MOVES[BLACK_PAWN][square] = 0ULL;

        if (square > 7 && square % 8 > 0)
          PAWN_CAPTURE_MOVES[WHITE_PAWN][square] |= position >> 9;
        if (square > 7 && square % 8 < 7)
          PAWN_CAPTURE_MOVES[WHITE_PAWN][square] |= position >> 7;
        if (square < 56 && square % 8 > 0)
          PAWN_CAPTURE_MOVES[BLACK_PAWN][square] |= position << 7;
        if (square < 56 && square % 8 < 7)
          PAWN_CAPTURE_MOVES[BLACK_PAWN][square] |= position << 9;

        PAWN_REVERSE_SINGLE_MOVES[WHITE_PAWN][square] = position << 8;
        PAWN_REVERSE_SINGLE_MOVES[BLACK_PAWN][square] = position >> 8;

        PAWN_REVERSE_DOUBLE_MOVES[WHITE_PAWN][square] = 0ULL;
        PAWN_REVERSE_DOUBLE_MOVES[BLACK_PAWN][square] = 0ULL;

        if (square / 8 == 4)
          PAWN_REVERSE_DOUBLE_MOVES[WHITE_PAWN][square] = position << 16;
        else if (square / 8 == 3)
          PAWN_REVERSE_DOUBLE_MOVES[BLACK_PAWN][square] = position >> 16;
      }

      PAWN_CAPTURE_MOVES[WHITE] = PAWN_CAPTURE_MOVES[WHITE_PAWN];
      PAWN_CAPTURE_MOVES[BLACK] = PAWN_CAPTURE_MOVES[BLACK_PAWN];

      PAWN_REVERSE_SINGLE_MOVES[WHITE] = PAWN_REVERSE_SINGLE_MOVES[WHITE_PAWN];
      PAWN_REVERSE_SINGLE_MOVES[BLACK] = PAWN_REVERSE_SINGLE_MOVES[BLACK_PAWN];

      PAWN_REVERSE_DOUBLE_MOVES[WHITE] = PAWN_REVERSE_DOUBLE_MOVES[WHITE_PAWN];
      PAWN_REVERSE_DOUBLE_MOVES[BLACK] = PAWN_REVERSE_DOUBLE_MOVES[BLACK_PAWN];
    }

    /**
     * @brief Initializes the bishop mask lookup tables
     */
    static void initBishopMasks()
    {
      for (int square = 0; square < 64; square++)
      {
        BISHOP_MASKS[square] = 0;

        int directions[4] = {-9, -7, 7, 9};
        int rankEdges[4] = {0, 0, 7, 7};
        int fileEdges[4] = {0, 7, 0, 7};

        for (int i = 0; i < 4; i++)
        {
          int to = square;

          while (true)
          {
            if ((to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]))
              break;

            BISHOP_MASKS[square] |= 1ULL << to;

            to += directions[i];
          }
        }

        BISHOP_MASKS[square] &= ~(1ULL << square);
      }
    }

    /**
     * @brief Initializes the rook mask lookup table
     */
    static void initRookMasks()
    {
      for (int square = 0; square < 64; square++)
      {
        ROOK_MASKS[square] = 0;

        for (int i = 0; i < 4; i++)
        {
          int to = square;

          int directions[4] = {-8, -1, 1, 8};
          int rankEdges[4] = {0, -1, -1, 7};
          int fileEdges[4] = {-1, 0, 7, -1};

          while (true)
          {
            if ((to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]))
              break;

            ROOK_MASKS[square] |= 1ULL << to;

            to += directions[i];
          }
        }

        ROOK_MASKS[square] &= ~(1ULL << square);
      }
    }
  };
}