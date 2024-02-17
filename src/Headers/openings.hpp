#pragma once

#include <vector>
#include <fstream>

#include "types.hpp"

namespace Chess
{
  class Openings
  {
  public:
    Openings() : lastMoveIndex(INVALID) {}

    bool inOpeningBook = true;

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     */
    void loadOpeningBook(const std::string &path)
    {
      std::ifstream file(path);

      // read the file 4 bytes at a time into the book array
      for (int i = 0; i < 16552; i++)
      {
        file.read((char *)&openingBook[i], sizeof(int));
      }

      file.close();
    }

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(int move)
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

    /**
     * @brief Gets the next move from the opening book, randomly selected weighted by the frequency of the moves
     */
    int getNextMove() const { return getWeightedRandomMove(); }

  private:
    int openingBook[16552];

    std::vector<int> moves;

    int lastMoveIndex;

    std::vector<int> getChildrenMoves() const
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

    int getWeightedRandomMove() const
    {
      std::vector<int> childrenMoves = getChildrenMoves();

      if (childrenMoves.size() == 0)
        return INVALID;

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
  };
}