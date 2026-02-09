#include "bot/transposition_table.hpp"

#include <iomanip>
#include <sstream>

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
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2)
       << (m_occupied * sizeof(Entry) / (double)MEGABYTE)
       << " MB / "
       << (TABLE_SIZE * sizeof(Entry) / (double)MEGABYTE)
       << " MB";
    return ss.str();
  }

  bool TranspositionTable::hasEntry(ZobristKey key) const
  {
    return m_transpositionTable[key % TABLE_SIZE].isSameKey(key);
  }

  const Entry& TranspositionTable::retrieve(ZobristKey key, bool& found)
  {
    Entry& entry = m_transpositionTable[key % TABLE_SIZE];
    found = entry.isSameKey(key);
    return entry;
  }

  void TranspositionTable::store(ZobristKey key, uint searchId, int evaluation, int depth, bool quiesce)
  {
    Entry& entry = m_transpositionTable[key % TABLE_SIZE];

    if (!entry.isOccupied())
      m_occupied++;

    // Prefer newer, deeper, non-quiesce entries
    if (!entry.isOccupied() ||
        searchId - entry.searchId() >= 3 ||
        depth > entry.depth() ||
        (!quiesce && entry.quiesce()))
      entry.overwrite(key, searchId, evaluation, depth, quiesce);
  }
}