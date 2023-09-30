#include "Headers/board.hpp"

namespace Chess
{
  Board::Board(std::string fen)
  {

    std::string fenParts[6];

    int fenPartIndex = 0;

    for (int i = 0; i < fen.length(); i++)
    {
      if (fen[i] == ' ')
      {
        fenPartIndex++;
        continue;
      }

      fenParts[fenPartIndex] += fen[i];
    }

    whitePawnsBitboard = Bitboard();
    whiteKnightsBitboard = Bitboard();
    whiteBishopsBitboard = Bitboard();
    whiteRooksBitboard = Bitboard();
    whiteQueensBitboard = Bitboard();

    blackPawnsBitboard = Bitboard();
    blackKnightsBitboard = Bitboard();
    blackBishopsBitboard = Bitboard();
    blackRooksBitboard = Bitboard();
    blackQueensBitboard = Bitboard();

    enPassantFile = -1;

    castlingRights = 0;

    sideToMove = Piece::WHITE;

    int pieceIndex = 0;
    for (int i = 0; i < fenParts[0].length(); i++)
    {
      if (fen[i] == '/')
      {
        continue;
      }
      else if (isdigit(fen[i]))
      {
        pieceIndex += fen[i] - '0';
      }
      else
      {
        board[pieceIndex] = Piece(fen[i]);

        if (fen[i] == 'K')
          whiteKingIndex = pieceIndex;
        else if (fen[i] == 'k')
          blackKingIndex = pieceIndex;

        if (fen[i] == 'P')
          whitePawnsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'N')
          whiteKnightsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'B')
          whiteBishopsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'R')
          whiteRooksBitboard.addBit(pieceIndex);
        else if (fen[i] == 'Q')
          whiteQueensBitboard.addBit(pieceIndex);

        else if (fen[i] == 'p')
          blackPawnsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'n')
          blackKnightsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'b')
          blackBishopsBitboard.addBit(pieceIndex);
        else if (fen[i] == 'r')
          blackRooksBitboard.addBit(pieceIndex);
        else if (fen[i] == 'q')
          blackQueensBitboard.addBit(pieceIndex);

        pieceIndex++;
      }
    }

    sideToMove = fenParts[1] == "w" ? Piece::WHITE : Piece::BLACK;

    if (fenParts[3] != "-")
    {
      enPassantFile = fenParts[3][0] - 'a';
    }

    if (fenParts[2] != "-")
    {
      for (int i = 0; i < fenParts[2].length(); i++)
      {
        if (fenParts[2][i] == 'K')
          castlingRights |= WHITE_KINGSIDE;
        else if (fenParts[2][i] == 'Q')
          castlingRights |= WHITE_QUEENSIDE;
        else if (fenParts[2][i] == 'k')
          castlingRights |= BLACK_KINGSIDE;
        else if (fenParts[2][i] == 'q')
          castlingRights |= BLACK_QUEENSIDE;
      }
    }
  }

  void Board::printBoard()
  {
    for (int i = 0; i < 64; i++)
    {
      if (i % 8 == 0)
      {
        std::cout << std::endl;
      }

      std::cout << board[i].getPieceChar() << " ";
    }
    std::cout << std::endl;
  }

  void Board::makeMove(Move move)
  {
    halfMoves++;

    sideToMove ^= 24;

    int from = move.from;
    int to = move.to;
    int movePiece = move.piece;
    int capturedPiece = move.capturedPiece;
    int promotionPiece = move.promotionPiece & 7;

    if (capturedPiece != Piece::EMPTY)
      removePieceFromBitboard(to);
    removePieceFromBitboard(from);

    board[to] = Piece(board[from].getPieceChar());
    board[from] = Piece();

    addPieceToBitboard(to);

    int movePieceType = movePiece & 7;
    int movePieceColor = movePiece & 24;

    enPassantFile = -1;

    if (movePiece == Piece::WHITE_KING)
    {
      whiteKingIndex = to;
      castlingRights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
    }
    else if (movePiece == Piece::BLACK_KING)
    {
      blackKingIndex = to;
      castlingRights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    }

    if (movePiece == Piece::WHITE_ROOK)
    {
      if (from == 56)
        castlingRights &= ~WHITE_QUEENSIDE;
      else if (from == 63)
        castlingRights &= ~WHITE_KINGSIDE;
    }
    else if (movePiece == Piece::BLACK_ROOK)
    {
      if (from == 0)
        castlingRights &= ~BLACK_QUEENSIDE;
      else if (from == 7)
        castlingRights &= ~BLACK_KINGSIDE;
    }

    if (capturedPiece == Piece::WHITE_ROOK)
    {
      if (to == 56)
        castlingRights &= ~WHITE_QUEENSIDE;
      else if (to == 63)
        castlingRights &= ~WHITE_KINGSIDE;
    }
    else if (capturedPiece == Piece::BLACK_ROOK)
    {
      if (to == 0)
        castlingRights &= ~BLACK_QUEENSIDE;
      else if (to == 7)
        castlingRights &= ~BLACK_KINGSIDE;
    }

    if (move.flags & Move::CAPTURE)
    {
      int capturedPieceType = capturedPiece & 7;
      int capturedPieceColor = capturedPiece & 24;

      if (capturedPieceColor == Piece::WHITE)
      {
        if (capturedPieceType == Piece::PAWN)
          whitePawnsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::KNIGHT)
          whiteKnightsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::BISHOP)
          whiteBishopsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::ROOK)
          whiteRooksBitboard.removeBit(to);
        else if (capturedPieceType == Piece::QUEEN)
          whiteQueensBitboard.removeBit(to);
      }
      else if (capturedPieceColor == Piece::BLACK)
      {
        if (capturedPieceType == Piece::PAWN)
          blackPawnsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::KNIGHT)
          blackKnightsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::BISHOP)
          blackBishopsBitboard.removeBit(to);
        else if (capturedPieceType == Piece::ROOK)
          blackRooksBitboard.removeBit(to);
        else if (capturedPieceType == Piece::QUEEN)
          blackQueensBitboard.removeBit(to);
      }
    }

    if (move.flags & Move::EP_CAPTURE)
    {
      if (movePiece & Piece::WHITE)
      {
        board[to + 8] = Piece();
        blackPawnsBitboard.removeBit(to + 8);
      }
      else if (movePiece & Piece::BLACK)
      {
        board[to - 8] = Piece();
        whitePawnsBitboard.removeBit(to - 8);
      }
    }

    if (move.flags & Move::PROMOTION)
    {
      board[to] = Piece(movePieceColor | promotionPiece);

      if (movePiece == Piece::WHITE_PAWN)
      {
        whitePawnsBitboard.removeBit(to);

        if (promotionPiece == Piece::KNIGHT)
          whiteKnightsBitboard.addBit(to);
        else if (promotionPiece == Piece::BISHOP)
          whiteBishopsBitboard.addBit(to);
        else if (promotionPiece == Piece::ROOK)
          whiteRooksBitboard.addBit(to);
        else if (promotionPiece == Piece::QUEEN)
          whiteQueensBitboard.addBit(to);
      }
      else if (movePiece == Piece::BLACK_PAWN)
      {
        blackPawnsBitboard.removeBit(to);

        if (promotionPiece == Piece::KNIGHT)
          blackKnightsBitboard.addBit(to);
        else if (promotionPiece == Piece::BISHOP)
          blackBishopsBitboard.addBit(to);
        else if (promotionPiece == Piece::ROOK)
          blackRooksBitboard.addBit(to);
        else if (promotionPiece == Piece::QUEEN)
          blackQueensBitboard.addBit(to);
      }
    }

    if (move.flags & Move::KSIDE_CASTLE)
    {
      board[to - 1] = Piece(movePieceColor | Piece::ROOK);
      board[to + 1] = Piece();

      if (movePiece == Piece::WHITE_KING)
      {
        whiteRooksBitboard.removeBit(63);
        whiteRooksBitboard.addBit(61);

        whiteHasCastled = true;
      }
      else if (movePiece == Piece::BLACK_KING)
      {
        blackRooksBitboard.removeBit(7);
        blackRooksBitboard.addBit(5);

        blackHasCastled = true;
      }
    }

    if (move.flags & Move::QSIDE_CASTLE)
    {
      board[to + 1] = Piece(movePieceColor | Piece::ROOK);
      board[to - 2] = Piece();

      if (movePiece == Piece::WHITE_KING)
      {
        whiteRooksBitboard.removeBit(56);
        whiteRooksBitboard.addBit(59);

        whiteHasCastled = true;
      }
      else if (movePiece == Piece::BLACK_KING)
      {
        blackRooksBitboard.removeBit(0);
        blackRooksBitboard.addBit(3);

        blackHasCastled = true;
      }
    }

    if (move.flags & Move::PAWN_DOUBLE)
    {
      enPassantFile = to % 8;
    }
  }

  void Board::unmakeMove(Move move)
  {
    halfMoves--;

    sideToMove ^= 24;

    int from = move.from;
    int to = move.to;
    int movePiece = move.piece;
    int capturedPiece = move.capturedPiece;
    int promotionPiece = move.promotionPiece;

    removePieceFromBitboard(to);

    if (move.flags & Move::CAPTURE)
    {
      board[to] = Piece(capturedPiece);

      addPieceToBitboard(to);
    }
    else
    {
      board[to] = Piece();
    }

    board[from] = Piece(movePiece);

    addPieceToBitboard(from);

    if ((move.flags & Move::KSIDE_CASTLE) || (move.flags & Move::QSIDE_CASTLE))
    {
      if (move.flags & Move::KSIDE_CASTLE)
      {
        board[to + 1] = Piece(movePiece & 24 | Piece::ROOK);
        board[to - 1] = Piece();

        if (movePiece == Piece::WHITE_KING)
        {
          whiteRooksBitboard.removeBit(61);
          whiteRooksBitboard.addBit(63);

          whiteHasCastled = false;
        }
        else if (movePiece == Piece::BLACK_KING)
        {
          blackRooksBitboard.removeBit(5);
          blackRooksBitboard.addBit(7);

          blackHasCastled = false;
        }
      }

      if (move.flags & Move::QSIDE_CASTLE)
      {
        board[to - 2] = Piece(movePiece & 24 | Piece::ROOK);
        board[to + 1] = Piece();

        if (movePiece == Piece::WHITE_KING)
        {
          whiteRooksBitboard.removeBit(59);
          whiteRooksBitboard.addBit(56);

          whiteHasCastled = false;
        }
        else if (movePiece == Piece::BLACK_KING)
        {
          blackRooksBitboard.removeBit(3);
          blackRooksBitboard.addBit(0);

          blackHasCastled = false;
        }
      }
    }

    int movePieceType = movePiece & 7;
    int movePieceColor = movePiece & 24;

    enPassantFile = move.enPassantFile;
    castlingRights = move.castlingRights;

    if (movePiece == Piece::WHITE_KING)
    {
      whiteKingIndex = from;
    }
    else if (movePiece == Piece::BLACK_KING)
    {
      blackKingIndex = from;
    }

    if (move.flags & Move::EP_CAPTURE)
    {
      if (movePiece & Piece::WHITE)
      {
        board[to + 8] = Piece(Piece::BLACK_PAWN);
        blackPawnsBitboard.addBit(to + 8);
      }
      else if (movePiece & Piece::BLACK)
      {
        board[to - 8] = Piece(Piece::WHITE_PAWN);
        whitePawnsBitboard.addBit(to - 8);
      }
    }

    if (move.flags & Move::PROMOTION)
    {
      if (movePiece == Piece::WHITE_PAWN)
      {
        whitePawnsBitboard.removeBit(to);

        if (promotionPiece == Piece::KNIGHT)
          whiteKnightsBitboard.removeBit(to);
        else if (promotionPiece == Piece::BISHOP)
          whiteBishopsBitboard.removeBit(to);
        else if (promotionPiece == Piece::ROOK)
          whiteRooksBitboard.removeBit(to);
        else if (promotionPiece == Piece::QUEEN)
          whiteQueensBitboard.removeBit(to);
      }
      else if (movePiece == Piece::BLACK_PAWN)
      {
        blackPawnsBitboard.removeBit(to);

        if (promotionPiece == Piece::KNIGHT)
          blackKnightsBitboard.removeBit(to);
        else if (promotionPiece == Piece::BISHOP)
          blackBishopsBitboard.removeBit(to);
        else if (promotionPiece == Piece::ROOK)
          blackRooksBitboard.removeBit(to);
        else if (promotionPiece == Piece::QUEEN)
          blackQueensBitboard.removeBit(to);
      }
    }
  }

  bool Board::pieceCanMove(int pieceIndex, int to)
  {
    return board[to].isEmpty() || board[to].getPieceColor() != board[pieceIndex].getPieceColor();
  }

  bool Board::isOnBoard(int x, int y)
  {
    return x >= 0 && x <= 7 && y >= 0 && y <= 7;
  }

  int Board::countLegalMoves(Move *moves)
  {
    int legalMovesCount = 0;

    for (int i = 0; i < 256; i++)
    {
      if (moves[i].from == 0 && moves[i].to == 0)
        break;

      legalMovesCount++;
    }

    return legalMovesCount;
  }

  Bitboard Board::getPseudoLegalPieceMoves(int pieceIndex, bool includeCastling, bool onlyCaptures)
  {
    Bitboard movesBitboard = Bitboard();

    switch (board[pieceIndex].getPieceType())
    {
    case Piece::PAWN:
      movesBitboard = getPawnMoves(pieceIndex, onlyCaptures);
      break;
    case Piece::KNIGHT:
      movesBitboard = getKnightMoves(pieceIndex);
      break;
    case Piece::BISHOP:
      movesBitboard = getBishopMoves(pieceIndex);
      break;
    case Piece::ROOK:
      movesBitboard = getRookMoves(pieceIndex);
      break;
    case Piece::QUEEN:
      movesBitboard = getQueenMoves(pieceIndex);
      break;
    case Piece::KING:
      movesBitboard = getKingMoves(pieceIndex, includeCastling);
      break;
    }

    return movesBitboard;
  }

  Bitboard Board::getPawnMoves(int pieceIndex, bool onlyCaptures)
  {
    Bitboard movesBitboard = Bitboard();

    int piece = board[pieceIndex].piece;

    if (piece == Piece::WHITE_PAWN)
    {
      if (board[pieceIndex - 8].isEmpty() && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex - 8);
        if (pieceIndex >= 48 && pieceIndex <= 55 && board[pieceIndex - 16].isEmpty())
        {
          movesBitboard.addBit(pieceIndex - 16);
        }
      }

      if (pieceIndex % 8 != 0 && ((!board[pieceIndex - 9].isEmpty() && board[pieceIndex - 9].getPieceColor() == Piece::BLACK) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex - 9);
      }
      if (pieceIndex % 8 != 7 && ((!board[pieceIndex - 7].isEmpty() && board[pieceIndex - 7].getPieceColor() == Piece::BLACK) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex - 7);
      }

      if (enPassantFile != -1)
      {
        if (pieceIndex % 8 != 0 && pieceIndex / 8 == 3 && (pieceIndex - 9) % 8 == enPassantFile)
        {
          movesBitboard.addBit(pieceIndex - 9);
        }

        if (pieceIndex % 8 != 7 && pieceIndex / 8 == 3 && (pieceIndex - 7) % 8 == enPassantFile)
        {
          movesBitboard.addBit(pieceIndex - 7);
        }
      }
    }
    else if (piece == Piece::BLACK_PAWN)
    {
      if (board[pieceIndex + 8].isEmpty() && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex + 8);
        if (pieceIndex >= 8 && pieceIndex <= 15 && board[pieceIndex + 16].isEmpty())
        {
          movesBitboard.addBit(pieceIndex + 16);
        }
      }

      if (pieceIndex % 8 != 0 && ((!board[pieceIndex + 7].isEmpty() && board[pieceIndex + 7].getPieceColor() == Piece::WHITE) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex + 7);
      }
      if (pieceIndex % 8 != 7 && ((!board[pieceIndex + 9].isEmpty() && board[pieceIndex + 9].getPieceColor() == Piece::WHITE) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex + 9);
      }

      if (enPassantFile != -1)
      {
        if (pieceIndex % 8 != 0 && pieceIndex / 8 == 4 && (pieceIndex + 7) % 8 == enPassantFile)
        {
          movesBitboard.addBit(pieceIndex + 7);
        }

        if (pieceIndex % 8 != 7 && pieceIndex / 8 == 4 && (pieceIndex + 9) % 8 == enPassantFile)
        {
          movesBitboard.addBit(pieceIndex + 9);
        }
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getKnightMoves(int pieceIndex)
  {
    Bitboard friendlyPiecesBitboard = Bitboard();

    if (board[pieceIndex].getPieceColor() == Piece::WHITE)
    {
      friendlyPiecesBitboard = whitePawnsBitboard | whiteKnightsBitboard | whiteBishopsBitboard | whiteRooksBitboard | whiteQueensBitboard | Bitboard(1ULL << whiteKingIndex);
    }
    else
    {
      friendlyPiecesBitboard = blackPawnsBitboard | blackKnightsBitboard | blackBishopsBitboard | blackRooksBitboard | blackQueensBitboard | Bitboard(1ULL << blackKingIndex);
    }

    Bitboard movesBitboard = Bitboard(MovesLookup::KNIGHT_MOVES[pieceIndex] & ~friendlyPiecesBitboard.bitboard);

    return movesBitboard;
  }

  Bitboard Board::getBishopMoves(int pieceIndex)
  {
    Bitboard movesBitboard = Bitboard();

    int piece = board[pieceIndex].piece;

    int directions[4] = {-9, -7, 7, 9};

    for (int i = 0; i < 4; i++)
    {
      int direction = directions[i];

      int to = pieceIndex;

      while (true)
      {
        bool isEdge = false;

        switch (direction)
        {
        case -9:
          if (to % 8 == 0 || to / 8 == 0)
            isEdge = true;
          break;
        case -7:
          if (to % 8 == 7 || to / 8 == 0)
            isEdge = true;
          break;
        case 7:
          if (to % 8 == 0 || to / 8 == 7)
            isEdge = true;
          break;
        case 9:
          if (to % 8 == 7 || to / 8 == 7)
            isEdge = true;
          break;
        }

        if (isEdge)
          break;

        to += direction;

        if (pieceCanMove(pieceIndex, to))
          movesBitboard.addBit(to);
        else
          break;

        if (!board[to].isEmpty())
          break;
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getRookMoves(int pieceIndex)
  {
    Bitboard movesBitboard = Bitboard();

    int piece = board[pieceIndex].piece;

    int directions[4] = {-8, -1, 1, 8};

    for (int i = 0; i < 4; i++)
    {
      int direction = directions[i];

      int to = pieceIndex;

      while (true)
      {
        bool isEdge = false;

        switch (direction)
        {
        case -8:
          if (to / 8 == 0)
            isEdge = true;
          break;
        case -1:
          if (to % 8 == 0)
            isEdge = true;
          break;
        case 1:
          if (to % 8 == 7)
            isEdge = true;
          break;
        case 8:
          if (to / 8 == 7)
            isEdge = true;
          break;
        }

        if (isEdge)
          break;

        to += direction;

        if (pieceCanMove(pieceIndex, to))
          movesBitboard.addBit(to);
        else
          break;

        if (!board[to].isEmpty())
          break;
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getQueenMoves(int pieceIndex)
  {
    Bitboard movesBitboard = Bitboard();

    movesBitboard = getBishopMoves(pieceIndex) | getRookMoves(pieceIndex);

    return movesBitboard;
  }

  Bitboard Board::getKingMoves(int pieceIndex, bool includeCastling)
  {
    Bitboard movesBitboard = Bitboard();

    Bitboard friendlyPiecesBitboard = Bitboard();

    if (board[pieceIndex].getPieceColor() == Piece::WHITE)
    {
      friendlyPiecesBitboard = whitePawnsBitboard | whiteKnightsBitboard | whiteBishopsBitboard | whiteRooksBitboard | whiteQueensBitboard;
    }
    else if (board[pieceIndex].getPieceColor() == Piece::BLACK)
    {
      friendlyPiecesBitboard = blackPawnsBitboard | blackKnightsBitboard | blackBishopsBitboard | blackRooksBitboard | blackQueensBitboard;
    }

    movesBitboard = Bitboard(MovesLookup::KING_MOVES[pieceIndex] & ~friendlyPiecesBitboard.bitboard);

    int piece = board[pieceIndex].piece;

    if (includeCastling && castlingRights)
    {
      if (piece == Piece::WHITE_KING)
      {
        if (castlingRights & WHITE_KINGSIDE)
        {
          if (board[61].isEmpty() && board[62].isEmpty() && !isInCheck(Piece::WHITE) && !isAttacked(61, Piece::BLACK))
          {
            movesBitboard.addBit(62);
          }
        }
        if (castlingRights & WHITE_QUEENSIDE)
        {
          if (board[59].isEmpty() && board[58].isEmpty() && board[57].isEmpty() && !isInCheck(Piece::WHITE) && !isAttacked(59, Piece::BLACK))
          {
            movesBitboard.addBit(58);
          }
        }
      }
      else if (piece == Piece::BLACK_KING)
      {
        if (castlingRights & BLACK_KINGSIDE)
        {
          if (board[5].isEmpty() && board[6].isEmpty() && !isInCheck(Piece::BLACK) && !isAttacked(5, Piece::WHITE))
          {
            movesBitboard.addBit(6);
          }
        }
        if (castlingRights & BLACK_QUEENSIDE)
        {
          if (board[3].isEmpty() && board[2].isEmpty() && board[1].isEmpty() && !isInCheck(Piece::BLACK) && !isAttacked(3, Piece::WHITE))
          {
            movesBitboard.addBit(2);
          }
        }
      }
    }

    return movesBitboard;
  }

  void Board::addPieceToBitboard(int pieceIndex)
  {
    int piece = board[pieceIndex].piece;

    if (piece == Piece::WHITE_PAWN)
      whitePawnsBitboard.addBit(pieceIndex);
    else if (piece == Piece::WHITE_KNIGHT)
      whiteKnightsBitboard.addBit(pieceIndex);
    else if (piece == Piece::WHITE_BISHOP)
      whiteBishopsBitboard.addBit(pieceIndex);
    else if (piece == Piece::WHITE_ROOK)
      whiteRooksBitboard.addBit(pieceIndex);
    else if (piece == Piece::WHITE_QUEEN)
      whiteQueensBitboard.addBit(pieceIndex);

    else if (piece == Piece::BLACK_PAWN)
      blackPawnsBitboard.addBit(pieceIndex);
    else if (piece == Piece::BLACK_KNIGHT)
      blackKnightsBitboard.addBit(pieceIndex);
    else if (piece == Piece::BLACK_BISHOP)
      blackBishopsBitboard.addBit(pieceIndex);
    else if (piece == Piece::BLACK_ROOK)
      blackRooksBitboard.addBit(pieceIndex);
    else if (piece == Piece::BLACK_QUEEN)
      blackQueensBitboard.addBit(pieceIndex);
  }

  void Board::removePieceFromBitboard(int pieceIndex)
  {
    int piece = board[pieceIndex].piece;

    if (piece == Piece::WHITE_PAWN)
      whitePawnsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::WHITE_KNIGHT)
      whiteKnightsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::WHITE_BISHOP)
      whiteBishopsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::WHITE_ROOK)
      whiteRooksBitboard.removeBit(pieceIndex);
    else if (piece == Piece::WHITE_QUEEN)
      whiteQueensBitboard.removeBit(pieceIndex);

    else if (piece == Piece::BLACK_PAWN)
      blackPawnsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::BLACK_KNIGHT)
      blackKnightsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::BLACK_BISHOP)
      blackBishopsBitboard.removeBit(pieceIndex);
    else if (piece == Piece::BLACK_ROOK)
      blackRooksBitboard.removeBit(pieceIndex);
    else if (piece == Piece::BLACK_QUEEN)
      blackQueensBitboard.removeBit(pieceIndex);
  }

  Move *Board::getLegalMoves(int color)
  {
    Move *legalMoves = new Move[256];

    int legalMovesCount = 0;

    for (int i = 0; i < 64; i++)
    {
      if (board[i].isEmpty() || board[i].getPieceColor() != color)
        continue;

      Bitboard movesBitboard = getLegalPieceMovesBitboard(i);

      for (int j = 0; j < 64; j++)
      {
        if (movesBitboard.hasBit(j))
        {
          legalMoves[legalMovesCount] = Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights);

          if (legalMoves[legalMovesCount].flags & Move::PROMOTION)
          {
            legalMoves[legalMovesCount].promotionPiece = Piece::QUEEN;
            legalMovesCount++;
            legalMoves[legalMovesCount] = Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, Piece::KNIGHT);
            legalMovesCount++;
            legalMoves[legalMovesCount] = Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, Piece::BISHOP);
            legalMovesCount++;
            legalMoves[legalMovesCount] = Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, Piece::ROOK);
          }

          legalMovesCount++;
        }
      }
    }

    return legalMoves;
  }

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex)
  {
    Bitboard legalMovesBitboard;

    legalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex);

    for (int j = 0; j < 64; j++)
    {
      if (legalMovesBitboard.hasBit(j))
      {
        Move move = Move(pieceIndex, j, board[pieceIndex].piece, board[j].piece, enPassantFile, castlingRights);

        makeMove(move);

        if (isInCheck(board[j].getPieceColor()))
        {
          legalMovesBitboard.removeBit(j);
        }

        unmakeMove(move);
      }
    }

    return legalMovesBitboard;
  }

  Bitboard Board::getAttackedSquaresBitboard(int color)
  {
    Bitboard movesBitboard = Bitboard();

    for (int i = 0; i < 64; i++)
    {
      if (board[i].isEmpty() || (board[i].getPieceColor() != color))
      {
        continue;
      }

      movesBitboard = movesBitboard | getPseudoLegalPieceMoves(i, false, true);
    }

    return movesBitboard;
  }

  bool Board::isInCheck(int color)
  {
    return isAttacked(color == Piece::WHITE ? whiteKingIndex : blackKingIndex, color ^ 24);
  }

  bool Board::isAttacked(int square, int color)
  {
    Bitboard movesBitboard = getAttackedSquaresBitboard(color);

    return movesBitboard.hasBit(square);
  }

  int Board::getGameStatus(int color)
  {
    Move *legalMoves = getLegalMoves(color);

    if (legalMoves[0].from == 0 && legalMoves[0].to == 0)
    {
      if (isInCheck(color))
      {
        return LOSE;
      }
      else
      {
        return STALEMATE;
      }
    }

    // insufficient material
    int whitePawns = whitePawnsBitboard.countBits();
    int whiteKnights = whiteKnightsBitboard.countBits();
    int whiteBishops = whiteBishopsBitboard.countBits();
    int whiteRooks = whiteRooksBitboard.countBits();
    int whiteQueens = whiteQueensBitboard.countBits();
    int whitePieces = whitePawns + whiteKnights + whiteBishops + whiteRooks + whiteQueens;

    int blackPawns = blackPawnsBitboard.countBits();
    int blackKnights = blackKnightsBitboard.countBits();
    int blackBishops = blackBishopsBitboard.countBits();
    int blackRooks = blackRooksBitboard.countBits();
    int blackQueens = blackQueensBitboard.countBits();
    int blackPieces = blackPawns + blackKnights + blackBishops + blackRooks + blackQueens;

    if (whitePawns + blackPawns == 0)
    {
      if (whiteKnights + whiteBishops + whiteRooks + whiteQueens == 0 && blackKnights + blackBishops + blackRooks + blackQueens == 0)
        return STALEMATE;
    }

    return NO_MATE;
  }

  int Board::countGames(int depth, bool verbose)
  {
    if (depth < 1)
      return 0;

    int games = 0;

    Move *legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = countLegalMoves(legalMoves);

    if (depth == 1)
      return legalMovesCount;

    if (legalMovesCount == 0)
      std::cout << "No legal moves" << std::endl;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i]);

      int numGames = countGames(depth - 1, false);

      unmakeMove(legalMoves[i]);

      if (verbose)
        std::cout << legalMoves[i].toString() << " " << numGames << std::endl;

      games += numGames;
    }

    return games;
  }

  Move Board::generateRandomMove()
  {
    Move *legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = countLegalMoves(legalMoves);

    if (legalMovesCount == 0)
      return Move(0, 0, 0, 0, 0, 0, 0);

    int randomMoveIndex = rand() % legalMovesCount;

    return legalMoves[randomMoveIndex];
  }

  Move Board::generateBotMove()
  {
    return generateBestMove(3);
    // return generateRandomMove();
  }

  int Board::getStaticEvaluation()
  {
    debug++;

    int gameStatus = getGameStatus(sideToMove);

    if (gameStatus != NO_MATE)
    {
      if (gameStatus == LOSE)
        return sideToMove == Piece::WHITE ? 1000000 : -1000000;
      else
        return 0;
    }

    int staticEvaluation = getMaterialEvaluation() + getPositionalEvaluation() + getEvaluationBonus();

    if (sideToMove == Piece::WHITE)
      return staticEvaluation;
    else
      return -staticEvaluation;
  }

  int Board::getMaterialEvaluation()
  {

    int materialEvaluation = 0;

    materialEvaluation += whitePawnsBitboard.countBits() * Piece::PIECE_VALUES[Piece::PAWN];
    materialEvaluation += whiteKnightsBitboard.countBits() * Piece::PIECE_VALUES[Piece::KNIGHT];
    materialEvaluation += whiteBishopsBitboard.countBits() * Piece::PIECE_VALUES[Piece::BISHOP];
    materialEvaluation += whiteRooksBitboard.countBits() * Piece::PIECE_VALUES[Piece::ROOK];
    materialEvaluation += whiteQueensBitboard.countBits() * Piece::PIECE_VALUES[Piece::QUEEN];

    materialEvaluation -= blackPawnsBitboard.countBits() * Piece::PIECE_VALUES[Piece::PAWN];
    materialEvaluation -= blackKnightsBitboard.countBits() * Piece::PIECE_VALUES[Piece::KNIGHT];
    materialEvaluation -= blackBishopsBitboard.countBits() * Piece::PIECE_VALUES[Piece::BISHOP];
    materialEvaluation -= blackRooksBitboard.countBits() * Piece::PIECE_VALUES[Piece::ROOK];
    materialEvaluation -= blackQueensBitboard.countBits() * Piece::PIECE_VALUES[Piece::QUEEN];

    return materialEvaluation;
  }

  int Board::getPositionalEvaluation()
  {
    int positionalEvaluation = 0;

    for (int i = 0; i < 64; i++)
    {
      switch (board[i].piece)
      {
      case Piece::WHITE_PAWN:
        positionalEvaluation += PieceEvalTables::PAWN_EVAL_TABLE[i];
        break;
      case Piece::WHITE_KNIGHT:
        positionalEvaluation += PieceEvalTables::KNIGHT_EVAL_TABLE[i];
        break;
      case Piece::WHITE_BISHOP:
        positionalEvaluation += PieceEvalTables::BISHOP_EVAL_TABLE[i];
        break;
      case Piece::WHITE_ROOK:
        positionalEvaluation += PieceEvalTables::ROOK_EVAL_TABLE[i];
        break;
      case Piece::WHITE_QUEEN:
        positionalEvaluation += PieceEvalTables::QUEEN_EVAL_TABLE[i];
        break;
      case Piece::BLACK_PAWN:
        positionalEvaluation -= PieceEvalTables::PAWN_EVAL_TABLE[63 - i];
        break;
      case Piece::BLACK_KNIGHT:
        positionalEvaluation -= PieceEvalTables::KNIGHT_EVAL_TABLE[63 - i];
        break;
      case Piece::BLACK_BISHOP:
        positionalEvaluation -= PieceEvalTables::BISHOP_EVAL_TABLE[63 - i];
        break;
      case Piece::BLACK_ROOK:
        positionalEvaluation -= PieceEvalTables::ROOK_EVAL_TABLE[63 - i];
        break;
      case Piece::BLACK_QUEEN:
        positionalEvaluation -= PieceEvalTables::QUEEN_EVAL_TABLE[63 - i];
        break;
      }

      if (board[i].piece == Piece::WHITE_KING)
      {
        Bitboard enemyPieces = blackPawnsBitboard | blackKnightsBitboard | blackBishopsBitboard | blackRooksBitboard | blackQueensBitboard;
        Bitboard friendlyPieces = whiteKnightsBitboard | whiteBishopsBitboard | whiteRooksBitboard | whiteQueensBitboard;

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation += PieceEvalTables::KING_EVAL_TABLE[i] * endgameScore;

        positionalEvaluation += PieceEvalTables::KING_ENDGAME_EVAL_TABLE[i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = 0;

          for (int j = 0; j < 64; j++)
          {
            if (board[j].piece == Piece::BLACK_KING)
            {
              kingsDistance = abs(i % 8 - j % 8) + abs(i / 8 - j / 8);
              break;
            }
          }

          positionalEvaluation += PieceEvalTables::KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
        }
      }
      else if (board[i].piece == Piece::BLACK_KING)
      {
        Bitboard enemyPieces = whitePawnsBitboard | whiteKnightsBitboard | whiteBishopsBitboard | whiteRooksBitboard | whiteQueensBitboard;
        Bitboard friendlyPieces = blackKnightsBitboard | blackBishopsBitboard | blackRooksBitboard | blackQueensBitboard;

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation -= PieceEvalTables::KING_EVAL_TABLE[63 - i] * endgameScore;

        positionalEvaluation -= PieceEvalTables::KING_ENDGAME_EVAL_TABLE[63 - i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = 0;

          for (int j = 0; j < 64; j++)
          {
            if (board[j].piece == Piece::WHITE_KING)
            {
              kingsDistance = abs(i % 8 - j % 8) + abs(i / 8 - j / 8);
              break;
            }
          }

          positionalEvaluation -= PieceEvalTables::KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
        }
      }
    }

    return positionalEvaluation;
  }

  int Board::getEvaluationBonus()
  {
    // enum EvaluationBonus
    // {
    //   BISHOP_PAIR_BONUS = 100,
    //   CASTLED_KING_BONUS = 50,
    //   CAN_CASTLE_BONUS = 15,
    //   ROOK_ON_OPEN_FILE_BONUS = 50,
    //   ROOK_ON_SEMI_OPEN_FILE_BONUS = 25,
    //   KNIGHT_OUTPOST_BONUS = 50,
    //   PASSED_PAWN_BONUS = 50,
    //   DOUBLED_PAWN_PENALTY = 50,
    //   ISOLATED_PAWN_PENALTY = 50,
    //   KING_SAFETY_PAWN_SHIELD_BONUS = 50,
    // };

    int evaluationBonus = 0;

    // Bitboard pieceBitboards[Piece::BLACK_KING + 1];
    // pieceBitboards[Piece::WHITE_PAWN] = whitePawnsBitBoard;
    // pieceBitboards[Piece::WHITE_KNIGHT] = whiteKnightsBitBoard;
    // pieceBitboards[Piece::WHITE_BISHOP] = whiteBishopsBitBoard;
    // pieceBitboards[Piece::WHITE_ROOK] = whiteRooksBitBoard;
    // pieceBitboards[Piece::WHITE_QUEEN] = whiteQueensBitBoard;
    // pieceBitboards[Piece::WHITE_KING] = Bitboard(1ULL << whiteKingIndex);

    // pieceBitboards[Piece::BLACK_PAWN] = blackPawnsBitBoard;
    // pieceBitboards[Piece::BLACK_KNIGHT] = blackKnightsBitBoard;
    // pieceBitboards[Piece::BLACK_BISHOP] = blackBishopsBitBoard;
    // pieceBitboards[Piece::BLACK_ROOK] = blackRooksBitBoard;
    // pieceBitboards[Piece::BLACK_QUEEN] = blackQueensBitBoard;
    // pieceBitboards[Piece::BLACK_KING] = Bitboard(1ULL << blackKingIndex);

    if (whiteBishopsBitboard.countBits() >= 2)
      evaluationBonus += BISHOP_PAIR_BONUS;
    if (blackBishopsBitboard.countBits() >= 2)
      evaluationBonus -= BISHOP_PAIR_BONUS;

    if (castlingRights & WHITE_KINGSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (castlingRights & BLACK_KINGSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;
    if (castlingRights & WHITE_QUEENSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (castlingRights & BLACK_QUEENSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;

    if (whiteHasCastled)
      evaluationBonus += CASTLED_KING_BONUS;
    if (blackHasCastled)
      evaluationBonus -= CASTLED_KING_BONUS;

    for (int i = 0; i < 64; i++)
    {
      if (board[i].isEmpty())
        continue;

      int file = i % 8;
      int rank = i / 8;

      if (board[i].piece == Piece::WHITE_ROOK)
      {
        Bitboard pawns = whitePawnsBitboard | blackPawnsBitboard;

        if (pawns.file(file).isEmpty())
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (blackPawnsBitboard.file(file).isEmpty())
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (board[i].piece == Piece::BLACK_ROOK)
      {
        Bitboard pawns = whitePawnsBitboard | blackPawnsBitboard;

        if (pawns.file(file).isEmpty())
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (whitePawnsBitboard.file(file).isEmpty())
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (board[i].piece == Piece::WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && blackPawnsBitboard.file(file - 1).isEmpty() && blackPawnsBitboard.file(file + 1).isEmpty())
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (board[i].piece == Piece::BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && whitePawnsBitboard.file(file - 1).isEmpty() && whitePawnsBitboard.file(file + 1).isEmpty())
          evaluationBonus -= KNIGHT_OUTPOST_BONUS;
        continue;
      }

      if (board[i].piece == Piece::WHITE_KING)
      {
      }

      if (rank == 0)
      {
        if (whitePawnsBitboard.file(file).countBits() > 1)
          evaluationBonus -= DOUBLED_PAWN_PENALTY;
        if (blackPawnsBitboard.file(file).countBits() > 1)
          evaluationBonus += DOUBLED_PAWN_PENALTY;

        if (whitePawnsBitboard.file(file))
        {
          if (blackPawnsBitboard.file(file - 1).isEmpty() && blackPawnsBitboard.file(file + 1).isEmpty())
            evaluationBonus += PASSED_PAWN_BONUS;
          if (whitePawnsBitboard.file(file - 1).isEmpty() && whitePawnsBitboard.file(file + 1).isEmpty())
            evaluationBonus -= ISOLATED_PAWN_PENALTY;
        }
        if (blackPawnsBitboard.file(file))
        {
          if (whitePawnsBitboard.file(file - 1).isEmpty() && whitePawnsBitboard.file(file + 1).isEmpty())
            evaluationBonus -= PASSED_PAWN_BONUS;
          if (blackPawnsBitboard.file(file - 1).isEmpty() && blackPawnsBitboard.file(file + 1).isEmpty())
            evaluationBonus += ISOLATED_PAWN_PENALTY;
        }
      }
    }

    return evaluationBonus;
  }

  int Board::negamax(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return getStaticEvaluation();

    Move *legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = countLegalMoves(legalMoves);

    if (legalMovesCount == 0)
    {
      if (isInCheck(sideToMove))
        return -1000000;
      else
        return 0;
    }

    int bestMoveEvaluation = -1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i]);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation > bestMoveEvaluation)
      {
        bestMoveEvaluation = evaluation;
      }

      if (evaluation > alpha)
      {
        alpha = evaluation;
      }

      if (alpha >= beta)
      {
        break;
      }
    }

    return bestMoveEvaluation;
  }

  Move Board::generateOneDeepMove()
  {
    Move *legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = countLegalMoves(legalMoves);

    if (legalMovesCount == 0)
      return Move(0, 0, 0, 0, 0, 0, 0);

    int bestMoveIndex = 0;
    int bestMoveEvaluation = 1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i]);

      int evaluation = getStaticEvaluation();

      unmakeMove(legalMoves[i]);

      if (evaluation < bestMoveEvaluation)
      {
        bestMoveEvaluation = evaluation;
        bestMoveIndex = i;
      }
    }

    return legalMoves[bestMoveIndex];
  }

  Move Board::generateBestMove(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return generateOneDeepMove();

    Move *legalMoves = getLegalMoves(sideToMove);

    // std::cout << legalMoves[0].toString() << std::endl;

    Move *sortedLegalMoves = heuristicSortMoves(legalMoves);
    // Move *sortedLegalMoves = legalMoves;

    int legalMovesCount = countLegalMoves(sortedLegalMoves);

    if (legalMovesCount == 0)
      return Move(0, 0, 0, 0, 0, 0, 0);

    int bestMoveIndex = 0;
    int bestMoveEvaluation = -1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(sortedLegalMoves[i]);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(sortedLegalMoves[i]);

      if (evaluation > bestMoveEvaluation)
      {
        bestMoveEvaluation = evaluation;
        bestMoveIndex = i;
      }

      if (evaluation > alpha)
      {
        alpha = evaluation;
      }

      if (alpha >= beta)
      {
        break;
      }
    }

    // std::cout << debug << std::endl;

    return sortedLegalMoves[bestMoveIndex];
  }

  Move *Board::heuristicSortMoves(Move *moves)
  {
    int legalMovesCount = countLegalMoves(moves);

    int evaluations[legalMovesCount];

    for (int i = 0; i < legalMovesCount; i++)
    {
      evaluations[i] = heuristicEvaluation(moves[i]);
    }

    for (int i = 0; i < legalMovesCount; i++)
    {
      for (int j = i + 1; j < legalMovesCount; j++)
      {
        if (evaluations[i] < evaluations[j])
        {
          Move temp = moves[i];
          moves[i] = moves[j];
          moves[j] = temp;
        }
      }
    }

    return moves;
  }

  int Board::heuristicEvaluation(Move move)
  {
    int evaluation = 0;

    if (move.flags & Move::CAPTURE)
    {
      evaluation += Piece::PIECE_VALUES[move.capturedPiece];
    }

    if (move.flags & Move::PROMOTION)
    {
      evaluation += Piece::PIECE_VALUES[move.promotionPiece] - Piece::PIECE_VALUES[move.piece];
    }

    if (move.flags & Move::KSIDE_CASTLE)
    {
      evaluation += 50;
    }

    if (move.flags & Move::QSIDE_CASTLE)
    {
      evaluation += 50;
    }

    if (move.flags & Move::EP_CAPTURE)
    {
      evaluation += 100;
    }

    return evaluation;
  }

  std::string Board::getStringRepresentation()
  {
    std::string boardString = "";

    for (int i = 0; i < 64; i++)
    {
      boardString += board[i].getPieceChar();
    }

    return boardString;
  }
}