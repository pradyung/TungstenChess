#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>

#include "bitboard.hpp"
#include "zobrist.hpp"
#include "magic.hpp"
#include "types.hpp"
#include "move.hpp"

#define NUM_FEN_PARTS 6
#define NO_EP 8

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

namespace TungstenChess
{
  class Board
  {
  private:
    std::array<Piece, 64> m_board;

    PieceColor m_sideToMove;

    int m_castlingRights;
    int m_enPassantFile = NO_EP;

    int m_hasCastled;

    int m_halfmoveClock;

    std::array<Bitboard, ALL_PIECES + 1> m_bitboards;

    ZobristKey m_zobristKey;

    std::vector<MoveInt> m_moveHistory;

    const bool m_isDefaultStartPosition; // Whether the board is in the default starting position (used for determining whether opening book can be used)

    std::array<int, PIECE_NUMBER> m_kingIndices; // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage

    std::vector<ZobristKey> m_positionHistory;

  public:
    Board(std::string fen = START_FEN) : m_isDefaultStartPosition(fen == START_FEN)
    {
      Zobrist::init();
      MovesLookup::init();
      MagicMoveGen::init();

      resetBoard(fen);
    }

    // Accessor methods
    Piece operator[](int index) { return m_board[index]; }
    PieceColor sideToMove() { return m_sideToMove; }
    int castlingRights() { return m_castlingRights; }
    int enPassantFile() { return m_enPassantFile; }
    int hasCastled() { return m_hasCastled; }
    int halfmoveClock() { return m_halfmoveClock; }
    Bitboard bitboard(Piece piece) { return m_bitboards[piece]; }
    ZobristKey zobristKey() { return m_zobristKey; }
    std::vector<MoveInt> moveHistory() { return m_moveHistory; }
    bool isDefaultStartPosition() { return m_isDefaultStartPosition; }
    int kingIndex(Piece piece) { return m_kingIndices[piece]; }

    /**
     * @brief Resets the board to the provided fen
     * @param fen The fen to reset the board to
     */
    void resetBoard(std::string fen = START_FEN);

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(PieceColor color)
    {
      return isAttacked(m_kingIndices[color | KING], color ^ COLOR);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(int pieceIndex)
    {
      return getLegalPieceMovesBitboard(pieceIndex, m_board[pieceIndex] & COLOR);
    }

    /**
     * @brief Makes a move on the board
     * @param move The move to make
     */
    void makeMove(Move move);

    /**
     * @brief Undoes a move, handling all board state changes
     * @param move The move to undo
     */
    void unmakeMove(Move move);

    /**
     * @brief Returns the game status for the current side - see enum GameStatus
     * @param color The color to check
     * @return int - Note: returns NO_MATE even if "color" has won, only returns LOSE if "color" has lost
     */
    int getGameStatus(PieceColor color);

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
     * @brief Counts the number of games that can be played from the current position to a given depth
     * @param depth The depth to search to
     * @param verbose Whether to print the number of games found after each 1-deep move
     */
    int countGames(int depth, bool verbose = true);

    /**
     * @brief Gets the legal moves for a color
     * @param color The color to get the moves for
     * @param onlyCaptures Whether to only include capture moves
     */
    std::vector<Move> getLegalMoves(PieceColor color, bool onlyCaptures = false);

    /**
     * @brief Counts the number of times a position has been repeated
     * @param key The Zobrist key of the position to check
     */
    int countRepetitions(ZobristKey key)
    {
      int count = 0;

      for (size_t i = 0; i < m_positionHistory.size(); i++)
        if (m_positionHistory[i] == key)
          count++;

      return count;
    }

  private:
    /**
     * @brief Calculates the Zobrist key for the current position. Should only be called once at board initialization
     *        After, the key is updated incrementally with updatePiece, updateCastlingRights, etc
     */
    void calculateInitialZobristKey();

    /**
     * @brief Updates bitboards for a single changing piece
     * @param pieceIndex The index of the piece
     * @param oldPiece The old piece
     * @param newPiece The new piece
     */
    void updateBitboards(int pieceIndex, Piece oldPiece, Piece newPiece)
    {
      Bitboard squareBitboard = 1ULL << pieceIndex;

      if (oldPiece)
      {
        m_bitboards[oldPiece] ^= squareBitboard;
        m_bitboards[oldPiece & COLOR] ^= squareBitboard;
        m_bitboards[ALL_PIECES] ^= squareBitboard;
      }

      if (newPiece)
      {
        m_bitboards[newPiece] |= squareBitboard;
        m_bitboards[newPiece & COLOR] |= squareBitboard;
        m_bitboards[ALL_PIECES] |= squareBitboard;
      }
    }

    /**
     * @brief Quickly makes a move, only updating bitboards and king indices (used for illegal move detection), does not update board array or Zobrist key
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @return MoveFlags returns flag only if move was en passant, promotion, kingside castle, or queenside castle
     */
    MoveFlags quickMakeMove(int from, int to)
    {
      Piece fromPiece = m_board[from];
      Piece toPiece = m_board[to];

      updateBitboards(from, fromPiece, EMPTY);
      updateBitboards(to, toPiece, fromPiece);

      if ((fromPiece & TYPE) == PAWN)
      {
        if (!toPiece && to % 8 != from % 8)
        {
          PieceColor color = fromPiece & COLOR;
          int epSquare = to + (color & WHITE ? 8 : -8);
          updateBitboards(epSquare, (color ^ COLOR) | PAWN, EMPTY);

          return EP_CAPTURE;
        }

        if (to <= 7 || to >= 56)
          return PROMOTION;
      }

      else if ((fromPiece & TYPE) == KING)
      {
        m_kingIndices[fromPiece] = to;

        if (to - from == 2)
        {
          Piece rook = (fromPiece & COLOR) | ROOK;
          updateBitboards(from + 3, rook, EMPTY);
          updateBitboards(from + 1, EMPTY, rook);

          return KSIDE_CASTLE;
        }
        else if (from - to == 2)
        {
          Piece rook = (fromPiece & COLOR) | ROOK;
          updateBitboards(from - 4, rook, EMPTY);
          updateBitboards(from - 1, EMPTY, rook);

          return QSIDE_CASTLE;
        }
      }

      return NORMAL;
    }

    /**
     * @brief Quickly unmakes a move, only updating bitboards and king indices (used for illegal move detection), does not update board array or Zobrist key
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @param flag The flag returned by quickMakeMove
     */
    void quickUnmakeMove(int from, int to, MoveFlags flag)
    {
      Piece fromPiece = m_board[from];
      Piece toPiece = m_board[to];

      updateBitboards(to, fromPiece, toPiece);
      updateBitboards(from, EMPTY, fromPiece);

      if ((fromPiece & TYPE) == KING)
        m_kingIndices[fromPiece] = from;

      if (flag & EP_CAPTURE)
      {
        PieceColor color = fromPiece & COLOR;
        int epSquare = to + (color & WHITE ? 8 : -8);
        updateBitboards(epSquare, EMPTY, (color ^ COLOR) | PAWN);
      }

      else if (flag & KSIDE_CASTLE)
      {
        Piece rook = (fromPiece & COLOR) | ROOK;
        updateBitboards(from + 3, EMPTY, rook);
        updateBitboards(from + 1, rook, EMPTY);
      }

      else if (flag & QSIDE_CASTLE)
      {
        Piece rook = (fromPiece & COLOR) | ROOK;
        updateBitboards(from - 4, EMPTY, rook);
        updateBitboards(from - 1, rook, EMPTY);
      }
    }

    /**
     * @brief Updates the piece at a given index and handles bitboard and Zobrist key updates
     * @param pieceIndex The index of the piece to update
     * @param newPiece The new piece
     */
    void updatePiece(int pieceIndex, Piece newPiece)
    {
      Piece oldPiece = m_board[pieceIndex];

      m_zobristKey ^= Zobrist::getPieceCombinationKey(pieceIndex, oldPiece, newPiece);

      m_kingIndices[newPiece] = pieceIndex;
      m_board[pieceIndex] = newPiece;

      updateBitboards(pieceIndex, oldPiece, newPiece);
    }

    /**
     * @brief Moves a piece from one square to another and handles bitboard and Zobrist key updates
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @param promotionPiece The piece to promote to (if any)
     */
    void movePiece(int from, int to, int promotionPiece = EMPTY)
    {
      updatePiece(to, (promotionPiece & TYPE) == EMPTY ? m_board[from] : promotionPiece);
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
    void unmovePiece(int from, int to, Piece movedPiece = EMPTY, Piece capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? m_board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    /**
     * @brief Removes castling rights
     * @param rights The rights to remove, see enum CastlingRights
     */
    void removeCastlingRights(int rights)
    {
      m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
      m_castlingRights &= ~rights;
      m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
    }

    /**
     * @brief Removes castling rights
     * @param color The color to remove the rights from
     * @param side The side to remove the rights from, see enum CastlingRights (KINGSIDE/QUEENSIDE/CASTLING)
     */
    void removeCastlingRights(PieceColor color, int side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    /**
     * @brief Updates the en passant file and handles Zobrist key updates
     * @param file The new en passant file (0-7), or NO_EP (8) if there is no en passant
     */
    void updateEnPassantFile(int file)
    {
      m_zobristKey ^= Zobrist::enPassantKeys[m_enPassantFile];
      m_enPassantFile = file;
      m_zobristKey ^= Zobrist::enPassantKeys[file];
    }

    /**
     * @brief Updates the castling rights and handles Zobrist key updates
     * @param rights The new castling rights, see enum CastlingRights
     */
    void updateCastlingRights(int rights)
    {
      m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
      m_castlingRights = rights;
      m_zobristKey ^= Zobrist::castlingKeys[rights];
    }

    /**
     * @brief Switches the side to move and handles Zobrist key updates
     */
    void switchSideToMove()
    {
      m_sideToMove ^= COLOR;
      m_zobristKey ^= Zobrist::sideKey;
    }

    /**
     * @brief Gets a bitboard of pseudo-legal moves for a piece (does not check for pins or checks)
     * @param pieceIndex The index of the piece
     * @param includeCastling Whether to include castling moves (should be false when checking for attacks on the king)
     */
    Bitboard getPseudoLegalPieceMoves(int pieceIndex, PieceColor color, bool includeCastling = true)
    {
      return (this->*getPieceMoves[m_board[pieceIndex] & TYPE])(pieceIndex, color, includeCastling);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     * @param color The color of the piece
     */
    Bitboard getLegalPieceMovesBitboard(int pieceIndex, PieceColor color, bool onlyCaptures = false);

    /**
     * @brief Returns the bitboard of pieces that can move to a given square. Does not include kings for technical reasons
     * @param targetSquare The square to check
     * @param targetPiece The piece on the target square
     * @param color The color of the pieces moving
     */
    Bitboard getAttackingPiecesBitboard(int targetSquare, Piece targetPiece, PieceColor color);

    Bitboard getPawnMoves(int pieceIndex, PieceColor color, bool _ = false);
    Bitboard getKnightMoves(int pieceIndex, PieceColor color, bool _ = false);
    Bitboard getBishopMoves(int pieceIndex, PieceColor color, bool _ = false);
    Bitboard getRookMoves(int pieceIndex, PieceColor color, bool _ = false);
    Bitboard getQueenMoves(int pieceIndex, PieceColor color, bool _ = false);
    Bitboard getKingMoves(int pieceIndex, PieceColor color, bool includeCastling = true);

    Bitboard (TungstenChess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(int, PieceColor, bool) = {
        nullptr,
        &TungstenChess::Board::getPawnMoves,
        &TungstenChess::Board::getKnightMoves,
        &TungstenChess::Board::getBishopMoves,
        &TungstenChess::Board::getRookMoves,
        &TungstenChess::Board::getQueenMoves,
        &TungstenChess::Board::getKingMoves};

    /**
     * @brief Checks if a square is attacked by a color
     * @param square The square to check
     * @param color The color to check
     */
    bool isAttacked(int square, PieceColor color);
  };
}