#include "core/moves_lookup/magic.hpp"

#include "utils/utils.hpp"
#include "core/moves_lookup/lookup.hpp"

namespace TungstenChess
{
  void MagicMoveGen::init()
  {
    static utils::once<false> initialized;
    if (initialized)
      return;

    MovesLookup::init();

    initRookLookupTables();
    initBishopLookupTables();
  }

  Bitboard MagicMoveGen::getBishopMoves(Square square, Bitboard allPieces)
  {
    Bitboard maskedPieces = allPieces & MovesLookup::BISHOP_MASKS[square];
    return BISHOP_LOOKUP_TABLES[square, (BISHOP_MAGICS[square] * (maskedPieces)) >> BISHOP_SHIFTS[square]];
  }

  Bitboard MagicMoveGen::getRookMoves(Square square, Bitboard allPieces)
  {
    Bitboard maskedPieces = allPieces & MovesLookup::ROOK_MASKS[square];
    return ROOK_LOOKUP_TABLES[square, (ROOK_MAGICS[square] * maskedPieces) >> ROOK_SHIFTS[square]];
  }

  void MagicMoveGen::getAllBlockers(std::vector<Bitboard> &blockers, Square square, Bitboard mask)
  {
    blockers.clear();

    std::vector<Square> setBits;

    for (Square i = 0; i < 64; i++)
      if (mask & Bitboards::bit(i))
        setBits.push_back(i);

    for (int i = 0; i < (1 << setBits.size()); i++)
    {
      Bitboard blocker = 0;

      for (size_t j = 0; j < setBits.size(); j++)
        if (i & (1 << j))
          blocker |= Bitboards::bit(setBits[j]);

      blockers.push_back(blocker);
    }
  }

  Bitboard MagicMoveGen::getRookMovesBitboard(Square square, Bitboard blockers)
  {
    Bitboard movesBitboard = 0;

    int directions[4] = {-8, -1, 1, 8};
    int rankEdges[4] = {0, -1, -1, 7};
    int fileEdges[4] = {-1, 0, 7, -1};

    for (int i = 0; i < 4; i++)
    {
      Square to = square;

      while (true)
      {
        movesBitboard |= Bitboards::bit(to);

        bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

        if (isEdge || (blockers & Bitboards::bit(to)))
          break;

        to += directions[i];
      }
    }

    return movesBitboard & ~Bitboards::bit(square);
  }

  Bitboard MagicMoveGen::getBishopMovesBitboard(Square square, Bitboard blockers)
  {
    Bitboard movesBitboard = 0;

    int directions[4] = {-9, -7, 7, 9};
    int rankEdges[4] = {0, 0, 7, 7};
    int fileEdges[4] = {0, 7, 0, 7};

    for (int i = 0; i < 4; i++)
    {
      Square to = square;

      while (true)
      {
        movesBitboard |= Bitboards::bit(to);

        bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

        if (isEdge || (blockers & Bitboards::bit(to)))
          break;

        to += directions[i];
      }
    }

    return movesBitboard & ~Bitboards::bit(square);
  }

  void MagicMoveGen::initRookLookupTables()
  {
    std::array<size_t, 64> rowSizes;
    for (Square square = 0; square < 64; square++)
    {
      rowSizes[square] = 1U << (64 - ROOK_SHIFTS[square]);
    }
    ROOK_LOOKUP_TABLES.reserve(rowSizes);

    std::vector<Bitboard> blockers;

    for (Square square = 0; square < 64; square++)
    {
      getAllBlockers(blockers, square, MovesLookup::ROOK_MASKS[square]);

      for (size_t i = 0; i < blockers.size(); i++)
      {
        Bitboard shiftedBlockers = (blockers[i] * ROOK_MAGICS[square]) >> ROOK_SHIFTS[square];
        ROOK_LOOKUP_TABLES[square, shiftedBlockers] = getRookMovesBitboard(square, blockers[i]);
      }
    }
  }

  void MagicMoveGen::initBishopLookupTables()
  {
    std::array<size_t, 64> rowSizes;
    for (Square square = 0; square < 64; square++)
    {
      rowSizes[square] = 1 << (64 - BISHOP_SHIFTS[square]);
    }
    BISHOP_LOOKUP_TABLES.reserve(rowSizes);

    std::vector<Bitboard> blockers;

    for (Square square = 0; square < 64; square++)
    {
      getAllBlockers(blockers, square, MovesLookup::BISHOP_MASKS[square]);

      for (size_t i = 0; i < blockers.size(); i++)
      {
        Bitboard shiftedBlockers = (blockers[i] * BISHOP_MAGICS[square]) >> BISHOP_SHIFTS[square];
        BISHOP_LOOKUP_TABLES[square, shiftedBlockers] = getBishopMovesBitboard(square, blockers[i]);
      }
    }
  }
}