#pragma once

#include <string>
#include <vector>
#include <array>

#include "bitboard.hpp"
#include "zobrist.hpp"
#include "magic.hpp"
#include "types.hpp"

#define NUM_FEN_PARTS 6
#define NO_EP 8

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

namespace TungstenChess
{
  typedef uint8_t Piece;
  typedef uint16_t MoveInt;

  enum CastlingRights
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

  enum GameStatus
  {
    NO_MATE = 0,
    STALEMATE = 1,
    LOSE = 2,
  };

  enum MoveFlags
  {
    NORMAL = 0,
    CAPTURE = 1,
    PAWN_DOUBLE = 2,
    EP_CAPTURE = 4,
    PROMOTION = 8,
    KSIDE_CASTLE = 16,
    QSIDE_CASTLE = 32,
    CASTLE = KSIDE_CASTLE | QSIDE_CASTLE
  };

  enum FenParts
  {
    BOARD = 0,
    SIDE_TO_MOVE = 1,
    CASTLING_RIGHTS = 2,
    EN_PASSANT = 3,
    HALFMOVE_CLOCK = 4,
    FULLMOVE_NUMBER = 5
  };

  struct Move
  {
    int from;
    int to;
    Piece piece;
    Piece capturedPiece;
    Piece promotionPieceType;

    int castlingRights;
    int enPassantFile;

    int halfmoveClock;

    int flags;

    Move() = default;

    /**
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param piece The piece that is moving
     * @param capturedPiece The piece that is being captured, if any
     * @param enPassantFile The current state of the en passant file, used to restore it when the move is unmade
     * @param castlingRights The current state of the castling rights, used to restore them when the move is unmade
     * @param promotionPieceType The piece that the moving piece is being promoted to, if any (only piece type)
     */
    Move(int from, int to, Piece piece, Piece capturedPiece, int castlingRights, int enPassantFile, int halfmoveClock, Piece promotionPieceType = EMPTY)
        : from(from), to(to), piece(piece), capturedPiece(capturedPiece), castlingRights(castlingRights), enPassantFile(enPassantFile), promotionPieceType(promotionPieceType), halfmoveClock(halfmoveClock), flags(NORMAL)
    {
      int pieceType = piece & TYPE;

      if (pieceType == KING && from - to == -2)
      {
        this->flags |= KSIDE_CASTLE;
        return;
      }

      if (pieceType == KING && from - to == 2)
      {
        this->flags |= QSIDE_CASTLE;
        return;
      }

      if (pieceType == PAWN && (from - to == 16 || from - to == -16))
      {
        this->flags |= PAWN_DOUBLE;
        return;
      }

      if (pieceType == PAWN && capturedPiece == EMPTY && (to - from) % 8)
      {
        this->flags |= EP_CAPTURE;
        return;
      }

      if (capturedPiece != EMPTY)
      {
        this->flags |= CAPTURE;
      }

      if (pieceType == PAWN && (to <= 7 || to >= 56))
      {
        this->flags |= PROMOTION;
      }
    }

    /**
     * Returns an integer representation of the move
     */
    int toInt() const { return from | (to << 6); }

    /**
     * Returns a UCI string representation of the move
     */
    std::string getUCI() const
    {
      std::string uci = "";

      uci += 'a' + (from % 8);
      uci += '8' - (from / 8);
      uci += 'a' + (to % 8);
      uci += '8' - (to / 8);

      if (promotionPieceType != EMPTY)
        uci += ".pnbrqk"[promotionPieceType];

      return uci;
    }
  };

  struct BoardSaveState
  {
    int sideToMove;
    int castlingRights;
    int enPassantFile;
    int hasCastled;
    int halfmoveClock;
    std::array<Piece, 64> board;
    std::array<Bitboard, PIECE_NUMBER> bitboards;
    std::array<int, PIECE_NUMBER> kingIndices;
    ZobristKey zobristKey;
    std::vector<MoveInt> moveHistory;
    std::vector<ZobristKey> positionHistory;
  };

  class Board
  {
  public:
    Board(std::string fen = START_FEN);

    const bool isDefaultStartPosition; // Whether the board is in the default starting position (used for determining whether opening book can be used)

    int sideToMove;

    int castlingRights;
    int enPassantFile = NO_EP;

    int hasCastled;

    int halfmoveClock;

    std::array<Bitboard, PIECE_NUMBER> bitboards;

    ZobristKey zobristKey;

    std::vector<MoveInt> moveHistory;

    // Only indexes WHITE_KING and BLACK_KING are valid, the rest are garbage
    std::array<int, PIECE_NUMBER> kingIndices;

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(int color)
    {
      return isAttacked(kingIndices[color | KING], color ^ COLOR);
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
     * @brief Undoes a move, handling all board state changes
     * @param move The move to undo
     */
    void unmakeMove(Move move);

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
     * @brief Counts the number of games that can be played from the current position to a given depth
     * @param depth The depth to search to
     */
    int countGames(int depth);

    /**
     * @brief Gets the legal moves for a color
     * @param color The color to get the moves for
     * @param includeCastling Whether to include castling moves (should be false for quiescence search)
     */
    std::vector<Move> getLegalMoves(int color, bool includeCastling = true);

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
     * @brief Gets a restorable save state of the board's current state
     */
    BoardSaveState getSaveState()
    {
      return BoardSaveState{
          sideToMove,
          castlingRights,
          enPassantFile,
          hasCastled,
          halfmoveClock,
          board,
          bitboards,
          kingIndices,
          zobristKey,
          moveHistory,
          positionHistory};
    }

    /**
     * @brief Restores the board to a previous state
     * @param saveState The state to restore
     */
    void restoreSaveState(BoardSaveState saveState)
    {
      sideToMove = saveState.sideToMove;
      castlingRights = saveState.castlingRights;
      enPassantFile = saveState.enPassantFile;
      hasCastled = saveState.hasCastled;
      halfmoveClock = saveState.halfmoveClock;
      zobristKey = saveState.zobristKey;

      board = saveState.board;
      bitboards = saveState.bitboards;
      kingIndices = saveState.kingIndices;
      moveHistory = saveState.moveHistory;
      positionHistory = saveState.positionHistory;
    }

    /**
     * @brief Allows indexing the board like an array
     */
    Piece operator[](int index) { return board[index]; }

  private:
    std::array<Piece, 64> board;

    std::vector<ZobristKey> positionHistory;

    const Zobrist zobrist = Zobrist::getInstance();
    const MovesLookup movesLookup = MovesLookup::getInstance();
    const MagicMoveGen magicMoveGen = MagicMoveGen::getInstance();

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
      zobristKey ^= zobrist.getPieceCombinationKey(pieceIndex, board[pieceIndex], piece);

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

    Bitboard getPawnMoves(int pieceIndex, bool _ = false, bool onlyCaptures = false);
    Bitboard getKnightMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getBishopMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getRookMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getQueenMoves(int pieceIndex, bool _ = false, bool __ = false);
    Bitboard getKingMoves(int pieceIndex, bool includeCastling = true, bool __ = false);

    Bitboard (TungstenChess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(int, bool, bool) = {
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
    bool isAttacked(int square, int color);
  };
}