#pragma once

#include <array>

namespace TungstenChess
{
  class MovesLookup
  {
  public:
    /**
     * @brief Get the instance of the MovesLookup singleton
     * @return MovesLookup&
     */
    static MovesLookup &getInstance()
    {
      static MovesLookup instance;
      return instance;
    }

    std::array<Bitboard, 64> KNIGHT_MOVES;
    std::array<Bitboard, 64> KING_MOVES;
    std::array<Bitboard, 64> WHITE_PAWN_CAPTURE_MOVES;
    std::array<Bitboard, 64> BLACK_PAWN_CAPTURE_MOVES;
    std::array<Bitboard, 64> BISHOP_MASKS;
    std::array<Bitboard, 64> ROOK_MASKS;

  private:
    /**
     * @brief Initializes the move lookup tables
     */
    MovesLookup()
    {
      initKnightMoves();
      initKingMoves();
      initPawnCaptureMoves();
      initBishopMasks();
      initRookMasks();
    }

    /**
     * @brief Initializes the knight move lookup table
     */
    void initKnightMoves()
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
    void initKingMoves()
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
     * @brief Initializes the pawn capture move lookup table
     */
    void initPawnCaptureMoves()
    {
      for (int square = 0; square < 64; square++)
      {
        Bitboard position = 1ULL << square;

        BLACK_PAWN_CAPTURE_MOVES[square] = 0ULL;
        WHITE_PAWN_CAPTURE_MOVES[square] = 0ULL;

        if (square < 56 && square % 8 > 0)
          BLACK_PAWN_CAPTURE_MOVES[square] |= position << 7;
        if (square < 56 && square % 8 < 7)
          BLACK_PAWN_CAPTURE_MOVES[square] |= position << 9;
        if (square > 7 && square % 8 > 0)
          WHITE_PAWN_CAPTURE_MOVES[square] |= position >> 9;
        if (square > 7 && square % 8 < 7)
          WHITE_PAWN_CAPTURE_MOVES[square] |= position >> 7;
      }
    }

    /**
     * @brief Initializes the bishop mask lookup tables
     */
    void initBishopMasks()
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
    void initRookMasks()
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