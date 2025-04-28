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
    return BISHOP_LOOKUP_TABLES[square][(BISHOP_MAGICS[square] * (maskedPieces)) >> BISHOP_SHIFTS[square]];
  }

  Bitboard MagicMoveGen::getRookMoves(Square square, Bitboard allPieces)
  {
    Bitboard maskedPieces = allPieces & MovesLookup::ROOK_MASKS[square];
    return ROOK_LOOKUP_TABLES[square][(ROOK_MAGICS[square] * maskedPieces) >> ROOK_SHIFTS[square]];
  }

  void MagicMoveGen::getAllBlockers(std::vector<Bitboard> &blockers, Square square, Bitboard mask)
  {
    std::vector<Square> setBits;

    for (Square i = 0; i < 64; i++)
      if (mask & (1ULL << i))
        setBits.push_back(i);

    for (int i = 0; i < (1 << setBits.size()); i++)
    {
      Bitboard blocker = 0;

      for (size_t j = 0; j < setBits.size(); j++)
        if (i & (1 << j))
          blocker |= 1ULL << setBits[j];

      blockers.push_back(blocker);
    }
  }

  void MagicMoveGen::shiftBlockers(std::vector<Bitboard> &blockers, Magic magic, Shift shift, Square square)
  {
    for (size_t i = 0; i < blockers.size(); i++)
      blockers[i] = (magic * blockers[i]) >> shift;
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
        movesBitboard |= 1ULL << to;

        bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

        if (isEdge || (blockers & (1ULL << to)))
          break;

        to += directions[i];
      }
    }

    return movesBitboard & ~(1ULL << square);
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
        movesBitboard |= 1ULL << to;

        bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

        if (isEdge || (blockers & (1ULL << to)))
          break;

        to += directions[i];
      }
    }

    return movesBitboard & ~(1ULL << square);
  }

  void MagicMoveGen::getAllMovesBitboards(std::vector<Bitboard> &moves, Square square, std::vector<Bitboard> blockers, bool rook)
  {
    for (size_t i = 0; i < blockers.size(); i++)
    {
      if (rook)
        moves.push_back(getRookMovesBitboard(square, blockers[i]));
      else
        moves.push_back(getBishopMovesBitboard(square, blockers[i]));
    }
  }

  void MagicMoveGen::getMovesLookupTable(std::vector<Bitboard> &lookupTable, Square square, bool rook)
  {
    Magic magic = rook ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
    Shift shift = rook ? ROOK_SHIFTS[square] : BISHOP_SHIFTS[square];

    std::vector<Bitboard> blockers;
    getAllBlockers(blockers, square, rook ? MovesLookup::ROOK_MASKS[square] : MovesLookup::BISHOP_MASKS[square]);

    std::vector<Bitboard> moves;
    getAllMovesBitboards(moves, square, blockers, rook);

    shiftBlockers(blockers, magic, shift, square);

    lookupTable.resize(1 << (64 - shift), 0);

    for (size_t i = 0; i < blockers.size(); i++)
      lookupTable[blockers[i]] = moves[i];
  }

  void MagicMoveGen::initRookLookupTables()
  {
    for (Square i = 0; i < 64; i++)
    {
      getMovesLookupTable(ROOK_LOOKUP_TABLES[i], i, true);
    }
  }

  void MagicMoveGen::initBishopLookupTables()
  {
    for (Square i = 0; i < 64; i++)
    {
      getMovesLookupTable(BISHOP_LOOKUP_TABLES[i], i, false);
    }
  }
}