#pragma once

#include "types.hpp"
#include "Data/move_gen_helpers.hpp"
#include "bitboard.hpp"

namespace Chess
{
  class MagicMoveGen
  {
  public:
    std::array<std::vector<BitboardInt>, 64> ROOK_LOOKUP_TABLES;
    std::array<std::vector<BitboardInt>, 64> BISHOP_LOOKUP_TABLES;

    MagicMoveGen(MovesLookup &movesLookup)
    {
      initRookLookupTables(movesLookup);
      initBishopLookupTables(movesLookup);
    }

    std::vector<BitboardInt> getAllBlockers(int square, BitboardInt mask)
    {
      std::vector<int> setBits = std::vector<int>();

      for (int i = 0; i < 64; i++)
        if (mask & (1ULL << i))
          setBits.push_back(i);

      std::vector<BitboardInt> blockers = std::vector<BitboardInt>();

      for (int i = 0; i < (1 << setBits.size()); i++)
      {
        BitboardInt blocker = 0;

        for (int j = 0; j < setBits.size(); j++)
          if (i & (1 << j))
            blocker |= 1ULL << setBits[j];

        blockers.push_back(blocker);
      }

      return blockers;
    }

    std::vector<BitboardInt> getShiftedBlockers(Magic magic, Shift shift, int square, std::vector<BitboardInt> blocks)
    {
      std::vector<BitboardInt> shifts = std::vector<BitboardInt>();

      for (int i = 0; i < blocks.size(); i++)
        shifts.push_back((magic * blocks[i]) >> shift);

      return shifts;
    }

    BitboardInt getRookMovesBitboard(int square, BitboardInt blockers)
    {
      BitboardInt movesBitboard = 0;

      int directions[4] = {-8, -1, 1, 8};
      int rankEdges[4] = {0, -1, -1, 7};
      int fileEdges[4] = {-1, 0, 7, -1};

      for (int i = 0; i < 4; i++)
      {
        int to = square;

        while (true)
        {
          movesBitboard |= 1ULL << to;

          bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

          if (isEdge || (blockers & (1ULL << to)))
            break;

          to += directions[i];
        }
      }

      return movesBitboard & ~(1ULL << square);
    }

    BitboardInt getBishopMovesBitboard(int square, BitboardInt blockers)
    {
      BitboardInt movesBitboard = 0;

      int directions[4] = {-9, -7, 7, 9};
      int rankEdges[4] = {0, 0, 7, 7};
      int fileEdges[4] = {0, 7, 0, 7};

      for (int i = 0; i < 4; i++)
      {
        int to = square;

        while (true)
        {
          movesBitboard |= 1ULL << to;

          bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

          if (isEdge || (blockers & (1ULL << to)))
            break;

          to += directions[i];
        }
      }

      return movesBitboard & ~(1ULL << square);
    }

    std::vector<BitboardInt> getAllMovesBitboards(int square, std::vector<BitboardInt> blockers, bool rook)
    {
      std::vector<BitboardInt> moves = std::vector<BitboardInt>();

      for (int i = 0; i < blockers.size(); i++)
      {
        if (rook)
          moves.push_back(getRookMovesBitboard(square, blockers[i]));
        else
          moves.push_back(getBishopMovesBitboard(square, blockers[i]));
      }

      return moves;
    }

    std::vector<BitboardInt> getMovesLookupTable(MovesLookup &movesLookup, int square, bool rook)
    {
      Magic magic = rook ? movesLookup.ROOK_MAGICS[square] : movesLookup.BISHOP_MAGICS[square];
      Shift shift = rook ? movesLookup.ROOK_SHIFTS[square] : movesLookup.BISHOP_SHIFTS[square];

      std::vector<BitboardInt> blocks = getAllBlockers(square, rook ? movesLookup.ROOK_MASKS[square] : movesLookup.BISHOP_MASKS[square]);
      std::vector<BitboardInt> shifts = getShiftedBlockers(magic, shift, square, blocks);
      std::vector<BitboardInt> moves = getAllMovesBitboards(square, blocks, rook);

      BitboardInt maxShift = 0;
      for (int i = 0; i < shifts.size(); i++)
        maxShift = std::max(maxShift, shifts[i]);

      std::vector<BitboardInt> lookupTable = std::vector<BitboardInt>(maxShift + 1, 0);

      for (int i = 0; i < shifts.size(); i++)
        lookupTable[shifts[i]] = moves[i];

      return lookupTable;
    }

    void initRookLookupTables(MovesLookup &movesLookup)
    {
      ROOK_LOOKUP_TABLES = std::array<std::vector<BitboardInt>, 64>();

      for (int i = 0; i < 64; i++)
      {
        ROOK_LOOKUP_TABLES[i] = getMovesLookupTable(movesLookup, i, true);
      }
    }

    void initBishopLookupTables(MovesLookup &movesLookup)
    {
      BISHOP_LOOKUP_TABLES = std::array<std::vector<BitboardInt>, 64>();

      for (int i = 0; i < 64; i++)
      {
        BISHOP_LOOKUP_TABLES[i] = getMovesLookupTable(movesLookup, i, false);
      }
    }
  };
}