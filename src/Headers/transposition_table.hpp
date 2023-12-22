#pragma once

#include <map>

namespace Chess
{
  class TranspositionTable
  {
  public:
    TranspositionTable();

    void store(unsigned long long key, int depth, int score);
    bool probe(unsigned long long key, int depth, int *score);

    void clear();

  private:
    std::map<unsigned long long, int> table;
  };
}