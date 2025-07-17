#pragma once

#include <fstream>
#include <vector>

#include "core/move.hpp"
#include "core/zobrist.hpp"

namespace TungstenChess
{
  typedef uint64_t OpeningBookMove;

  class OpeningBook
  {
  private:
    std::vector<OpeningBookMove> m_openingBook;
    std::vector<Move> m_moves;

    ZobristKey m_startingZobristKey;

    bool m_inOpeningBook = true;
    int m_lastMoveIndex = -1;

  public:
    OpeningBook(ZobristKey startingZobristKey) : m_startingZobristKey(startingZobristKey) {}

    bool isInOpeningBook() const { return m_inOpeningBook; }

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     */
    void loadOpeningBook(const std::filesystem::path &path);

    /**
     * @brief Adds a move to the move history
     * @param move The move to add
     * @return Whether the move was added successfully - if false, the move is not in the opening book
     */
    bool addMove(Move move);

    /**
     * @brief Gets the next move from the opening book, randomly selected weighted by the frequency of the moves
     */
    Move getNextMove() const;

  private:
    uint8_t m_moveFrequencyShift, m_moveDepthShift, m_moveNextMoveShift;
    uint64_t m_moveMask, m_moveFrequencyMask, m_moveDepthMask;
    uint64_t m_moveNoNextMove;

    uint getMove(OpeningBookMove move) const { return move & m_moveMask; }
    uint getMoveFrequency(OpeningBookMove move) const { return (move >> m_moveFrequencyShift) & m_moveFrequencyMask; }
    uint getMoveDepth(OpeningBookMove move) const { return (move >> m_moveDepthShift) & m_moveDepthMask; }
    uint getMoveNextMove(OpeningBookMove move) const { return move >> m_moveNextMoveShift; }

    /**
     * @brief Gets the next possible "children" moves (as indices) from the opening book
     * @param childrenMoves The vector to store the move indices in
     */
    void getChildrenMoves(std::vector<size_t> &childrenMoves) const;

    /**
     * @brief Gets a random next move, weighted by the frequency of the children moves
     */
    Move getWeightedRandomMove() const;
  };
}