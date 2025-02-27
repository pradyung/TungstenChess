#pragma once

#include "types.hpp"
#include "zobrist.hpp"

#include <array>
#include <vector>

#define MEGABYTE 1048576

namespace TungstenChess
{
  class TranspositionTable
  {
  public:
    class Entry
    {
    private:
      ZobristKey m_key;
      uint m_searchId : 12;
      int m_evaluation : 21;
      uint m_depth : 6;
      bool m_quiesce : 1;

    public:
      Entry() = default;

      Entry(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
      {
        overwrite(key, searchId, evaluation, depth, quiesce);
      }

      void overwrite(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
      {
        m_searchId = searchId;
        m_key = key;
        m_evaluation = evaluation;
        m_depth = depth;
        m_quiesce = quiesce;
      }

      bool isOccupied() const { return m_key; }
      ZobristKey key() const { return m_key; }
      uint searchId() const { return m_searchId; }
      int evaluation() const { return m_evaluation; }
      int depth() const { return m_depth; }
      bool quiesce() const { return m_quiesce; }

      bool isSameKey(ZobristKey key) const { return m_key == key; }
    };

    static constexpr int TABLE_SIZE = 64 * MEGABYTE / sizeof(Entry);

  private:
    std::vector<Entry> m_transpositionTable;
    int m_occupied = 0;

  public:
    TranspositionTable() : m_transpositionTable(TABLE_SIZE) {}

    std::string occupancy() const
    {
      return std::format("{:.2f}/{} MB", m_occupied / (double)MEGABYTE * sizeof(Entry), TABLE_SIZE / (double)MEGABYTE * sizeof(Entry));
    }

    bool hasEntry(ZobristKey key) const
    {
      return m_transpositionTable[key % TABLE_SIZE].isSameKey(key);
    }

    const Entry &retrieve(ZobristKey key, bool &found)
    {
      Entry &entry = m_transpositionTable[key % TABLE_SIZE];
      found = entry.isSameKey(key);
      return entry;
    }

    void store(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
    {
      Entry &entry = m_transpositionTable[key % TABLE_SIZE];

      if (!entry.isOccupied())
        m_occupied++;

      // Prefer newer, deeper, non-quiesce entries
      if (!entry.isOccupied() || searchId > entry.searchId() || depth > entry.depth() || (!quiesce && entry.quiesce()))
        entry.overwrite(key, searchId, evaluation, depth, quiesce);
    }
  };
}