#pragma once

#include <vector>
#include <fstream>

#include "utils/types.hpp"
#include "core/move.hpp"

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
    OpeningBook(bool inOpeningBook) : m_inOpeningBook(inOpeningBook) {}

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::filesystem::path &path, uint openingBookSize);

    /**
     * @brief Updates move history to synchronize with given vector
     * @param moves The moves to update the history with
     * @return Whether the moves were added successfully - if false, the moves are not in the opening book
     */
    bool updateMoveHistory(const std::vector<Move> &newMoves);

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
    /**
     * @brief Gets the next possible "children" moves from the opening book
     * @param childrenMoves The vector to store the moves in
     */
    void getChildrenMoves(std::vector<Move> &childrenMoves) const;

    /**
     * @brief Gets a random next move, weighted by the frequency of the children moves
     */
    Move getWeightedRandomMove() const;
  };
}