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
  const uint8_t SEARCH_DEPTH = 5;
  const uint8_t QUIESCE_DEPTH = 5;

  const Evaluation PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  class Board
  {
  public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Piece sideToMove;

    uint8_t castlingRights;
    uint8_t enPassantFile;

    Bitboard bitboards[PIECE_NUMBER];

    ZobristKey zobristKey;

    // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage
    Square kingIndices[PIECE_NUMBER];

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(Square pieceIndex, bool includeCastling = true);

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
    bool isInCheck(Piece color);

    /**
     * @brief Returns the game status for the current side - see enum GameStatus
     * @param color The color to check
     * @return uint8_t - Note: returns NO_MATE even if "color" has won, only returns LOSE if "color" has lost
     */
    uint8_t getGameStatus(Piece color);

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
    inline Piece operator[](Square index) { return board[index]; }

  private:
    Piece board[64];

    uint8_t hasCastled;

    std::vector<ZobristKey> positionHistory;

    Openings openings;
    bool inOpeningBook = true;

    MovesLookup movesLookup;

    PieceEvalTables pieceEvalTables;

    MagicMoveGen magicMoveGen;

    Zobrist zobrist;

    ZobristKey getInitialZobristKey() const;

    inline void updatePiece(Square pieceIndex, Piece piece)
    {
      zobristKey ^= zobrist.pieceKeys[pieceIndex][board[pieceIndex]];
      zobristKey ^= zobrist.pieceKeys[pieceIndex][piece];

      kingIndices[piece] = pieceIndex;

      removePieceFromBitboard(pieceIndex);

      board[pieceIndex] = piece;

      addPieceToBitboard(pieceIndex);
    }

    inline void movePiece(Square from, Square to)
    {
      updatePiece(to, board[from]);
      updatePiece(from, EMPTY);
    }

    inline void unmovePiece(Square from, Square to, Piece movedPiece = EMPTY, Piece capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    inline void removeCastlingRights(uint8_t rights)
    {
      zobristKey ^= zobrist.castlingKeys[castlingRights];
      castlingRights &= ~rights;
      zobristKey ^= zobrist.castlingKeys[castlingRights];
    }

    inline void removeCastlingRights(Piece color, uint8_t side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    inline void updateEnPassantFile(Square file)
    {
      zobristKey ^= zobrist.enPassantKeys[enPassantFile];
      enPassantFile = file;
      zobristKey ^= zobrist.enPassantKeys[file];
    }

    inline void updateCastlingRights(uint8_t rights)
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

    inline bool pieceCanMove(Square pieceIndex, Square to) const
    {
      return (!board[to]) || (board[to] & COLOR) != (board[pieceIndex] & COLOR);
    }

    inline Bitboard getFriendlyPiecesBitboard(Piece color) const
    {
      return bitboards[color | PAWN] | bitboards[color | KNIGHT] | bitboards[color | BISHOP] | bitboards[color | ROOK] | bitboards[color | QUEEN] | Bitboard(1ULL << kingIndices[color | KING]);
    }

    inline Bitboard getEnemyPiecesBitboard(Piece color) const
    {
      return getFriendlyPiecesBitboard(color ^ COLOR);
    }

    inline Bitboard getPseudoLegalPieceMoves(Square pieceIndex, bool includeCastling = true, bool onlyCaptures = false)
    {
      Bitboard movesBitboard = Bitboard();

      movesBitboard = (this->*getPieceMoves[board[pieceIndex] & TYPE])(pieceIndex, includeCastling, onlyCaptures);

      return movesBitboard;
    }

    inline void addPieceToBitboard(Square pieceIndex)
    {
      bitboards[board[pieceIndex]].addBit(pieceIndex);
    }

    inline void removePieceFromBitboard(Square pieceIndex)
    {
      bitboards[board[pieceIndex]].removeBit(pieceIndex);
    }

    inline uint8_t countRepetitions(ZobristKey key) const
    {
      uint8_t count = 0;

      for (uint16_t i = 0; i < positionHistory.size(); i++)
        if (positionHistory[i] == key)
          count++;

      return count;
    }

    inline std::vector<Move> getSortedLegalMoves(Piece color, bool includeCastling = true)
    {
      return heuristicSortMoves(getLegalMoves(color, includeCastling));
    }

    void unmakeMove(Move move);

    Bitboard getPawnMoves(Square pieceIndex, bool _ = false, bool onlyCaptures = false);
    Bitboard getKnightMoves(Square pieceIndex, bool _ = false, bool __ = false);
    Bitboard getBishopMoves(Square pieceIndex, bool _ = false, bool __ = false);
    Bitboard getRookMoves(Square pieceIndex, bool _ = false, bool __ = false);
    Bitboard getQueenMoves(Square pieceIndex, bool _ = false, bool __ = false);
    Bitboard getKingMoves(Square pieceIndex, bool includeCastling = true, bool __ = false);

    Bitboard (Chess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(Square, bool, bool) = {
        nullptr,
        &Chess::Board::getPawnMoves,
        &Chess::Board::getKnightMoves,
        &Chess::Board::getBishopMoves,
        &Chess::Board::getRookMoves,
        &Chess::Board::getQueenMoves,
        &Chess::Board::getKingMoves};

    bool isAttacked(Square square, Piece color);

    std::vector<Move> getLegalMoves(Piece color, bool includeCastling = true);

    Move generateMoveFromInt(MoveInt moveInt);

    Move generateOneDeepMove();

    Move generateBestMove(uint8_t depth, Evaluation alpha = -1000000, Evaluation beta = 1000000);

    Evaluation getStaticEvaluation();

    Evaluation getMaterialEvaluation();

    Evaluation getPositionalEvaluation();

    Evaluation getEvaluationBonus();

    Evaluation negamax(uint8_t depth, Evaluation alpha, Evaluation beta);

    Evaluation quiesce(uint8_t depth, Evaluation alpha, Evaluation beta);

    Evaluation heuristicEvaluation(Move move);

    std::vector<Move> heuristicSortMoves(std::vector<Move> moves);
  };
}