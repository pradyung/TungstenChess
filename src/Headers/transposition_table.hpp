#pragma once

#include <map>

#include "zobrist.hpp"

namespace Chess
{
  class TranspositionTable
  {
  public:
    TranspositionTable();

    void store(ZobristKey key, int depth, int score);
    bool probe(ZobristKey key, int depth, int *score);

    void clear();

  private:
    std::map<ZobristKey, int> table;
  };
}