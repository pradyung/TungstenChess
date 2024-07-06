#pragma once

#include <iostream>
#include <chrono>
#include <algorithm>

#include "board.hpp"
#include "opening_book.hpp"
#include "piece_eval_tables.hpp"

#define POSITIVE_INFINITY 1000000
#define NEGATIVE_INFINITY -1000000

namespace TungstenChess
{
  struct BotSettings
  {
    int maxSearchTime = 500;     // In milliseconds, not a hard limit
    int minSearchDepth = 3;      // for iterative deepening
    int maxSearchDepth = 5;      // for fixed depth search
    int quiesceDepth = -1;       // for quiescence search, set to -1 to search indefinitely (recommended)
    bool useOpeningBook = false; // only used if board starting position is default
    bool logSearchInfo = true;
    bool logPGNMoves = true;      // as opposed to UCI moves
    bool fixedDepthSearch = true; // as opposed to iterative deepening
  };

  enum EvaluationBonus
  {
    BISHOP_PAIR_BONUS = 100,
    CASTLED_KING_BONUS = 25,
    CAN_CASTLE_BONUS = 25,
    ROOK_ON_OPEN_FILE_BONUS = 50,
    ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
    KNIGHT_OUTPOST_BONUS = 50,
    PASSED_PAWN_BONUS = 50,
    DOUBLED_PAWN_PENALTY = 50,
    ISOLATED_PAWN_PENALTY = 25,
    BACKWARDS_PAWN_PENALTY = 50,
    KING_SAFETY_PAWN_SHIELD_BONUS = 50,
    STALEMATE_PENALTY = 150,
  };

  class Bot
  {
  public:
    Bot(Board &board, const BotSettings &settings) : board(board), botSettings(settings)
    {
      openingBook.inOpeningBook = board.isDefaultStartPosition();
    }

    Bot(Board &board) : Bot(board, BotSettings()) {}

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
     * @param onlyCaptures Whether to only get captures
     */
    std::vector<Move> getSortedLegalMoves(PieceColor color, bool onlyCaptures = false)
    {
      std::vector<Move> moves = board.getLegalMoves(color, onlyCaptures);
      heuristicSortMoves(moves);
      return moves;
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
     * @brief Generates the best move for the bot
     * @param depth The depth to search to
     * @param alpha The alpha value for alpha-beta pruning (should not be set outside of recursive calls)
     * @param beta The beta value for alpha-beta pruning (should not be set outside of recursive calls)
     */
    Move generateBestMove(int depth);

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
    int getMaterialEvaluation() const;

    /**
     * @brief Gets the positional evaluation of the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getPositionalEvaluation() const;

    /**
     * @brief Gets the evaluation bonus for the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getEvaluationBonus() const;

    /**
     * @brief Negamax search with alpha-beta pruning and quiescence search
     * @param depth The depth to search to
     * @param alpha The alpha value for alpha-beta pruning
     * @param beta The beta value for alpha-beta pruning
     * @param quiesce Whether the search is in quiescence mode (captures only)
     */
    int negamax(int depth, int alpha = NEGATIVE_INFINITY, int beta = POSITIVE_INFINITY, bool quiesce = false);

    /**
     * @brief Heuristic evaluation of a move, used for move ordering to improve alpha-beta pruning
     * @param move The move to evaluate
     */
    int heuristicEvaluation(Move move);

    /**
     * @brief Sorts moves by heuristic evaluation (in place) to improve alpha-beta pruning
     * @param moves The moves to sort
     */
    void heuristicSortMoves(std::vector<Move> &moves);
  };
}