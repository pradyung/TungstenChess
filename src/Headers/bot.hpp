#pragma once

#include "board.hpp"
#include "opening_book.hpp"

namespace Chess
{
  const BotSettings DEFAULT_BOT_SETTINGS = {
      500, // max search time in ms
      3,   // min search depth
      5,   // max search depth
      10,  // quiesce depth
      1,   // use opening book
      1,   // log search info
      1,   // log PGN moves
      1    // fixed depth search
  };

  class Bot
  {
  public:
    Bot(Board &board, const BotSettings &settings = DEFAULT_BOT_SETTINGS) : board(board), botSettings(settings) {}

    int positionsEvaluated;
    int depthSearched;

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::string path, uint openingBookSize)
    {
      openingBook.loadOpeningBook(path, openingBookSize);
    }

    /**
     * @brief Generates the best move for the bot
     */
    Move generateBotMove();

  private:
    Board &board;
    OpeningBook openingBook;

    const BotSettings botSettings;

    /**
     * @brief Gets the legal moves for a color, sorted by heuristic evaluation
     * @param color The color to get the moves for
     * @param includeCastling Whether to include castling moves (should be false for quiescence search)
     */
    std::vector<Move> getSortedLegalMoves(int color, bool includeCastling = true)
    {
      return heuristicSortMoves(board.getLegalMoves(color, includeCastling));
    }

    /**
     * @brief Gets the positional evaluation of a single piece
     * @param pieceIndex The index of the piece
     * @param absolute Whether to return the value of the evaluation as if the piece was white (true for heuristic evaluation, false for static evaluation)
     */
    int getPiecePositionalEvaluation(int pieceIndex, bool absolute = false) const
    {
      int positionalEvaluation = 0;

      positionalEvaluation = PIECE_EVAL_TABLES[board[pieceIndex]][pieceIndex];

      if (!absolute && (board[pieceIndex] & BLACK))
        positionalEvaluation = -positionalEvaluation;

      return positionalEvaluation;
    }

    /**
     * @brief Generates a move from the integer representation, used for opening book parsing (see Move::toInt())
     * @param moveInt The integer representation of the move
     */
    Move generateMoveFromInt(MoveInt moveInt);

    /**
     * @brief Generates a move using a depth of 1 (not used unless the bot is set to depth 1)
     */
    Move generateOneDeepMove();

    /**
     * @brief Generates the best move for the bot
     * @param depth The depth to search to
     * @param alpha The alpha value for alpha-beta pruning (should not be set outside of recursive calls)
     * @param beta The beta value for alpha-beta pruning (should not be set outside of recursive calls)
     */
    Move generateBestMove(int depth, int alpha = NEGATIVE_INFINITY, int beta = POSITIVE_INFINITY);

    /**
     * @brief Uses iterative deepening to find the best move in a constant amount of time
     * @param time The time in milliseconds to search for (this time is not exact, but the bot will stop after a search is complete AND the time has run out.
     *             It will not stop in the middle of a search, so the actual time spent may be significantly longer than the time parameter)
     * @param start The time the search started, used to check if the time has run out
     */
    Move iterativeDeepening(int time, std::chrono::time_point<std::chrono::high_resolution_clock> start);

    /**
     * @brief Gets the static evaluation of the current position, from the perspective of the side to move
     */
    int getStaticEvaluation();

    /**
     * @brief Gets the material evaluation of the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getMaterialEvaluation();

    /**
     * @brief Gets the positional evaluation of the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getPositionalEvaluation();

    /**
     * @brief Gets the evaluation bonus for the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getEvaluationBonus();

    /**
     * @brief Negamax search with alpha-beta pruning and quiescence search
     * @param depth The depth to search to
     * @param alpha The alpha value for alpha-beta pruning
     * @param beta The beta value for alpha-beta pruning
     */
    int negamax(int depth, int alpha, int beta);

    /**
     * @brief Quiescence search
     * @param depth The depth to search to
     * @param alpha The alpha value for alpha-beta pruning
     * @param beta The beta value for alpha-beta pruning
     */
    int quiesce(int depth, int alpha, int beta);

    /**
     * @brief Heuristic evaluation of a move, used for move ordering to improve alpha-beta pruning
     * @param move The move to evaluate
     */
    int heuristicEvaluation(Move move);

    /**
     * @brief Sorts moves by heuristic evaluation to improve alpha-beta pruning
     * @param moves The moves to sort
     */
    std::vector<Move> heuristicSortMoves(std::vector<Move> moves);
  };
}