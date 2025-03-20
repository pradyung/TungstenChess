#include "bot/transposition_table.hpp"

#include <format>

namespace TungstenChess
{
  using Entry = TranspositionTable::Entry;

  Entry::Entry(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
  {
    overwrite(key, searchId, evaluation, depth, quiesce);
  }

  void Entry::overwrite(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
  {
    m_searchId = searchId;
    m_key = key;
    m_evaluation = evaluation;
    m_depth = depth;
    m_quiesce = quiesce;
  }

  std::string TranspositionTable::occupancy() const
  {
    return std::format("{:.2f}/{} MB", m_occupied / (double)MEGABYTE * sizeof(Entry), TABLE_SIZE / (double)MEGABYTE * sizeof(Entry));
  }

  bool TranspositionTable::hasEntry(ZobristKey key) const
  {
    return m_transpositionTable[key % TABLE_SIZE].isSameKey(key);
  }

  const Entry &TranspositionTable::retrieve(ZobristKey key, bool &found)
  {
    Entry &entry = m_transpositionTable[key % TABLE_SIZE];
    found = entry.isSameKey(key);
    return entry;
  }

  void TranspositionTable::store(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
  {
    Entry &entry = m_transpositionTable[key % TABLE_SIZE];

    if (!entry.isOccupied())
      m_occupied++;

    // Prefer newer, deeper, non-quiesce entries
    if (!entry.isOccupied() || searchId - entry.searchId() >= 3 || depth > entry.depth() || (!quiesce && entry.quiesce()))
      entry.overwrite(key, searchId, evaluation, depth, quiesce);
  }
}