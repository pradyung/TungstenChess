#pragma once

#include <string>
#include <vector>
#include <stack>
#include <map>

#include "move.hpp"
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "openings.hpp"
#include "magic.hpp"
#include "Data/move_gen_helpers.hpp"
#include "Data/piece_eval_tables.hpp"

namespace Chess
{
  const BotSettings DEFAULT_BOT_SETTINGS = {
      5, // search depth
      6, // quiesce depth
      0  // use opening book
  };

  const int PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  class Board
  {
  public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int sideToMove;

    int castlingRights;
    int enPassantFile;

    Bitboard bitboards[PIECE_NUMBER];

    ZobristKey zobristKey;

    // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage
    int kingIndices[PIECE_NUMBER];

    /**
     * @brief Loads the opening book from a file
     * @param path The path to the opening book file
     */
    void loadOpeningBook(const std::string path)
    {
      openings.loadOpeningBook(path);
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
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(int color);

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
     * @brief Generates the best move for the bot
     */
    Move generateBotMove();

    /**
     * @brief Allows indexing the board like an array
     */
    Piece operator[](int index) { return board[index]; }

  private:
    Piece board[64];

    int hasCastled;

    std::vector<ZobristKey> positionHistory;

    Openings openings;
    bool inOpeningBook = true;

    MovesLookup movesLookup;

    PieceEvalTables pieceEvalTables;

    MagicMoveGen magicMoveGen;

    Zobrist zobrist;

    BotSettings botSettings = DEFAULT_BOT_SETTINGS;

    ZobristKey getInitialZobristKey() const;

    void updatePiece(int pieceIndex, Piece piece)
    {
      zobristKey ^= zobrist.pieceKeys[pieceIndex][board[pieceIndex]];
      zobristKey ^= zobrist.pieceKeys[pieceIndex][piece];

      kingIndices[piece] = pieceIndex;

      removePieceFromBitboard(pieceIndex);

      board[pieceIndex] = piece;

      addPieceToBitboard(pieceIndex);
    }

    void movePiece(int from, int to, int promotionPiece = EMPTY)
    {
      updatePiece(to, (promotionPiece & TYPE) == EMPTY ? board[from] : promotionPiece);
      updatePiece(from, EMPTY);
    }

    void unmovePiece(int from, int to, int movedPiece = EMPTY, int capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    void removeCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights &= ~rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    void removeCastlingRights(int color, int side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    void updateEnPassantFile(int file)
    {
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
      enPassantFile = file;
      zobristKey ^= zobrist.enPassantKeys[file];
    }

    void updateCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights = rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    void switchSideToMove()
    {
      sideToMove ^= COLOR;
      zobristKey ^= zobrist.sideKey;
    }

    Bitboard getFriendlyPiecesBitboard(int color) const
    {
      return bitboards[color | PAWN] | bitboards[color | KNIGHT] | bitboards[color | BISHOP] | bitboards[color | ROOK] | bitboards[color | QUEEN] | Bitboard(1ULL << kingIndices[color | KING]);
    }

    Bitboard getEnemyPiecesBitboard(int color) const
    {
      return getFriendlyPiecesBitboard(color ^ COLOR);
    }

    Bitboard getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling = true, bool onlyCaptures = false)
    {
      return (this->*getPieceMoves[board[pieceIndex] & TYPE])(pieceIndex, includeCastling, onlyCaptures);
    }

    void addPieceToBitboard(int pieceIndex)
    {
      bitboards[board[pieceIndex]].addBit(pieceIndex);
    }

    void removePieceFromBitboard(int pieceIndex)
    {
      bitboards[board[pieceIndex]].removeBit(pieceIndex);
    }

    int countRepetitions(ZobristKey key) const
    {
      int count = 0;

      for (int i = 0; i < positionHistory.size(); i++)
        if (positionHistory[i] == key)
          count++;

      return count;
    }

    std::vector<Move> getSortedLegalMoves(int color, bool includeCastling = true)
    {
      return heuristicSortMoves(getLegalMoves(color, includeCastling));
    }

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

    bool isAttacked(int square, int color);

    std::vector<Move> getLegalMoves(int color, bool includeCastling = true);

    Move generateMoveFromInt(int moveInt);

    Move generateOneDeepMove();

    Move generateBestMove(int depth, int alpha = NEGATIVE_INFINITY, int beta = POSITIVE_INFINITY);

    int getStaticEvaluation();

    int getMaterialEvaluation();

    int getPositionalEvaluation();

    int getEvaluationBonus();

    int negamax(int depth, int alpha, int beta);

    int quiesce(int depth, int alpha, int beta);

    int heuristicEvaluation(Move move);

    std::vector<Move> heuristicSortMoves(std::vector<Move> moves);
  };
}