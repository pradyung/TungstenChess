#pragma once

#include "opening_book.hpp"
#include <vector>
#include <iostream>
#include <stack>

namespace Chess
{
  class Openings
  {
  public:
    Openings();

    bool addMove(int move);

    int getNextMove();

  private:
    OpeningBook openingBook;

    std::vector<int> moves;

    int lastMoveIndex;

    std::vector<int> getChildrenMoves();

    int getWeightedRandomMove();
  };
}