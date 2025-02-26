#pragma once

#include "types.hpp"
#include "zobrist.hpp"

#include <array>
#include <vector>

namespace TungstenChess
{
  class TranspositionTable
  {
  public:
    struct Entry
    {
      bool isOccupied;
      ZobristKey key;
      int evaluation;
      int depth;
      bool quiesce;
    };

    static constexpr int TABLE_SIZE = 1 << 20;

  private:
    std::vector<Entry> m_transpositionTable;
    int m_occupied = 0;

  public:
    TranspositionTable() : m_transpositionTable(TABLE_SIZE) {}

    int occupied() const
    {
      return m_occupied;
    }

    bool hasEntry(ZobristKey key) const
    {
      return m_transpositionTable[key % TABLE_SIZE].key == key;
    }

    Entry retrieve(ZobristKey key, bool &found)
    {
      Entry &entry = m_transpositionTable[key % TABLE_SIZE];
      found = entry.key == key;
      return entry;
    }

    void store(ZobristKey key, int evaluation, int depth, bool quiesce)
    {
      Entry &entry = m_transpositionTable[key % TABLE_SIZE];
      if (entry.isOccupied == 0)
        m_occupied++;
      entry = {true, key, evaluation, depth, quiesce};
    }
  };
}