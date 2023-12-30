#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

#include "move.hpp"
#include "bitboard.hpp"
#include "move_gen_helpers.hpp"
#include "piece_eval_tables.hpp"
#include "openings.hpp"
#include "zobrist.hpp"
#include "transposition_table.hpp"

namespace Chess
{
  class Board
  {
  public:
    Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int sideToMove;

    int enPassantFile;
    int castlingRights;

    Piece board[64];

    Bitboard *bitboards[PIECE_NUMBER];

    int whiteKingIndex;
    int blackKingIndex;

    enum GameStatus
    {
      NO_MATE,
      STALEMATE,
      LOSE
    };

    Bitboard getLegalPieceMovesBitboard(int pieceIndex);

    void makeMove(Move move, bool speculative = true);

    bool isInCheck(int color);

    int getGameStatus(int color);

    Move generateBotMove();

  private:
    int hasCastled;

    std::map<ZobristKey, int> positionHistory;

    Openings openings;
    bool inOpeningBook = true;

    Zobrist zobrist;
    ZobristKey zobristKey;

    TranspositionTable transpositionTable;

    enum CastlingRights
    {
      WHITE_KINGSIDE = 1,
      WHITE_QUEENSIDE = 2,
      BLACK_KINGSIDE = 4,
      BLACK_QUEENSIDE = 8,
      KINGSIDE = 16,
      QUEENSIDE = 32,
      CASTLING = KINGSIDE | QUEENSIDE,
      WHITE_CASTLING = WHITE_KINGSIDE | WHITE_QUEENSIDE,
      BLACK_CASTLING = BLACK_KINGSIDE | BLACK_QUEENSIDE,
    };

    enum EvaluationBonus
    {
      BISHOP_PAIR_BONUS = 100,
      CASTLED_KING_BONUS = 50,
      CAN_CASTLE_BONUS = 25,
      ROOK_ON_OPEN_FILE_BONUS = 50,
      ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
      KNIGHT_OUTPOST_BONUS = 50,
      PASSED_PAWN_BONUS = 50,
      DOUBLED_PAWN_PENALTY = 50,
      ISOLATED_PAWN_PENALTY = 25,
      BACKWARDS_PAWN_PENALTY = 50,
      KING_SAFETY_PAWN_SHIELD_BONUS = 50,
    };

    ZobristKey getInitialZobristKey();

    void updatePiece(int pieceIndex, int piece);
    void moveOnePiece(int from, int to);

    void removeCastlingRights(int rights);
    void removeCastlingRights(int color, int side);
    void updateCastlingRights(int rights);

    void updateEnPassantFile(int file);
    void switchSideToMove();

    void unmakeMove(Move move);

    bool pieceCanMove(int pieceIndex, int to);

    static bool isOnBoard(int x, int y);

    Bitboard getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling = true, bool onlyCaptures = false);

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

    void addPieceToBitboard(int pieceIndex);
    void removePieceFromBitboard(int pieceIndex);

    bool isAttacked(int square, int color);

    std::vector<Move> getLegalMoves(int color);

    Bitboard getAttackedSquaresBitboard(int color);

    int countGames(int depth, bool verbose = true);

    Move generateMoveFromInt(int moveInt);

    Move generateRandomMove();

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

    std::string getStringRepresentation();
  };
}