#include "core/board.hpp"

namespace TungstenChess
{
  Board::UnmoveData Board::makeMove(Move move)
  {
    switchSideToMove();

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    PieceType promotionPieceType = move >> 12;

    Piece piece = m_board[from];
    PieceType pieceType = piece & TYPE;
    PieceColor pieceColor = piece & COLOR;

    Piece capturedPiece = m_board[to];
    PieceType capturedPieceType = capturedPiece & TYPE;
    PieceColor capturedPieceColor = capturedPiece & COLOR;

    uint8_t flags = Moves::getMoveFlags(from, to, pieceType, capturedPiece);

    UnmoveData unmoveData = { piece, capturedPiece, m_castlingRights, m_enPassantFile, m_halfmoveClock, flags };

    m_halfmoveClock++;
    if (m_board[to] || pieceType == PAWN)
      m_halfmoveClock = 0;

    movePiece(from, to, promotionPieceType | pieceColor);

    updateEnPassantFile(flags & PAWN_DOUBLE ? to % 8 : NO_EP);

    if (pieceType == KING)
      removeCastlingRights(pieceColor, BOTHSIDES);

    if (pieceType == ROOK &&
        ((pieceColor == WHITE && (from == A1 || from == H1)) ||
         (pieceColor == BLACK && (from == A8 || from == H8))))
    {
      removeCastlingRights(pieceColor, from % 8 == 0 ? QUEENSIDE : KINGSIDE);
    }

    if (capturedPieceType == ROOK &&
        ((capturedPieceColor == WHITE && (to == A1 || to == H1)) ||
         (capturedPieceColor == BLACK && (to == A8 || to == H8))))
    {
      removeCastlingRights(capturedPieceColor, to % 8 == 0 ? QUEENSIDE : KINGSIDE);
    }

    if (flags & EP_CAPTURE)
      updatePiece(to + (piece & WHITE ? 8 : -8), NO_PIECE);

    if (flags & CASTLE)
    {
      m_hasCastled |= pieceColor;

      if (flags & KSIDE_CASTLE)
        movePiece(to + 1, to - 1);
      else
        movePiece(to - 2, to + 1);
    }

    m_positionHistory.stack.push(m_zobristKey);

    return unmoveData;
  }

  Board::UnmoveData Board::makeMove(std::string move)
  {
    return makeMove(generateMoveFromUCI(move));
  }

  void Board::unmakeMove(Move move, UnmoveData unmoveData)
  {
    m_positionHistory.stack.pop();

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;

    auto [piece, capturedPiece, castlingRights, enPassantFile, halfmoveClock, flags] = unmoveData;

    switchSideToMove();

    m_halfmoveClock = halfmoveClock;

    unmovePiece(from, to, piece, capturedPiece);

    if (flags & CASTLE)
    {
      m_hasCastled &= ~(piece & COLOR);

      if (flags & KSIDE_CASTLE)
        unmovePiece(to + 1, to - 1);
      else
        unmovePiece(to - 2, to + 1);
    }

    updateEnPassantFile(enPassantFile);
    updateCastlingRights(castlingRights);

    if (flags & EP_CAPTURE)
      updatePiece((piece & WHITE) ? to + 8 : to - 8, piece ^ COLOR);
  }

  void Board::updateBitboards(Square pieceIndex, Piece oldPiece, Piece newPiece)
  {
    Bitboard squareBitboard = Bitboards::bit(pieceIndex);

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

  MoveFlags Board::quickMakeMove(Square from, Square to)
  {
    Piece fromPiece = m_board[from];
    Piece toPiece = m_board[to];

    updateBitboards(from, fromPiece, NO_PIECE);
    updateBitboards(to, toPiece, fromPiece);

    if ((fromPiece & TYPE) == PAWN)
    {
      if (!toPiece && to % 8 != from % 8)
      {
        PieceColor color = fromPiece & COLOR;
        Square epSquare = to + (color & WHITE ? 8 : -8);
        updateBitboards(epSquare, (color ^ COLOR) | PAWN, NO_PIECE);

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
        updateBitboards(from + 3, rook, NO_PIECE);
        updateBitboards(from + 1, NO_PIECE, rook);

        return KSIDE_CASTLE;
      }
      else if (from - to == 2)
      {
        Piece rook = (fromPiece & COLOR) | ROOK;
        updateBitboards(from - 4, rook, NO_PIECE);
        updateBitboards(from - 1, NO_PIECE, rook);

        return QSIDE_CASTLE;
      }
    }

    return NORMAL;
  }

  void Board::quickUnmakeMove(Square from, Square to, MoveFlags flag)
  {
    Piece fromPiece = m_board[from];
    Piece toPiece = m_board[to];

    updateBitboards(to, fromPiece, toPiece);
    updateBitboards(from, NO_PIECE, fromPiece);

    if ((fromPiece & TYPE) == KING)
      m_kingIndices[fromPiece] = from;

    if (flag & EP_CAPTURE)
    {
      PieceColor color = fromPiece & COLOR;
      Square epSquare = to + (color & WHITE ? 8 : -8);
      updateBitboards(epSquare, NO_PIECE, (color ^ COLOR) | PAWN);
    }

    else if (flag & KSIDE_CASTLE)
    {
      Piece rook = (fromPiece & COLOR) | ROOK;
      updateBitboards(from + 3, NO_PIECE, rook);
      updateBitboards(from + 1, rook, NO_PIECE);
    }

    else if (flag & QSIDE_CASTLE)
    {
      Piece rook = (fromPiece & COLOR) | ROOK;
      updateBitboards(from - 4, NO_PIECE, rook);
      updateBitboards(from - 1, rook, NO_PIECE);
    }
  }

  void Board::updatePiece(Square pieceIndex, Piece newPiece)
  {
    Piece oldPiece = m_board[pieceIndex];

    m_pieceCounts[oldPiece]--;
    m_pieceCounts[newPiece]++;

    m_zobristKey ^= Zobrist::getPieceCombinationKey(pieceIndex, oldPiece, newPiece);

    m_kingIndices[newPiece] = pieceIndex;
    m_board[pieceIndex] = newPiece;

    updateBitboards(pieceIndex, oldPiece, newPiece);
  }

  void Board::movePiece(Square from, Square to, Piece promotionPiece)
  {
    updatePiece(to, (promotionPiece & TYPE) == NO_TYPE ? m_board[from] : promotionPiece);
    updatePiece(from, NO_PIECE);
  }

  void Board::unmovePiece(Square from, Square to, Piece movedPiece, Piece capturedPiece)
  {
    updatePiece(from, movedPiece == NO_PIECE ? m_board[to] : movedPiece);
    updatePiece(to, capturedPiece);
  }

  void Board::removeCastlingRights(uint8_t rights)
  {
    m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
    m_castlingRights &= ~rights;
    m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
  }

  void Board::removeCastlingRights(PieceColor color, CastlingRights side)
  {
    removeCastlingRights(color == WHITE ? side >> 4 : side >> 2);
  }

  void Board::updateEnPassantFile(File file)
  {
    m_zobristKey ^= Zobrist::enPassantKeys[m_enPassantFile];
    m_enPassantFile = file;
    m_zobristKey ^= Zobrist::enPassantKeys[file];
  }

  void Board::updateCastlingRights(uint8_t rights)
  {
    m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
    m_castlingRights = rights;
    m_zobristKey ^= Zobrist::castlingKeys[rights];
  }

  void Board::switchSideToMove()
  {
    m_sideToMove ^= COLOR;
    m_zobristKey ^= Zobrist::sideKey;
  }
}