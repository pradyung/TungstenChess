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

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(int move);

    /**
     * @brief Gets the next move from the opening book, randomly selected weighted by the frequency of the moves
     */
    int getNextMove();

  private:
    OpeningBook openingBook;

    std::vector<int> moves;

    int lastMoveIndex;

    std::vector<int> getChildrenMoves();

    int getWeightedRandomMove();
  };
}