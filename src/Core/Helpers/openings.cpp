#include "../../Headers/openings.hpp"

namespace Chess
{
  Openings::Openings()
  {
    moves = std::vector<int>();
    lastMoveIndex = -1;
  }

  bool Openings::addMove(int move)
  {
    for (int i = lastMoveIndex + 1;; i++)
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

  std::vector<int> Openings::getChildrenMoves()
  {
    std::vector<int> childrenMoves;
    int childrenMovesIndex = 0;

    for (int i = lastMoveIndex + 1;; i++)
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

  int Openings::getNextMove()
  {
    return getWeightedRandomMove();
  }

  int Openings::getWeightedRandomMove()
  {
    std::vector<int> childrenMoves = getChildrenMoves();

    int totalWeight = 0;

    for (int i = 0; i < childrenMoves.size(); i++)
    {
      totalWeight += openingBook[lastMoveIndex + 1 + i] >> 12 & 0x1FFF;
    }

    int randomWeight = rand() % totalWeight;

    int currentWeight = 0;

    for (int i = 0; i < childrenMoves.size(); i++)
    {
      currentWeight += openingBook[lastMoveIndex + 1 + i] >> 12 & 0x1FFF;

      if (currentWeight > randomWeight)
      {
        return childrenMoves[i];
      }
    }

    return 0;
  }
}