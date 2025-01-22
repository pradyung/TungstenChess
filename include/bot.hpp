#pragma once

#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "board.hpp"
#include "opening_book.hpp"
#include "piece_eval_tables.hpp"

#define POSITIVE_INFINITY 1000000
#define NEGATIVE_INFINITY -1000000

namespace TungstenChess
{
  class Bot
  {
  private:
    Board &m_board;
    OpeningBook m_openingBook;

    enum EvaluationConstants : int
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

    struct BotSettings
    {
      int maxSearchTime = 1000;                   // In milliseconds
      int maxSearchDepth = 5;                     // for fixed depth search
      int quiesceDepth = -1;                      // for quiescence search, set to -1 to search indefinitely (recommended)
      bool useOpeningBook = DEF_USE_OPENING_BOOK; // only used if board starting position is default
      bool logSearchInfo = true;
      bool logPGNMoves = true;
      bool fixedDepthSearch = !DEF_USE_ITERATIVE_DEEPENING;
    };

    const BotSettings m_botSettings;

    struct SearchInfo
    {
      int positionsEvaluated;
      int depthSearched;
      int evaluation;
      bool mateFound;
      int mateIn;
      bool lossFound;

      std::string evalString(PieceColor sideToMove) const
      {
        if (lossFound)
          return "Loss in " + std::to_string(mateIn);

        if (mateFound)
          return "Mate" + (mateIn > 0 ? " in " + std::to_string(mateIn) : "");

        return std::to_string(evaluation * (sideToMove == WHITE ? 1 : -1));
      }
    };

    SearchInfo m_previousSearchInfo = {0, 0};

    std::atomic<bool> m_searchCancelled = false;
    std::atomic<int> m_maxSearchTime = 0;
    std::thread m_searchTimerThread;
    std::condition_variable m_searchTimerEvent;
    std::atomic<bool> m_searchTimerReset = false;
    std::mutex m_searchTimerMutex;
    std::atomic<bool> m_searchTimerTerminated = false;

    once<false> m_openingBookLoaded;

  public:
    Bot(Board &board, const BotSettings &settings) : m_board(board), m_botSettings(settings), m_openingBook(board.isDefaultStartPosition())
    {
      if (m_botSettings.maxSearchTime > 0)
      {
        m_searchTimerThread = std::thread(
            [this]()
            {
              std::unique_lock<std::mutex> lock(m_searchTimerMutex);
              m_searchTimerEvent.wait(lock);
              while (!m_searchTimerTerminated)
              {
                m_searchTimerReset = false;

                if (m_searchTimerEvent.wait_for(lock, std::chrono::milliseconds(m_maxSearchTime), [this]
                                                { return m_searchTimerReset.load(); }))
                  continue;

                m_searchCancelled = true;
              }
            });

        m_searchTimerThread.detach();
      }
    }

    Bot(Board &board) : Bot(board, BotSettings()) {}

    ~Bot()
    {
      m_searchCancelled = true;
      m_searchTimerTerminated = true;
      m_searchTimerReset = true;
      if (m_searchTimerThread.joinable())
        m_searchTimerThread.join();
    }

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::string path, uint openingBookSize)
    {
      if (!m_openingBookLoaded)
        m_openingBook.loadOpeningBook(path, openingBookSize);
    }

    /**
     * @brief Generates the best move for the bot
     */
    Move generateBotMove();

  private:
    /**
     * @brief Gets the legal moves for a color, sorted by heuristic evaluation
     * @param color The color to get the moves for
     * @param onlyCaptures Whether to only get captures
     */
    std::vector<Move> getSortedLegalMoves(PieceColor color, bool onlyCaptures = false)
    {
      std::vector<Move> moves = m_board.getLegalMoves(color, onlyCaptures);
      heuristicSortMoves(moves);
      return moves;
    }

    /**
     * @brief Gets the positional evaluation of a single piece
     * @param pieceIndex The index of the piece
     * @param absolute Whether to return the value of the evaluation as if the piece was white (true for heuristic evaluation, false for static evaluation)
     */
    int getPiecePositionalEvaluation(Square pieceIndex, bool absolute = false) const
    {
      int positionalEvaluation = 0;

      positionalEvaluation = PIECE_EVAL_TABLES[m_board[pieceIndex]][pieceIndex];

      if (!absolute && (m_board[pieceIndex] & BLACK))
        positionalEvaluation = -positionalEvaluation;

      return positionalEvaluation;
    }

    /**
     * @brief Generates the best move for the bot
     * @param depth The depth to search to
     */
    Move generateBestMove(int depth);

    /**
     * @brief Uses iterative deepening to find the best move in a constant amount of time
     * @param time The time in milliseconds to search for
     */
    Move iterativeDeepening(int time);

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