#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>

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
    FEN_BOARD = 0,
    FEN_SIDE_TO_MOVE = 1,
    FEN_CASTLING_RIGHTS = 2,
    FEN_EN_PASSANT = 3,
    FEN_HALFMOVE_CLOCK = 4,
    FEN_FULLMOVE_NUMBER = 5
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
        : from(from), to(to), piece(piece), capturedPiece(capturedPiece), castlingRights(castlingRights), enPassantFile(enPassantFile), halfmoveClock(halfmoveClock), promotionPieceType(promotionPieceType), flags(NORMAL)
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
    std::array<Bitboard, ALL_PIECES + 1> bitboards;
    std::array<int, PIECE_NUMBER> kingIndices;
    ZobristKey zobristKey;
    std::vector<MoveInt> moveHistory;
    std::vector<ZobristKey> positionHistory;
  };

  class Board
  {
  private:
    std::array<Piece, 64> m_board;

    int m_sideToMove;

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

    const Zobrist &zobrist = Zobrist::getInstance();
    const MovesLookup &movesLookup = MovesLookup::getInstance();
    const MagicMoveGen &magicMoveGen = MagicMoveGen::getInstance();

  public:
    Board(std::string fen = START_FEN) : m_isDefaultStartPosition(fen == START_FEN)
    {
      resetBoard(fen);
    }

    // Accessor methods
    Piece operator[](int index) { return m_board[index]; }
    int sideToMove() { return m_sideToMove; }
    int castlingRights() { return m_castlingRights; }
    int enPassantFile() { return m_enPassantFile; }
    int hasCastled() { return m_hasCastled; }
    int halfmoveClock() { return m_halfmoveClock; }
    Bitboard bitboard(int piece) { return m_bitboards[piece]; }
    ZobristKey zobristKey() { return m_zobristKey; }
    std::vector<MoveInt> moveHistory() { return m_moveHistory; }
    bool isDefaultStartPosition() { return m_isDefaultStartPosition; }
    int kingIndex(int piece) { return m_kingIndices[piece]; }

    /**
     * @brief Resets the board to the provided fen
     * @param fen The fen to reset the board to
     */
    void resetBoard(std::string fen = START_FEN);

    /**
     * @brief Checks if a color is in check in the current position
     * @param color The color to check
     */
    bool isInCheck(int color)
    {
      return isAttacked(m_kingIndices[color | KING], color ^ COLOR);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     */
    Bitboard getLegalPieceMovesBitboard(int pieceIndex, bool includeCastling = true)
    {
      return getLegalPieceMovesBitboard(pieceIndex, m_board[pieceIndex] & COLOR, includeCastling);
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
     * @param verbose Whether to print the number of games found after each 1-deep move
     */
    int countGames(int depth, bool verbose = true);

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

      for (size_t i = 0; i < m_positionHistory.size(); i++)
        if (m_positionHistory[i] == key)
          count++;

      return count;
    }

    /**
     * @brief Gets a restorable save state of the board's current state
     */
    BoardSaveState getSaveState()
    {
      return BoardSaveState{
          m_sideToMove,
          m_castlingRights,
          m_enPassantFile,
          m_hasCastled,
          m_halfmoveClock,
          m_board,
          m_bitboards,
          m_kingIndices,
          m_zobristKey,
          m_moveHistory,
          m_positionHistory};
    }

    /**
     * @brief Restores the board to a previous state
     * @param saveState The state to restore
     */
    void restoreSaveState(BoardSaveState saveState)
    {
      m_sideToMove = saveState.sideToMove;
      m_castlingRights = saveState.castlingRights;
      m_enPassantFile = saveState.enPassantFile;
      m_hasCastled = saveState.hasCastled;
      m_halfmoveClock = saveState.halfmoveClock;
      m_zobristKey = saveState.zobristKey;

      m_board = saveState.board;
      m_bitboards = saveState.bitboards;
      m_kingIndices = saveState.kingIndices;
      m_moveHistory = saveState.moveHistory;
      m_positionHistory = saveState.positionHistory;
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
          Piece color = fromPiece & COLOR;
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
        Piece color = fromPiece & COLOR;
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

      m_zobristKey ^= zobrist.getPieceCombinationKey(pieceIndex, oldPiece, newPiece);

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
    void unmovePiece(int from, int to, int movedPiece = EMPTY, int capturedPiece = EMPTY)
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
      m_zobristKey ^= zobrist.castlingKeys[m_castlingRights];
      m_castlingRights &= ~rights;
      m_zobristKey ^= zobrist.castlingKeys[m_castlingRights];
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
      m_zobristKey ^= zobrist.enPassantKeys[m_enPassantFile];
      m_enPassantFile = file;
      m_zobristKey ^= zobrist.enPassantKeys[file];
    }

    /**
     * @brief Updates the castling rights and handles Zobrist key updates
     * @param rights The new castling rights, see enum CastlingRights
     */
    void updateCastlingRights(int rights)
    {
      m_zobristKey ^= zobrist.castlingKeys[m_castlingRights];
      m_castlingRights = rights;
      m_zobristKey ^= zobrist.castlingKeys[rights];
    }

    /**
     * @brief Switches the side to move and handles Zobrist key updates
     */
    void switchSideToMove()
    {
      m_sideToMove ^= COLOR;
      m_zobristKey ^= zobrist.sideKey;
    }

    /**
     * @brief Gets a bitboard of pseudo-legal moves for a piece (does not check for pins or checks)
     * @param pieceIndex The index of the piece
     * @param includeCastling Whether to include castling moves (should be false when checking for attacks on the king)
     * @param onlyCaptures Whether to only include capture moves (should be true when checking for attacks on the king)
     */
    Bitboard getPseudoLegalPieceMoves(int pieceIndex, Piece color, bool includeCastling = true)
    {
      return (this->*getPieceMoves[m_board[pieceIndex] & TYPE])(pieceIndex, color, includeCastling);
    }

    /**
     * @brief Returns the bitboard of the squares a piece can move to
     * @param pieceIndex The index of the piece
     * @param color The color of the piece
     */
    Bitboard getLegalPieceMovesBitboard(int pieceIndex, Piece color, bool includeCastling = true);

    /**
     * @brief Returns the bitboard of pieces that can move to a given square. Does not include kings for technical reasons
     * @param targetSquare The square to check
     * @param targetPiece The piece on the target square
     * @param color The color of the pieces moving
     * @param capturesOnly Whether to only include capture moves
     */
    Bitboard getAttackingPiecesBitboard(int targetSquare, Piece targetPiece, Piece color, bool onlyCaptures = false);

    Bitboard getPawnMoves(int pieceIndex, Piece color, bool _ = false);
    Bitboard getKnightMoves(int pieceIndex, Piece color, bool _ = false);
    Bitboard getBishopMoves(int pieceIndex, Piece color, bool _ = false);
    Bitboard getRookMoves(int pieceIndex, Piece color, bool _ = false);
    Bitboard getQueenMoves(int pieceIndex, Piece color, bool _ = false);
    Bitboard getKingMoves(int pieceIndex, Piece color, bool includeCastling = true);

    Bitboard (TungstenChess::Board::*getPieceMoves[PIECE_TYPE_NUMBER])(int, Piece, bool) = {
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