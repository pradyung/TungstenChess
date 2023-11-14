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

    OpeningBook openingBook;

    std::vector<int> moves;

    int lastMoveIndex;
    std::stack<int> lastMoveIndexStack;

    bool addMove(int move);
    void removeLastMove();

    int *getChildrenMoves();

    int getNextMove();

    int getWeightedRandomMove();
  };
}