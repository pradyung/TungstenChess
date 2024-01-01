#pragma once

#include <string>
#include <vector>
#include <stack>
#include <map>

#include "move.hpp"
#include "bitboard.hpp"
#include "zobrist.hpp"
#include "openings.hpp"
#include "Data/move_gen_helpers.hpp"
#include "Data/piece_eval_tables.hpp"

namespace Chess
{
  const int SEARCH_DEPTH = 3;

  const int PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  class Board
  {
  public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int sideToMove;

    int enPassantFile;
    int castlingRights;

    Piece board[64];

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

  private:
    int hasCastled;

    std::map<ZobristKey, int> positionHistory;

    Openings openings;
    bool inOpeningBook = true;

    MovesLookup movesLookup;

    PieceEvalTables pieceEvalTables;

    Zobrist zobrist;

    ZobristKey getInitialZobristKey();

    void updatePiece(int pieceIndex, int piece);

    inline void movePiece(int from, int to)
    {
      updatePiece(to, board[from].piece);
      updatePiece(from, EMPTY);
    }

    inline void unmovePiece(int from, int to, int capturedPiece = EMPTY)
    {
      updatePiece(from, board[to].piece);
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

    inline void updateCastlingRights(int rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights |= rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    inline void updateEnPassantFile(int file)
    {
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
      enPassantFile = file;
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
    }

    inline void switchSideToMove()
    {
      sideToMove ^= 24;
      zobristKey ^= zobrist.sideKey;
    }

    inline bool pieceCanMove(int pieceIndex, int to)
    {
      return board[to].isEmpty() || board[to].getPieceColor() != board[pieceIndex].getPieceColor();
    }

    inline static bool isOnBoard(int x, int y)
    {
      return x >= 0 && x <= 7 && y >= 0 && y <= 7;
    }

    inline Bitboard getFriendlyPiecesBitboard(int color)
    {
      return bitboards[color | PAWN] | bitboards[color | KNIGHT] | bitboards[color | BISHOP] | bitboards[color | ROOK] | bitboards[color | QUEEN] | Bitboard(1ULL << kingIndices[color]);
    }

    inline Bitboard getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling = true, bool onlyCaptures = false)
    {
      Bitboard movesBitboard = Bitboard();

      movesBitboard = (this->*getPieceMoves[board[pieceIndex].getPieceType()])(pieceIndex, includeCastling, onlyCaptures);

      return movesBitboard;
    }

    inline void addPieceToBitboard(int pieceIndex)
    {
      if (!(board[pieceIndex].getPieceType() == KING || board[pieceIndex].isEmpty()))
        bitboards[board[pieceIndex].piece].addBit(pieceIndex);
    }

    inline void removePieceFromBitboard(int pieceIndex)
    {
      if (!(board[pieceIndex].getPieceType() == KING || board[pieceIndex].isEmpty()))
        bitboards[board[pieceIndex].piece].removeBit(pieceIndex);
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

    Bitboard getAttackedSquaresBitboard(int color);

    Move generateMoveFromInt(int moveInt);

    Move generateOneDeepMove();

    Move generateBestMove(int depth, int alpha = -1000000, int beta = 1000000);

    int getStaticEvaluation();

    int getMaterialEvaluation();

    int getPositionalEvaluation();

    int getEvaluationBonus();

    int negamax(int depth, int alpha, int beta);

    int quiesce(int alpha, int beta);

    int heuristicEvaluation(Move move);

    std::vector<Move> heuristicSortMoves(std::vector<Move> moves);
  };
}