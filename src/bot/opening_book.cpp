#include "bot/opening_book.hpp"

namespace TungstenChess
{
  void OpeningBook::loadOpeningBook(const std::filesystem::path &path)
  {
    std::ifstream file(path);

    Zobrist::init();

    ZobristKey key = 0;
    for (int i = 0; i < 64; i++)
    {
      uint8_t rawPiece;
      file.read((char *)&rawPiece, 1);

      if (rawPiece == 0)
        continue;

      Piece piece = (rawPiece & TYPE) | (WHITE << (rawPiece >> 3));
      key ^= Zobrist::pieceKeys[piece, i];
    }

    uint8_t castlingRights;
    file.read((char *)&castlingRights, 1);
    key ^= Zobrist::castlingKeys[castlingRights];

    uint8_t enPassantFile;
    file.read((char *)&enPassantFile, 1);
    key ^= Zobrist::enPassantKeys[enPassantFile];

    uint8_t sideToMove;
    file.read((char *)&sideToMove, 1);
    if (sideToMove == 0)
      key ^= Zobrist::sideKey;

    m_inOpeningBook = (key == m_startingZobristKey);

    uint openingBookSize;
    file.read((char *)&openingBookSize, 4);

    uint8_t numBytesPerMove;
    file.read((char *)&numBytesPerMove, 1);

    m_moveFrequencyShift = 12;

    file.read((char *)&m_moveDepthShift, 1);
    m_moveDepthShift += m_moveFrequencyShift;

    m_moveNextMoveShift = m_moveDepthShift + 4;

    m_moveMask = (1 << m_moveFrequencyShift) - 1;
    m_moveFrequencyMask = (1 << (m_moveDepthShift - m_moveFrequencyShift)) - 1;
    m_moveDepthMask = 0x7;
    m_moveNoNextMove = (1ULL << (numBytesPerMove * 8 - m_moveNextMoveShift)) - 1;

    m_openingBook.resize(openingBookSize);

    for (size_t i = 0; i < openingBookSize; i++)
    {
      m_openingBook[i] = 0;
      file.read((char *)&m_openingBook[i], numBytesPerMove);
    }

    file.close();
  }

  bool OpeningBook::updateMoveHistory(const MoveStack &newMoves)
  {
    if (!m_inOpeningBook)
      return false;

    for (size_t i = m_moves.size(); i < newMoves.size() && m_inOpeningBook; i++)
      m_inOpeningBook = addMove(newMoves[i]);

    return m_inOpeningBook;
  }

  bool OpeningBook::addMove(Move move)
  {
    if (m_lastMoveIndex != -1)
    {
      uint64_t nextMove = getMoveNextMove(m_openingBook[m_lastMoveIndex]);
      if (nextMove == m_moveNoNextMove || nextMove == m_lastMoveIndex + 1)
        return false;
    }

    for (uint64_t i = m_lastMoveIndex + 1; i != m_moveNoNextMove; i = getMoveNextMove(m_openingBook[i]))
    {
      if (getMove(m_openingBook[i]) == move)
      {
        m_lastMoveIndex = i;

        m_moves.push_back(move);
        return true;
      }
    }

    return false;
  }

  Move OpeningBook::getNextMove() const
  {
    return getWeightedRandomMove();
  }

  void OpeningBook::getChildrenMoves(std::vector<size_t> &childrenMoves) const
  {
    for (uint64_t i = m_lastMoveIndex + 1; i != m_moveNoNextMove; i = getMoveNextMove(m_openingBook[i]))
    {
      childrenMoves.push_back(i);
    }
  }

  Move OpeningBook::getWeightedRandomMove() const
  {
    std::vector<size_t> childrenMoves;
    getChildrenMoves(childrenMoves);

    if (childrenMoves.size() == 0)
      return NULL_MOVE;

    if (childrenMoves.size() == 1)
      return getMove(m_openingBook[childrenMoves[0]]);

    int totalWeight = 0;
    for (size_t i : childrenMoves)
    {
      totalWeight += getMoveFrequency(m_openingBook[i]);
    }

    int randomWeight = rand() % totalWeight;

    int currentWeight = 0;

    for (size_t i : childrenMoves)
    {
      currentWeight += getMoveFrequency(m_openingBook[i]);

      if (currentWeight > randomWeight)
      {
        return getMove(m_openingBook[i]);
      }
    }

    return 0;
  }
}