#pragma once

#include <thread>

#include "bot/opening_book.hpp"
#include "bot/transposition_table.hpp"
#include "core/board.hpp"
#include "utils/utils.hpp"

#define INF_EVAL 1000000

#define AUXILIARY_MOVE_STACK_SIZE 16384

#define DEF_USE_OPENING_BOOK !DEBUG_MODE

namespace TungstenChess
{
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
    KING_SAFETY_PAWN_SHIELD_PER_PAWN_BONUS = 20,
    CONTEMPT = 100,
  };

  class Bot
  {
  private:
    Board &m_board;
    OpeningBook m_openingBook;

    utils::auxiliary_stack<Move> m_moveStack;

    struct BotSettings
    {
      int maxSearchTime = 2000; // in milliseconds
      int quiesceDepth = -1;    // set to -1 to quiesce indefinitely
      bool useOpeningBook = DEF_USE_OPENING_BOOK;
      bool logSearchInfo = true;
      bool logPGNMoves = true;
      int transpositionTableSizeMB = 128;
    };

    const BotSettings m_botSettings;

    TranspositionTable m_transpositionTable;

    static constexpr inline int CASTLING_BONUS_MULTIPLIERS[16] =
        {0, 1, 1, 2, 0, -1, 1, 0, 0, 1, -1, 0, 0, -1, -1, -2};

    static const int MATERIAL_DIMINISH_SHIFT = 14;

    struct SearchInfo
    {
      int positionsEvaluated = 0;
      int transpositionsUsed = 0;
      int depthSearched = 0;

      int nextDepthNumMovesSearched = 0;
      int nextDepthTotalMoves = 0;

      int evaluation = 0;
      bool mateFound = false;
      int mateIn = 0;
      bool lossFound = false;
    };

    SearchInfo m_previousSearchInfo;
    uint m_currentSearchId = 0;

    std::atomic<bool> m_searchCancelled = false;
    std::atomic<int> m_maxSearchTime = 0;
    std::thread m_searchTimerThread;
    std::condition_variable m_searchTimerEvent;
    std::atomic<bool> m_searchTimerReset = false;
    std::mutex m_searchTimerMutex;
    std::atomic<bool> m_searchTimerTerminated = false;

    utils::once<false> m_onceOpeningBookLoaded;

  public:
    Bot(Board &board, const BotSettings &settings);
    Bot(Board &board) : Bot(board, BotSettings()) {}
    Bot(Board &board, int maxSearchTime) : Bot(board, BotSettings{maxSearchTime}) {}

    ~Bot();

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     */
    void loadOpeningBook(const std::filesystem::path path);

    /**
     * @brief Generates the best move for the bot
     * @param maxSearchTime The maximum time to search for in milliseconds
     */
    Move generateBotMove(int maxSearchTime = -1);

  private:
    /**
     * @brief Starts the search timer thread
     * @note This function is called automatically by the constructor
     */
    void startSearchTimerThread();

    /**
     * @brief Stops the search timer thread
     * @note This function is called automatically by the destructor
     */
    void stopSearchTimerThread();

    /**
     * @brief Gets the legal moves for a color, sorted by heuristic evaluation
     * @param moves The array to store the moves in
     * @param onlyCaptures Whether to only get captures
     * @param bestMove The best move found so far, used when iterative deepening has already found a good move
     * @return The number of legal moves generated
     */
    int getSortedLegalMoves(MoveAllocation &moves, bool onlyCaptures = false, Move bestMove = NULL_MOVE);

    /**
     * @brief Gets the positional evaluation of a single piece
     * @param pieceIndex The index of the piece
     * @param absolute Whether to return the value of the evaluation as if the piece was white (true for heuristic evaluation, false for static evaluation)
     */
    int getPiecePositionalEvaluation(Square pieceIndex, bool absolute = false) const;

    /**
     * @brief Generates the best move for the bot
     * @param depth The depth to search to
     * @param bestMoveSoFar The best move found so far, used when iterative deepening has already found a good move
     */
    Move generateBestMove(int depth, Move bestMoveSoFar = NULL_MOVE);

    /**
     * @brief Uses iterative deepening to find the best move in a constant amount of time
     * @param time The time in milliseconds to search for
     */
    Move iterativeDeepeningSearch(int time);

    /**
     * @brief Gets the static evaluation of the current position, from the perspective of the side to move (positive if favorable, negative if unfavorable)
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
     * @brief Gets the mobility evaluation of the current position, independent of the side to move (positive for white favor, negative for black favor)
     */
    int getMobilityEvaluation() const;

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
     * @return The evaluation of the current position, from the perspective of the side to move (positive if favorable, negative if unfavorable)
     */
    int negamax(int depth, int alpha = -INF_EVAL, int beta = INF_EVAL, bool quiesce = false);

    /**
     * @brief Heuristic evaluation of a move, used for move ordering to improve alpha-beta pruning
     * @param move The move to evaluate
     * @param bestMove The best move found so far, used when iterative deepening has already found a good move
     */
    int heuristicEvaluation(Move move, Move bestMove = NULL_MOVE);

    /**
     * @brief Sorts moves by heuristic evaluation (in place) to improve alpha-beta pruning
     * @param moves The moves to sort
     * @param movesCount The number of moves in the array
     * @param bestMove The best move found so far, used when iterative deepening has already found a good move
     */
    void heuristicSortMoves(MoveAllocation &moves, int movesCount, Move bestMove = NULL_MOVE);
  };
}