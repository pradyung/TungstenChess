#include "bot/opening_book.hpp"

namespace TungstenChess
{
  void OpeningBook::loadOpeningBook(const std::filesystem::path &path, uint openingBookSize)
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

  bool OpeningBook::updateMoveHistory(const std::vector<Move> &newMoves)
  {
    if (!m_inOpeningBook)
      return false;

    for (size_t i = m_moves.size(); i < newMoves.size() && m_inOpeningBook; i++)
      m_inOpeningBook = addMove(newMoves[i]);

    return m_inOpeningBook;
  }

  bool OpeningBook::addMove(Move move)
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

  Move OpeningBook::getNextMove() const
  {
    return getWeightedRandomMove();
  }

  void OpeningBook::getChildrenMoves(std::vector<Move> &childrenMoves) const
  {
    for (size_t i = m_lastMoveIndex + 1; i < m_openingBook.size(); i++)
    {
      if (m_openingBook[i] >> 25 == m_moves.size())
        childrenMoves.push_back(m_openingBook[i] & 0xFFF);
      else if (m_openingBook[i] >> 25 == m_moves.size() - 1)
        break;
    }
  }

  Move OpeningBook::getWeightedRandomMove() const
  {
    std::vector<Move> childrenMoves;
    getChildrenMoves(childrenMoves);

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
}