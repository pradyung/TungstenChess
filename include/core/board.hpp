#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "core/bitboard.hpp"
#include "core/move.hpp"
#include "core/zobrist.hpp"

#define NO_EP 8
#define MAX_MOVE_COUNT 218

#define DEFAULT_START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#ifndef START_FEN
#define START_FEN DEFAULT_START_FEN
#endif

namespace TungstenChess
{
  typedef std::array<Move, MAX_MOVE_COUNT> MoveArray;

  enum CastlingRights : uint8_t
  {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8,
    KINGSIDE = 16,
    QUEENSIDE = 32,
    BOTHSIDES = KINGSIDE | QUEENSIDE,
    WHITE_CASTLING = WHITE_KINGSIDE | WHITE_QUEENSIDE,
    BLACK_CASTLING = BLACK_KINGSIDE | BLACK_QUEENSIDE,
  };

  class Board
  {
  private:
    std::array<Piece, 64> m_board;
    std::array<Bitboard, ALL_PIECES + 1> m_bitboards;
    std::array<Piece, PIECE_NUMBER> m_kingIndices; // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage
    std::array<uint, PIECE_NUMBER> m_pieceCounts;

    PieceColor m_sideToMove;

    uint8_t m_castlingRights;
    uint8_t m_enPassantFile = NO_EP;
    uint8_t m_hasCastled;
    uint8_t m_halfmoveClock;

    ZobristKey m_zobristKey;

    std::vector<ZobristKey> m_positionHistory;

    const bool m_wasDefaultStartPosition; // Whether the board started from the default starting position
    std::vector<Move> m_moveHistory;

  public:
    Board(std::string fen = START_FEN);

    Piece operator[](Square index) const { return m_board[index]; }
    PieceColor sideToMove() const { return m_sideToMove; }
    uint8_t castlingRights() const { return m_castlingRights; }
    uint8_t enPassantFile() const { return m_enPassantFile; }
    uint8_t hasCastled() const { return m_hasCastled; }
    uint8_t halfmoveClock() const { return m_halfmoveClock; }
    const Bitboard &bitboard(Piece piece) const { return m_bitboards[piece]; }
    ZobristKey zobristKey() const { return m_zobristKey; }
    const std::vector<Move> &moveHistory() const { return m_moveHistory; }
    bool wasDefaultStartPosition() const { return m_wasDefaultStartPosition; }
    Square kingIndex(Piece piece) const { return m_kingIndices[piece]; }
    uint pieceCount(Piece piece) const { return m_pieceCounts[piece]; }

    /**
     * @brief Resets the board to the provided fen
     * @param fen The fen to reset the board to
     */
    void resetBoard(std::string fen = START_FEN);

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(PieceColor color) const;

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(Square pieceIndex);

    /**
     * @brief Returns the bitboard of the squares a piece can move to, not excluding moves that leave the king in check
     * @param pieceIndex The index of the piece
     */
    Bitboard getPseudoLegalPieceMovesBitboard(Square pieceIndex) const;

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
     * @brief Makes a move on the board
     * @param move The move to make (UCI string)
     */
    UnmoveData makeMove(std::string move);

    /**
     * @brief Undoes a move, handling all board state changes
     * @param move The move to undo
     */
    void unmakeMove(Move move, UnmoveData unmoveData);

    enum GameStatus : uint8_t
    {
      NO_MATE = 0,
      STALEMATE = 1,
      LOSE = 2,
    };
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
     * @param legalMoves The array to store the moves in (entry after last generated move will be NULL_MOVE)
     * @param color The color to get the moves for
     * @param onlyCaptures Whether to only include capture moves
     * @return The number of legal moves
     */
    int getLegalMoves(MoveArray &legalMoves, PieceColor color, bool onlyCaptures = false);

    /**
     * @brief Counts the number of times a position has been repeated
     * @param key The Zobrist key of the position to check
     */
    uint8_t countRepetitions(ZobristKey key) const;

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
    void updateBitboards(Square pieceIndex, Piece oldPiece, Piece newPiece);

    /**
     * @brief Quickly makes a move, only updating bitboards and king indices (used for illegal move detection), does not update board array, Zobrist key or piece counts
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @return MoveFlags returns flag only if move was en passant, promotion, kingside castle, or queenside castle
     */
    MoveFlags quickMakeMove(Square from, Square to);

    /**
     * @brief Quickly unmakes a move, only updating bitboards and king indices (used for illegal move detection), does not update board array or Zobrist key
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @param flag The flag returned by quickMakeMove
     */
    void quickUnmakeMove(Square from, Square to, MoveFlags flag);

    /**
     * @brief Updates the piece at a given index and handles bitboard and Zobrist key updates
     * @param pieceIndex The index of the piece to update
     * @param newPiece The new piece
     */
    void updatePiece(Square pieceIndex, Piece newPiece);

    /**
     * @brief Moves a piece from one square to another and handles bitboard and Zobrist key updates
     * @param from The index of the piece to move
     * @param to The index to move the piece to
     * @param promotionPiece The piece to promote to (if any)
     */
    void movePiece(Square from, Square to, Piece promotionPiece = NO_PIECE);

    /**
     * @brief Unmoves a piece from one square to another and handles bitboard and Zobrist key updates
     *        Similar to movePiece, but reversed (Moves piece from "to" to "from", unlike movePiece)
     * @param from The index of the piece that was moved
     * @param to The index of the piece now
     * @param movedPiece The piece that was moved (Used for undoing promotions)
     * @param capturedPiece The piece that was captured (Used for undoing captures)
     */
    void unmovePiece(Square from, Square to, Piece movedPiece = NO_PIECE, Piece capturedPiece = NO_PIECE);

    /**
     * @brief Removes castling rights
     * @param rights The rights to remove, see enum CastlingRights
     */
    void removeCastlingRights(uint8_t rights);

    /**
     * @brief Removes castling rights
     * @param color The color to remove the rights from
     * @param side The side to remove the rights from, see enum CastlingRights (KINGSIDE/QUEENSIDE/CASTLING)
     */
    void removeCastlingRights(PieceColor color, CastlingRights side);

    /**
     * @brief Updates the en passant file and handles Zobrist key updates
     * @param file The new en passant file (0-7), or NO_EP (8) if there is no en passant
     */
    void updateEnPassantFile(File file);

    /**
     * @brief Updates the castling rights and handles Zobrist key updates
     * @param rights The new castling rights, see enum CastlingRights
     */
    void updateCastlingRights(uint8_t rights);

    /**
     * @brief Switches the side to move and handles Zobrist key updates
     */
    void switchSideToMove();

    /**
     * @brief Gets a bitboard of pseudo-legal moves for a piece (does not check for pins or checks)
     * @param pieceIndex The index of the piece
     * @param includeCastling Whether to include castling moves (should be false when checking for attacks on the king)
     */
    Bitboard getPseudoLegalPieceMoves(Square pieceIndex, PieceColor color, bool includeCastling = true) const;

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