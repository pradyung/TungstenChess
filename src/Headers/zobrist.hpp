#pragma once

#include <random>

#include "piece.hpp"

typedef unsigned long long ZobristKey;

namespace Chess
{
  class Zobrist
  {
  public:
    Zobrist();
    ~Zobrist();

    ZobristKey **pieceKeys;
    ZobristKey *castlingKeys;
    ZobristKey *enPassantKeys;
    ZobristKey sideKey;

    void init();

    ZobristKey getInitialHash(int board[64], int castlingRights, int enPassantFile, int sideToMove);
  };
}