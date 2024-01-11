#pragma once

#include <vector>

#include "Data/opening_book.hpp"

namespace Chess
{
  class Openings
  {
  public:
    Openings() : lastMoveIndex(-1) {}

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(MoveInt move)
    {
      for (int16_t i = lastMoveIndex + 1;; i++)
      {
        if (openingBook[i] >> 25 == moves.size() - 1)
        {
          return false;
        }
        if ((openingBook[i] & 0xFFF) == move && openingBook[i] >> 25 == moves.size())
        {
          lastMoveIndex = i;
          break;
        }
      }

      moves.push_back(move);

      return true;
    }

    /**
     * @brief Gets the next move from the opening book, randomly selected weighted by the frequency of the moves
     */
    MoveInt getNextMove() const { return getWeightedRandomMove(); }

  private:
    OpeningBook openingBook;

    std::vector<MoveInt> moves;

    int16_t lastMoveIndex;

    std::vector<MoveInt> getChildrenMoves() const
    {
      std::vector<MoveInt> childrenMoves;
      int16_t childrenMovesIndex = 0;

      for (int16_t i = lastMoveIndex + 1;; i++)
      {
        if (openingBook[i] >> 25 == moves.size())
        {
          childrenMoves.push_back(openingBook[i] & 0xFFF);
          childrenMovesIndex++;
        }
        else if (openingBook[i] >> 25 == moves.size() - 1)
        {
          break;
        }
      }

      return childrenMoves;
    }

    MoveInt getWeightedRandomMove() const
    {
      std::vector<MoveInt> childrenMoves = getChildrenMoves();

      if (childrenMoves.size() == 0)
        return -1;

      uint16_t totalWeight = 0;

      for (uint8_t i = 0; i < childrenMoves.size(); i++)
      {
        totalWeight += openingBook[lastMoveIndex + 1 + i] >> 12 & 0x1FFF;
      }

      uint16_t randomWeight = rand() % totalWeight;

      uint16_t currentWeight = 0;

      for (uint16_t i = 0; i < childrenMoves.size(); i++)
      {
        currentWeight += openingBook[lastMoveIndex + 1 + i] >> 12 & 0x1FFF;

        if (currentWeight > randomWeight)
        {
          return childrenMoves[i];
        }
      }

      return 0;
    }
  };
}