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

#define NO_EP 8

#define DEFAULT_START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#ifndef START_FEN
#define START_FEN DEFAULT_START_FEN
#endif

namespace TungstenChess
{
  class Board
  {
  private:
    std::array<Piece, 64> m_board;
    std::array<Bitboard, ALL_PIECES + 1> m_bitboards;
    std::array<Piece, PIECE_NUMBER> m_kingIndices; // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage

    PieceColor m_sideToMove;

    uint8_t m_castlingRights;
    uint8_t m_enPassantFile = NO_EP;
    uint8_t m_hasCastled;
    uint8_t m_halfmoveClock;

    ZobristKey m_zobristKey;

    std::vector<ZobristKey> m_positionHistory;

    const bool m_isDefaultStartPosition; // Whether the board is in the default starting position (used for determining whether opening book can be used)
    std::vector<Move> m_moveHistory;

  public:
    Board(std::string fen = START_FEN) : m_isDefaultStartPosition(fen == DEFAULT_START_FEN)
    {
      Zobrist::init();
      MovesLookup::init();
      MagicMoveGen::init();

      resetBoard(fen);
    }

    // Accessor methods
    Piece operator[](Square index) const { return m_board[index]; }
    PieceColor sideToMove() const { return m_sideToMove; }
    uint8_t castlingRights() const { return m_castlingRights; }
    uint8_t enPassantFile() const { return m_enPassantFile; }
    uint8_t hasCastled() const { return m_hasCastled; }
    uint8_t halfmoveClock() const { return m_halfmoveClock; }
    const Bitboard &bitboard(Piece piece) const { return m_bitboards[piece]; }
    ZobristKey zobristKey() const { return m_zobristKey; }
    const std::vector<Move> &moveHistory() const { return m_moveHistory; }
    bool isDefaultStartPosition() const { return m_isDefaultStartPosition; }
    Square kingIndex(Piece piece) const { return m_kingIndices[piece]; }

    /**
     * @brief Gets a key representing the pieces still on the board
     */
    uint64_t getPieceKey() const
    {
      uint64_t key = 0;

      std::array<uint8_t, PIECE_NUMBER> pieceCounts = {0};

      for (Square i = 0; i < 64; i++)
        if (m_board[i])
          pieceCounts[m_board[i]]++;

      for (Piece piece : validPieces)
      {
        if (piece == EMPTY)
          continue;

        key = key * 10 + pieceCounts[piece];
      }

      return key;
    }

    /**
     * @brief Resets the board to the provided fen
     * @param fen The fen to reset the board to
     */
    void resetBoard(std::string fen = START_FEN);

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(PieceColor color) const
    {
      return isAttacked(m_kingIndices[color | KING], color ^ COLOR);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(Square pieceIndex)
    {
      return getLegalPieceMovesBitboard(pieceIndex, m_board[pieceIndex] & COLOR);
    }

    struct UnmoveData
    {
      Piece piece;
      Piece capturedPiece;
      uint8_t castlingRights;
      uint8_t enPassantFile;
      uint8_t halfmoveClock;
      uint8_t flags;
    };

    /**
     * @brief Makes a move on the board
     * @param move The move to make
     */
    UnmoveData makeMove(Move move);

    /**
     * @brief Undoes a move, handling all board state changes
     * @param move The move to undo
     */
    void unmakeMove(Move move, UnmoveData unmoveData);

    /**
     * @brief Returns the game status for the current side - see enum GameStatus
     * @param color The color to check
     * @return GameStatus - Note: returns NO_MATE even if "color" has won, only returns LOSE if "color" has lost
     */
    GameStatus getGameStatus(PieceColor color);

    /**
     * @brief Generates a move from a UCI string
     * @param uci The UCI string
     */
    Move generateMoveFromUCI(std::string uci) const;

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
    uint countGames(uint8_t depth, bool verbose = true);

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
    uint8_t countRepetitions(ZobristKey key) const
    {
      uint8_t count = 0;

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
    ZobristKey calculateInitialZobristKey() const;

    /**
     * @brief Updates bitboards for a single changing piece
     * @param pieceIndex The index of the piece
     * @param oldPiece The old piece
     * @param newPiece The new piece
     */
    void updateBitboards(Square pieceIndex, Piece oldPiece, Piece newPiece)
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
    MoveFlags quickMakeMove(Square from, Square to)
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
          Square epSquare = to + (color & WHITE ? 8 : -8);
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
    void quickUnmakeMove(Square from, Square to, MoveFlags flag)
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
        Square epSquare = to + (color & WHITE ? 8 : -8);
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
    void updatePiece(Square pieceIndex, Piece newPiece)
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
    void movePiece(Square from, Square to, Piece promotionPiece = EMPTY)
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
    void unmovePiece(Square from, Square to, Piece movedPiece = EMPTY, Piece capturedPiece = EMPTY)
    {
      updatePiece(from, movedPiece == EMPTY ? m_board[to] : movedPiece);
      updatePiece(to, capturedPiece);
    }

    /**
     * @brief Removes castling rights
     * @param rights The rights to remove, see enum CastlingRights
     */
    void removeCastlingRights(uint8_t rights)
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
    void removeCastlingRights(PieceColor color, CastlingRights side)
    {
      removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
    }

    /**
     * @brief Updates the en passant file and handles Zobrist key updates
     * @param file The new en passant file (0-7), or NO_EP (8) if there is no en passant
     */
    void updateEnPassantFile(File file)
    {
      m_zobristKey ^= Zobrist::enPassantKeys[m_enPassantFile];
      m_enPassantFile = file;
      m_zobristKey ^= Zobrist::enPassantKeys[file];
    }

    /**
     * @brief Updates the castling rights and handles Zobrist key updates
     * @param rights The new castling rights, see enum CastlingRights
     */
    void updateCastlingRights(uint8_t rights)
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
    Bitboard getPseudoLegalPieceMoves(Square pieceIndex, PieceColor color, bool includeCastling = true) const
    {
      return (this->*getPieceMoves[m_board[pieceIndex] & TYPE])(pieceIndex, color, includeCastling);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     * @param color The color of the piece
     */
    Bitboard getLegalPieceMovesBitboard(Square pieceIndex, PieceColor color, bool onlyCaptures = false);

    /**
     * @brief Returns the bitboard of pieces that can move to a given square. Does not include kings for technical reasons
     * @param targetSquare The square to check
     * @param targetPiece The piece on the target square
     * @param color The color of the pieces moving
     */
    Bitboard getAttackingPiecesBitboard(Square targetSquare, Piece targetPiece, PieceColor color) const;

    Bitboard getPawnMoves(Square pieceIndex, PieceColor color, bool _ = false) const;
    Bitboard getKnightMoves(Square pieceIndex, PieceColor color, bool _ = false) const;
    Bitboard getBishopMoves(Square pieceIndex, PieceColor color, bool _ = false) const;
    Bitboard getRookMoves(Square pieceIndex, PieceColor color, bool _ = false) const;
    Bitboard getQueenMoves(Square pieceIndex, PieceColor color, bool _ = false) const;
    Bitboard getKingMoves(Square pieceIndex, PieceColor color, bool includeCastling = true) const;

    Bitboard (TungstenChess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(Square, PieceColor, bool) const = {
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
    bool isAttacked(Square square, PieceColor color) const;
  };
}