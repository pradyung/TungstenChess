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
  const int SEARCH_DEPTH = 4;
  const int MAX_QUIESCE_DEPTH = 5;

  const int PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  const int EN_PASSANT = 0x70;
  const int CASTLING_RIGHTS = 0x0F;

  class Board
  {
  public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int sideToMove;

    int castlingRights;
    int enPassantFile;

    Bitboard bitboards[PIECE_NUMBER];

    ZobristKey zobristKey;

    int kingIndices[BLACK + 1];

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
    inline Piece operator[](int index) { return board[index]; }

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

    ZobristKey getInitialZobristKey();

    void updatePiece(int pieceIndex, Piece piece);

    inline void movePiece(int from, int to)
    {
      updatePiece(to, board[from]);
      updatePiece(from, EMPTY);
    }

    inline void unmovePiece(int from, int to, int movedPiece = EMPTY, int capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    inline void removeCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights &= ~rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    inline void removeCastlingRights(int color, int side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    inline void updateEnPassantFile(int file)
    {
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
      enPassantFile = file;
      zobristKey ^= zobrist.enPassantKeys[file];
    }

    inline void updateCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights = rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    inline void switchSideToMove()
    {
      sideToMove ^= COLOR;
      zobristKey ^= zobrist.sideKey;
    }

    inline bool pieceCanMove(int pieceIndex, int to)
    {
      return (!board[to]) || (board[to] & COLOR) != (board[pieceIndex] & COLOR);
    }

    inline Bitboard getFriendlyPiecesBitboard(int color)
    {
      return bitboards[color | PAWN] | bitboards[color | KNIGHT] | bitboards[color | BISHOP] | bitboards[color | ROOK] | bitboards[color | QUEEN] | Bitboard(1ULL << kingIndices[color]);
    }

    inline Bitboard getEnemyPiecesBitboard(int color)
    {
      return getFriendlyPiecesBitboard(color ^ COLOR);
    }

    inline Bitboard getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling = true, bool onlyCaptures = false)
    {
      Bitboard movesBitboard = Bitboard();

      movesBitboard = (this->*getPieceMoves[board[pieceIndex] & TYPE])(pieceIndex, includeCastling, onlyCaptures);

      return movesBitboard;
    }

    inline void addPieceToBitboard(int pieceIndex)
    {
      bitboards[board[pieceIndex]].addBit(pieceIndex);
    }

    inline void removePieceFromBitboard(int pieceIndex)
    {
      bitboards[board[pieceIndex]].removeBit(pieceIndex);
    }

    inline int countRepetitions(ZobristKey key)
    {
      int count = 0;

      for (int i = 0; i < positionHistory.size(); i++)
        if (positionHistory[i] == key)
          count++;

      return count;
    }

    inline std::vector<Move> getSortedLegalMoves(int color, bool includeCastling = true)
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

    Move generateBestMove(int depth, int alpha = -1000000, int beta = 1000000);

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