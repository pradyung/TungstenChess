#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "move.hpp"
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "openings.hpp"
#include "magic.hpp"
#include "move_gen_helpers.hpp"
#include "piece_eval_tables.hpp"

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

  const int PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  class Board
  {
  public:
    Board(std::string fen = START_FEN, BotSettings settings = DEFAULT_BOT_SETTINGS);

    int sideToMove;

    int castlingRights;
    int enPassantFile;

    int positionsEvaluated;
    int depthSearched;

    Bitboard bitboards[PIECE_NUMBER];

    ZobristKey zobristKey;

    // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage
    int kingIndices[PIECE_NUMBER];

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(int color)
    {
      return isAttacked(kingIndices[color | KING], color ^ COLOR);
    }

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     * @param openingBookSize The number of entries in the opening book (i.e. the size of the file in bytes divided by 4)
     */
    void loadOpeningBook(const std::string path, uint openingBookSize)
    {
      openings.loadOpeningBook(path, openingBookSize);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(int pieceIndex, bool includeCastling = true);

    /**
     * @brief Makes a move on the board
     * @param move The move to make
     * @param speculative Whether the move is speculative (used for move tree search and check detection) - MUST BE SET TO FALSE FOR ACTUAL MOVES
     */
    void makeMove(Move move, bool speculative = false);

    /**
     * @brief Returns the game status for the current side - see enum GameStatus
     * @param color The color to check
     * @return int - Note: returns NO_MATE even if "color" has won, only returns LOSE if "color" has lost
     */
    int getGameStatus(int color);

    /**
     * @brief Generates a move from a UCI string
     * @param uci The UCI string
     */
    Move generateMoveFromUCI(std::string uci);

    /**
     * @brief Generates a PGN string from a move
     * @param move The move to convert
     */
    std::string getMovePGN(Move move);

    /**
     * @brief Generates the best move for the bot
     */
    Move generateBotMove();

    /**
     * @brief Counts the number of games that can be played from the current position to a given depth
     * @param depth The depth to search to
     */
    int countGames(int depth);

    /**
     * @brief Allows indexing the board like an array
     */
    Piece operator[](int index) { return board[index]; }

  private:
    std::array<Piece, 64> board;

    int hasCastled;

    std::vector<int> positionHistory;

    Openings openings;
    const Zobrist zobrist = Zobrist::getInstance();
    const MovesLookup movesLookup = MovesLookup::getInstance();
    const MagicMoveGen magicMoveGen = MagicMoveGen::getInstance();

    BotSettings botSettings;

    /**
     * @brief Calculates the Zobrist key for the current position. Should only be called once at board initialization
     *        After, the key is updated incrementally with updatePiece, updateCastlingRights, etc
     */
    void calculateInitialZobristKey();

    /**
     * @brief Updates the piece at a given index and handles bitboard and Zobrist key updates
     * @param pieceIndex The index of the piece to update
     * @param piece The new piece
     */
    void updatePiece(int pieceIndex, Piece piece)
    {
      zobristKey ^= zobrist.precomputedPieceCombinationKeys[pieceIndex][board[pieceIndex]][piece];

      kingIndices[piece] = pieceIndex;

      Bitboards::removeBit(bitboards[board[pieceIndex]], pieceIndex);

      board[pieceIndex] = piece;

      Bitboards::addBit(bitboards[piece], pieceIndex);
    }

    /**
     * @brief Moves a piece from one square to another and handles bitboard and Zobrist key updates
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @param promotionPiece The piece to promote to (if any)
     */
    void movePiece(int from, int to, int promotionPiece = EMPTY)
    {
      updatePiece(to, (promotionPiece & TYPE) == EMPTY ? board[from] : promotionPiece);
      updatePiece(from, EMPTY);
    }

    /**
     * @brief Unmoves a piece from one square to another and handles bitboard and Zobrist key updates
     *        Similar to movePiece, but reversed (Moves piece from "to" to "from", unlike movePiece)
     * @param from The index of the piece that was moved
     * @param to The index of the piece now
     * @param movedPiece The piece that was moved (Used for undoing promotions)
     * @param capturedPiece The piece that was captured (Used for undoing captures)
     */
    void unmovePiece(int from, int to, int movedPiece = EMPTY, int capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    /**
     * @brief Removes castling rights
     * @param rights The rights to remove, see enum CastlingRights
     */
    void removeCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights &= ~rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    /**
     * @brief Removes castling rights
     * @param color The color to remove the rights from
     * @param side The side to remove the rights from, see enum CastlingRights (KINGSIDE/QUEENSIDE/CASTLING)
     */
    void removeCastlingRights(int color, int side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    /**
     * @brief Updates the en passant file and handles Zobrist key updates
     * @param file The new en passant file (0-7), or NO_EP (8) if there is no en passant
     */
    void updateEnPassantFile(int file)
    {
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
      enPassantFile = file;
      zobristKey ^= zobrist.enPassantKeys[file];
    }

    /**
     * @brief Updates the castling rights and handles Zobrist key updates
     * @param rights The new castling rights, see enum CastlingRights
     */
    void updateCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights = rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    /**
     * @brief Switches the side to move and handles Zobrist key updates
     */
    void switchSideToMove()
    {
      sideToMove ^= COLOR;
      zobristKey ^= zobrist.sideKey;
    }

    /**
     * @brief Gets the bitboard of all friendly pieces from the perspective of a color
     * @param color The color to get the bitboard for
     */
    Bitboard getFriendlyPiecesBitboard(int color) const
    {
      return (
          bitboards[color | PAWN] |
          bitboards[color | KNIGHT] |
          bitboards[color | BISHOP] |
          bitboards[color | ROOK] |
          bitboards[color | QUEEN] |
          (1ULL << kingIndices[color | KING]));
    }

    /**
     * @brief Gets the bitboard of all enemy pieces from the perspective of a color
     * @param color The color to get the bitboard for (WHITE returns bitboard of BLACK pieces and vice versa)
     */
    Bitboard getEnemyPiecesBitboard(int color) const
    {
      return getFriendlyPiecesBitboard(color ^ COLOR);
    }

    /**
     * @brief Gets a bitboard of pseudo-legal moves for a piece (does not check for pins or checks)
     * @param pieceIndex The index of the piece
     * @param includeCastling Whether to include castling moves (should be false when checking for attacks on the king)
     * @param onlyCaptures Whether to only include capture moves (should be true when checking for attacks on the king)
     */
    Bitboard getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling = true, bool onlyCaptures = false)
    {
      return (this->*getPieceMoves[board[pieceIndex] & TYPE])(pieceIndex, includeCastling, onlyCaptures);
    }

    /**
     * @brief Counts the number of times a position has been repeated
     * @param key The Zobrist key of the position to check
     */
    int countRepetitions(ZobristKey key)
    {
      int count = 0;

      for (int i = 0; i < positionHistory.size(); i++)
        if (positionHistory[i] == key)
          count++;

      return count;
    }

    /**
     * @brief Gets the legal moves for a color, sorted by heuristic evaluation
     * @param color The color to get the moves for
     * @param includeCastling Whether to include castling moves (should be false for quiescence search)
     */
    std::vector<Move> getSortedLegalMoves(int color, bool includeCastling = true)
    {
      return heuristicSortMoves(getLegalMoves(color, includeCastling));
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
     * @brief Undoes a move, handling all board state changes
     * @param move The move to undo
     */
    void unmakeMove(Move move);

    Bitboard getPawnMoves(int pieceIndex, bool _ = false, bool onlyCaptures = false);
    Bitboard getKnightMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getBishopMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getRookMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getQueenMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getKingMoves(int pieceIndex, bool includeCastling = true, bool __ = false);

    Bitboard (Chess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(int, bool, bool) = {
        nullptr,
        &Chess::Board::getPawnMoves,
        &Chess::Board::getKnightMoves,
        &Chess::Board::getBishopMoves,
        &Chess::Board::getRookMoves,
        &Chess::Board::getQueenMoves,
        &Chess::Board::getKingMoves};

    /**
     * @brief Checks if a square is attacked by a color
     * @param square The square to check
     * @param color The color to check
     */
    bool isAttacked(int square, int color);

    /**
     * @brief Gets the legal moves for a color
     * @param color The color to get the moves for
     * @param includeCastling Whether to include castling moves (should be false for quiescence search)
     */
    std::vector<Move> getLegalMoves(int color, bool includeCastling = true);

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