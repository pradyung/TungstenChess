#include "../../Headers/transposition_table.hpp"

namespace Chess
{
  TranspositionTable::TranspositionTable()
  {
    table = std::map<ZobristKey, int>();
  }

  void TranspositionTable::store(ZobristKey key, int depth, int score)
  {
    table[key] = (score << 4) | depth;
  }

  bool TranspositionTable::probe(ZobristKey key, int depth, int *score)
  {
    if (table.find(key) == table.end())
    {
      return false;
    }

    int entry = table[key];

    *score = entry >> 4;

    return (entry & 0xF) >= depth;
  }

  void TranspositionTable::clear()
  {
    table.clear();
  }
}
