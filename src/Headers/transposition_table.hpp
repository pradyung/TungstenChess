#pragma once

#include <map>

#include "zobrist.hpp"

namespace Chess
{
  class TranspositionTable
  {
  public:
    TranspositionTable();

    /**
     * @brief Stores a score in the transposition table.
     */
    void store(ZobristKey key, int depth, int score);

    /**
     * @brief Probes the transposition table for a score. Populates the score parameter if a score is found.
     * @return Whether a score was found
     */
    bool probe(ZobristKey key, int depth, int *score);

    /**
     * @brief Clears the transposition table
     */
    void clear();

  private:
    std::map<ZobristKey, int> table;
  };
}