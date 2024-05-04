#pragma once

#include <vector>
#include <fstream>

#include "types.hpp"

namespace Chess
{
  class Openings
  {
  public:
    bool inOpeningBook = true;

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::string &path, uint openingBookSize)
    {
      std::ifstream file(path);

      openingBook.resize(openingBookSize);

      // read the file 4 bytes at a time into the book array
      for (int i = 0; i < openingBookSize; i++)
      {
        file.read((char *)&openingBook[i], sizeof(uint));
      }

      file.close();
    }

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(MoveInt move)
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
    MoveInt getNextMove() const { return getWeightedRandomMove(); }

  private:
    std::vector<uint> openingBook;

    std::vector<MoveInt> moves;

    int lastMoveIndex = INVALID;

    /**
     * @brief Gets the next possible "children" moves from the opening book
     */
    std::vector<MoveInt> getChildrenMoves() const
    {
      std::vector<MoveInt> childrenMoves;
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

    /**
     * @brief Gets a random next move, weighted by the frequency of the children moves
     */
    MoveInt getWeightedRandomMove() const
    {
      std::vector<MoveInt> childrenMoves = getChildrenMoves();

      if (childrenMoves.size() == 0)
        return INVALID_MOVE;

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