#pragma once

#include <vector>

#include "core/zobrist.hpp"

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

      Entry(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce);

      void overwrite(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce);

      bool isOccupied() const { return m_key; }
      ZobristKey key() const { return m_key; }
      uint searchId() const { return m_searchId; }
      int evaluation() const { return m_evaluation; }
      int depth() const { return m_depth; }
      bool quiesce() const { return m_quiesce; }

      bool isSameKey(ZobristKey key) const { return m_key == key; }
    };

  private:
    const int TABLE_SIZE;
    std::vector<Entry> m_transpositionTable;
    int m_occupied = 0;

  public:
    TranspositionTable(int sizeMB)
        : TABLE_SIZE(sizeMB * MEGABYTE / sizeof(Entry)),
          m_transpositionTable(TABLE_SIZE)
    {}

    std::string occupancy() const;

    bool hasEntry(ZobristKey key) const;

    const Entry& retrieve(ZobristKey key, bool& found);

    void store(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce);
  };
}