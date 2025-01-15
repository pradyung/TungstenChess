#pragma once

#include <vector>
#include <fstream>

#include "types.hpp"

namespace TungstenChess
{
  class OpeningBook
  {
  private:
    std::vector<uint> m_openingBook;
    std::vector<Move> m_moves;

    bool m_inOpeningBook = true;
    int m_lastMoveIndex = -1;

  public:
    OpeningBook(bool m_inOpeningBook) : m_inOpeningBook(m_inOpeningBook) {}

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::string &path, uint openingBookSize)
    {
      std::ifstream file(path);

      m_openingBook.resize(openingBookSize);

      // read the file 4 bytes at a time into the book array
      for (size_t i = 0; i < openingBookSize; i++)
      {
        file.read((char *)&m_openingBook[i], sizeof(uint));
      }

      file.close();
    }

    /**
     * @brief Updates move history to synchronize with given vector
     * @param moves The moves to update the history with
     * @return Whether the moves were added successfully - if false, the moves are not in the opening book
     */
    bool updateMoveHistory(const std::vector<Move> &newMoves)
    {
      if (!m_inOpeningBook)
        return false;

      for (size_t i = m_moves.size(); i < newMoves.size() && m_inOpeningBook; i++)
        m_inOpeningBook = addMove(newMoves[i]);

      return m_inOpeningBook;
    }

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(Move move)
    {
      for (int i = m_lastMoveIndex + 1;; i++)
      {
        if (m_openingBook[i] >> 25 == m_moves.size() - 1)
        {
          return false;
        }
        if ((m_openingBook[i] & 0xFFF) == move && m_openingBook[i] >> 25 == m_moves.size())
        {
          m_lastMoveIndex = i;
          break;
        }
      }

      m_moves.push_back(move);

      return true;
    }

    /**
     * @brief Gets the next move from the opening book, randomly selected weighted by the frequency of the moves
     */
    Move getNextMove() const { return getWeightedRandomMove(); }

  private:
    /**
     * @brief Gets the next possible "children" moves from the opening book
     */
    std::vector<Move> getChildrenMoves() const
    {
      std::vector<Move> childrenMoves;

      for (size_t i = m_lastMoveIndex + 1; i < m_openingBook.size(); i++)
      {
        if (m_openingBook[i] >> 25 == m_moves.size())
          childrenMoves.push_back(m_openingBook[i] & 0xFFF);
        else if (m_openingBook[i] >> 25 == m_moves.size() - 1)
          break;
      }

      return childrenMoves;
    }

    /**
     * @brief Gets a random next move, weighted by the frequency of the children moves
     */
    Move getWeightedRandomMove() const
    {
      std::vector<Move> childrenMoves = getChildrenMoves();

      if (childrenMoves.size() == 0)
        return NULL_MOVE;

      int totalWeight = 0;

      for (size_t i = 0; i < childrenMoves.size(); i++)
      {
        totalWeight += m_openingBook[m_lastMoveIndex + 1 + i] >> 12 & 0x1FFF;
      }

      int randomWeight = rand() % totalWeight;

      int currentWeight = 0;

      for (size_t i = 0; i < childrenMoves.size(); i++)
      {
        currentWeight += m_openingBook[m_lastMoveIndex + 1 + i] >> 12 & 0x1FFF;

        if (currentWeight > randomWeight)
        {
          return childrenMoves[i];
        }
      }

      return 0;
    }
  };
}