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

    bitboards[WHITE_PAWN] = Bitboard();
    bitboards[WHITE_KNIGHT] = Bitboard();
    bitboards[WHITE_BISHOP] = Bitboard();
    bitboards[WHITE_ROOK] = Bitboard();
    bitboards[WHITE_QUEEN] = Bitboard();

    bitboards[BLACK_PAWN] = Bitboard();
    bitboards[BLACK_KNIGHT] = Bitboard();
    bitboards[BLACK_BISHOP] = Bitboard();
    bitboards[BLACK_ROOK] = Bitboard();
    bitboards[BLACK_QUEEN] = Bitboard();

    enPassantFile = NO_EN_PASSANT;

    castlingRights = 0;

    sideToMove = WHITE;

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
          kingIndices[WHITE] = pieceIndex;
        else if (fen[i] == 'k')
          kingIndices[BLACK] = pieceIndex;

        if (fen[i] == 'P')
          bitboards[WHITE_PAWN].addBit(pieceIndex);
        else if (fen[i] == 'N')
          bitboards[WHITE_KNIGHT].addBit(pieceIndex);
        else if (fen[i] == 'B')
          bitboards[WHITE_BISHOP].addBit(pieceIndex);
        else if (fen[i] == 'R')
          bitboards[WHITE_ROOK].addBit(pieceIndex);
        else if (fen[i] == 'Q')
          bitboards[WHITE_QUEEN].addBit(pieceIndex);

        else if (fen[i] == 'p')
          bitboards[BLACK_PAWN].addBit(pieceIndex);
        else if (fen[i] == 'n')
          bitboards[BLACK_KNIGHT].addBit(pieceIndex);
        else if (fen[i] == 'b')
          bitboards[BLACK_BISHOP].addBit(pieceIndex);
        else if (fen[i] == 'r')
          bitboards[BLACK_ROOK].addBit(pieceIndex);
        else if (fen[i] == 'q')
          bitboards[BLACK_QUEEN].addBit(pieceIndex);

        pieceIndex++;
      }
    }

    sideToMove = fenParts[1] == "w" ? WHITE : BLACK;

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

    zobristKey = getInitialZobristKey();

    positionHistory[zobristKey]++;
  }

  ZobristKey Board::getInitialZobristKey()
  {
    ZobristKey hash = 0;

    for (int i = 0; i < 64; i++)
    {
      if (!board[i].isEmpty())
      {
        hash ^= zobrist.pieceKeys[i][board[i].piece];
      }
    }

    hash ^= zobrist.castlingKeys[castlingRights];
    hash ^= zobrist.enPassantKeys[enPassantFile];

    if (sideToMove == WHITE)
      hash ^= zobrist.sideKey;

    return hash;
  }

  void Board::updatePiece(int pieceIndex, int piece)
  {
    zobristKey ^= zobrist.pieceKeys[pieceIndex][board[pieceIndex].piece];
    zobristKey ^= zobrist.pieceKeys[pieceIndex][piece];

    if ((piece & 7) == KING)
      kingIndices[piece & 24] = pieceIndex;

    removePieceFromBitboard(pieceIndex);

    board[pieceIndex] = Piece(piece);

    addPieceToBitboard(pieceIndex);
  }

  void Board::makeMove(Move move, bool speculative)
  {
    switchSideToMove();

    if (!speculative && inOpeningBook)
      inOpeningBook = openings.addMove(move.toInt());

    int from = move.from;
    int to = move.to;
    int piece = move.piece;
    int capturedPiece = move.capturedPiece;
    int promotionPiece = move.promotionPiece & 7;

    int pieceType = piece & 7;
    int pieceColor = piece & 24;

    int capturedPieceType = capturedPiece & 7;
    int capturedPieceColor = capturedPiece & 24;

    movePiece(from, to);

    enPassantFile = NO_EN_PASSANT;

    if (pieceType == KING)
      removeCastlingRights(pieceColor, CASTLING);

    if (pieceType == ROOK && (from == 0 || from == 7 || from == 56 || from == 63))
      removeCastlingRights(pieceColor, from % 8 == 0 ? QUEENSIDE : KINGSIDE);
    if (capturedPieceType == ROOK && (to == 0 || to == 7 || to == 56 || to == 63))
      removeCastlingRights(capturedPieceColor, to % 8 == 0 ? QUEENSIDE : KINGSIDE);

    if (move.flags & EP_CAPTURE)
      updatePiece(to + (piece & WHITE ? 8 : -8), EMPTY);

    if (move.flags & PROMOTION)
      updatePiece(to, pieceColor | promotionPiece);

    if (move.flags & CASTLE)
    {
      hasCastled |= pieceColor;

      if (move.flags & KSIDE_CASTLE)
        movePiece(to + 1, to - 1);
      else
        movePiece(to - 2, to + 1);
    }

    if (move.flags & PAWN_DOUBLE)
      updateEnPassantFile(to % 8);

    positionHistory[zobristKey]++;
  }

  void Board::unmakeMove(Move move)
  {
    positionHistory[zobristKey]--;

    switchSideToMove();

    int from = move.from;
    int to = move.to;
    int piece = move.piece;
    int capturedPiece = move.capturedPiece;
    int promotionPiece = move.promotionPiece;

    updatePiece(to, capturedPiece);
    updatePiece(from, piece);

    if (move.flags & KSIDE_CASTLE)
    {
      updatePiece(to + 1, piece & 24 | ROOK);
      updatePiece(to - 1, EMPTY);

      hasCastled &= ~(piece & 24);
    }

    else if (move.flags & QSIDE_CASTLE)
    {
      updatePiece(to - 2, piece & 24 | ROOK);
      updatePiece(to + 1, EMPTY);

      hasCastled &= ~(piece & 24);
    }

    updateEnPassantFile(move.enPassantFile);
    updateCastlingRights(move.castlingRights);

    if (piece & 7 == KING)
      kingIndices[piece & 24] = from;

    if (move.flags & EP_CAPTURE)
    {
      if (piece & WHITE)
        updatePiece(to + 8, BLACK_PAWN);
      else
        updatePiece(to - 8, WHITE_PAWN);
    }
  }

  Bitboard Board::getPawnMoves(int pieceIndex, bool _, bool onlyCaptures)
  {
    Bitboard movesBitboard = Bitboard();

    int piece = board[pieceIndex].piece;

    if (piece == WHITE_PAWN)
    {
      if (board[pieceIndex - 8].isEmpty() && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex - 8);
        if (pieceIndex >= 48 && pieceIndex <= 55 && board[pieceIndex - 16].isEmpty())
        {
          movesBitboard.addBit(pieceIndex - 16);
        }
      }

      if (pieceIndex % 8 != 0 && ((!board[pieceIndex - 9].isEmpty() && board[pieceIndex - 9].getPieceColor() == BLACK) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex - 9);
      }
      if (pieceIndex % 8 != 7 && ((!board[pieceIndex - 7].isEmpty() && board[pieceIndex - 7].getPieceColor() == BLACK) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex - 7);
      }

      if (enPassantFile != NO_EN_PASSANT)
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
    else if (piece == BLACK_PAWN)
    {
      if (board[pieceIndex + 8].isEmpty() && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex + 8);
        if (pieceIndex >= 8 && pieceIndex <= 15 && board[pieceIndex + 16].isEmpty())
        {
          movesBitboard.addBit(pieceIndex + 16);
        }
      }

      if (pieceIndex % 8 != 0 && ((!board[pieceIndex + 7].isEmpty() && board[pieceIndex + 7].getPieceColor() == WHITE) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex + 7);
      }
      if (pieceIndex % 8 != 7 && ((!board[pieceIndex + 9].isEmpty() && board[pieceIndex + 9].getPieceColor() == WHITE) || onlyCaptures))
      {
        movesBitboard.addBit(pieceIndex + 9);
      }

      if (enPassantFile != NO_EN_PASSANT)
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

  Bitboard Board::getKnightMoves(int pieceIndex, bool _, bool __)
  {
    return Bitboard(movesLookup.KNIGHT_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex].getPieceColor()).bitboard);
  }

  Bitboard Board::getBishopMoves(int pieceIndex, bool _, bool __)
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

  Bitboard Board::getRookMoves(int pieceIndex, bool _, bool __)
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

  Bitboard Board::getQueenMoves(int pieceIndex, bool _, bool __)
  {
    return getBishopMoves(pieceIndex) | getRookMoves(pieceIndex);
  }

  Bitboard Board::getKingMoves(int pieceIndex, bool includeCastling, bool __)
  {
    Bitboard movesBitboard = Bitboard(movesLookup.KING_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex].getPieceColor()).bitboard);

    int piece = board[pieceIndex].piece;

    if (includeCastling && castlingRights)
    {
      if (piece == WHITE_KING)
      {
        if (castlingRights & WHITE_KINGSIDE)
        {
          if (board[61].isEmpty() && board[62].isEmpty() && !isInCheck(WHITE) && !isAttacked(61, BLACK))
          {
            movesBitboard.addBit(62);
          }
        }
        if (castlingRights & WHITE_QUEENSIDE)
        {
          if (board[59].isEmpty() && board[58].isEmpty() && board[57].isEmpty() && !isInCheck(WHITE) && !isAttacked(59, BLACK))
          {
            movesBitboard.addBit(58);
          }
        }
      }
      else if (piece == BLACK_KING)
      {
        if (castlingRights & BLACK_KINGSIDE)
        {
          if (board[5].isEmpty() && board[6].isEmpty() && !isInCheck(BLACK) && !isAttacked(5, WHITE))
          {
            movesBitboard.addBit(6);
          }
        }
        if (castlingRights & BLACK_QUEENSIDE)
        {
          if (board[3].isEmpty() && board[2].isEmpty() && board[1].isEmpty() && !isInCheck(BLACK) && !isAttacked(3, WHITE))
          {
            movesBitboard.addBit(2);
          }
        }
      }
    }

    return movesBitboard;
  }

  std::vector<Move> Board::getLegalMoves(int color, bool includeCastling)
  {
    std::vector<Move> legalMoves;

    for (int i = 0; i < 64; i++)
    {
      if (board[i].isEmpty() || board[i].getPieceColor() != color)
        continue;

      Bitboard movesBitboard = getLegalPieceMovesBitboard(i, includeCastling);

      for (int j = 0; j < 64; j++)
      {
        if (movesBitboard.hasBit(j))
        {
          legalMoves.push_back(Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights));

          if (legalMoves.back().flags & PROMOTION)
          {
            legalMoves.back().promotionPiece = QUEEN;
            legalMoves.push_back(Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, KNIGHT));
            legalMoves.push_back(Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, BISHOP));
            legalMoves.push_back(Move(i, j, board[i].piece, board[j].piece, enPassantFile, castlingRights, ROOK));
          }
        }
      }
    }

    return legalMoves;
  }

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex, bool includeCastling)
  {
    Bitboard legalMovesBitboard;

    legalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, includeCastling);

    for (int j = 0; j < 64; j++)
    {
      if (legalMovesBitboard.hasBit(j))
      {
        Move move = Move(pieceIndex, j, board[pieceIndex].piece, board[j].piece, enPassantFile, castlingRights, QUEEN);

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
    return isAttacked(kingIndices[color], color ^ 24);
  }

  bool Board::isAttacked(int square, int color)
  {
    int piece = board[square].piece;

    if (piece == EMPTY)
      board[square].piece = color | PAWN ^ 24;

    bool attacked = false;

    if (movesLookup.REVERSE_PAWN_CAPTURE_MOVES[square] & bitboards[(color) | PAWN].bitboard)
      attacked = true;

    else if (movesLookup.KNIGHT_MOVES[square] & bitboards[(color) | KNIGHT].bitboard)
      attacked = true;

    else if (movesLookup.KING_MOVES[square] & bitboards[(color) | KING].bitboard)
      attacked = true;

    else if (getBishopMoves(square) & (bitboards[(color) | BISHOP].bitboard | bitboards[(color) | QUEEN].bitboard))
      attacked = true;

    else if (getRookMoves(square) & (bitboards[(color) | ROOK].bitboard | bitboards[(color ^ 24) | QUEEN].bitboard))
      attacked = true;

    board[square].piece = piece;
    return attacked;
  }

  int Board::getGameStatus(int color)
  {
    if (getLegalMoves(color).size() == 0)
      return isInCheck(color) ? LOSE : STALEMATE;

    if (positionHistory[zobristKey] >= 3)
      return STALEMATE;

    return NO_MATE;
  }

  Move Board::generateMoveFromInt(int moveInt)
  {
    int from = moveInt & 0x3f ^ 0x38;
    int to = (moveInt >> 6) & 0x3f ^ 0x38;

    int piece = board[from].piece;
    int capturedPiece = board[to].piece;

    return Move(from, to, piece, capturedPiece, enPassantFile, castlingRights);
  }

  Move Board::generateBotMove()
  {
    if (inOpeningBook)
      return generateMoveFromInt(openings.getNextMove());

    return generateBestMove(SEARCH_DEPTH);
  }

  int Board::getStaticEvaluation()
  {
    int gameStatus = getGameStatus(sideToMove);

    if (gameStatus != NO_MATE)
    {
      if (gameStatus == LOSE)
        return sideToMove == WHITE ? 1000000 : -1000000;
      else
        return 0;
    }

    int staticEvaluation = getMaterialEvaluation() + getPositionalEvaluation() + getEvaluationBonus();

    if (sideToMove == BLACK)
      staticEvaluation = -staticEvaluation;

    return staticEvaluation;
  }

  int Board::getMaterialEvaluation()
  {

    int materialEvaluation = 0;

    materialEvaluation += bitboards[WHITE_PAWN].countBits() * PIECE_VALUES[PAWN];
    materialEvaluation += bitboards[WHITE_KNIGHT].countBits() * PIECE_VALUES[KNIGHT];
    materialEvaluation += bitboards[WHITE_BISHOP].countBits() * PIECE_VALUES[BISHOP];
    materialEvaluation += bitboards[WHITE_ROOK].countBits() * PIECE_VALUES[ROOK];
    materialEvaluation += bitboards[WHITE_QUEEN].countBits() * PIECE_VALUES[QUEEN];

    materialEvaluation -= bitboards[BLACK_PAWN].countBits() * PIECE_VALUES[PAWN];
    materialEvaluation -= bitboards[BLACK_KNIGHT].countBits() * PIECE_VALUES[KNIGHT];
    materialEvaluation -= bitboards[BLACK_BISHOP].countBits() * PIECE_VALUES[BISHOP];
    materialEvaluation -= bitboards[BLACK_ROOK].countBits() * PIECE_VALUES[ROOK];
    materialEvaluation -= bitboards[BLACK_QUEEN].countBits() * PIECE_VALUES[QUEEN];

    return materialEvaluation;
  }

  int Board::getPositionalEvaluation()
  {
    int positionalEvaluation = 0;

    for (int i = 0; i < 64; i++)
    {
      switch (board[i].piece)
      {
      case WHITE_PAWN:
        positionalEvaluation += pieceEvalTables.PAWN_EVAL_TABLE[i];
        break;
      case WHITE_KNIGHT:
        positionalEvaluation += pieceEvalTables.KNIGHT_EVAL_TABLE[i];
        break;
      case WHITE_BISHOP:
        positionalEvaluation += pieceEvalTables.BISHOP_EVAL_TABLE[i];
        break;
      case WHITE_ROOK:
        positionalEvaluation += pieceEvalTables.ROOK_EVAL_TABLE[i];
        break;
      case WHITE_QUEEN:
        positionalEvaluation += pieceEvalTables.QUEEN_EVAL_TABLE[i];
        break;
      case BLACK_PAWN:
        positionalEvaluation -= pieceEvalTables.PAWN_EVAL_TABLE[63 - i];
        break;
      case BLACK_KNIGHT:
        positionalEvaluation -= pieceEvalTables.KNIGHT_EVAL_TABLE[63 - i];
        break;
      case BLACK_BISHOP:
        positionalEvaluation -= pieceEvalTables.BISHOP_EVAL_TABLE[63 - i];
        break;
      case BLACK_ROOK:
        positionalEvaluation -= pieceEvalTables.ROOK_EVAL_TABLE[63 - i];
        break;
      case BLACK_QUEEN:
        positionalEvaluation -= pieceEvalTables.QUEEN_EVAL_TABLE[63 - i];
        break;
      }

      if (board[i].piece == WHITE_KING)
      {
        Bitboard enemyPieces = bitboards[BLACK_PAWN] | bitboards[BLACK_KNIGHT] | bitboards[BLACK_BISHOP] | bitboards[BLACK_ROOK] | bitboards[BLACK_QUEEN];
        Bitboard friendlyPieces = bitboards[WHITE_KNIGHT] | bitboards[WHITE_BISHOP] | bitboards[WHITE_ROOK] | bitboards[WHITE_QUEEN];

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation += pieceEvalTables.KING_EVAL_TABLE[i] * endgameScore;

        positionalEvaluation += pieceEvalTables.KING_ENDGAME_EVAL_TABLE[i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = 0;

          for (int j = 0; j < 64; j++)
          {
            if (board[j].piece == BLACK_KING)
            {
              kingsDistance = abs(i % 8 - j % 8) + abs(i / 8 - j / 8);
              break;
            }
          }

          positionalEvaluation += pieceEvalTables.KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
        }
      }
      else if (board[i].piece == BLACK_KING)
      {
        Bitboard enemyPieces = bitboards[WHITE_PAWN] | bitboards[WHITE_KNIGHT] | bitboards[WHITE_BISHOP] | bitboards[WHITE_ROOK] | bitboards[WHITE_QUEEN];
        Bitboard friendlyPieces = bitboards[BLACK_KNIGHT] | bitboards[BLACK_BISHOP] | bitboards[BLACK_ROOK] | bitboards[BLACK_QUEEN];

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation -= pieceEvalTables.KING_EVAL_TABLE[63 - i] * endgameScore;

        positionalEvaluation -= pieceEvalTables.KING_ENDGAME_EVAL_TABLE[63 - i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = 0;

          for (int j = 0; j < 64; j++)
          {
            if (board[j].piece == WHITE_KING)
            {
              kingsDistance = abs(i % 8 - j % 8) + abs(i / 8 - j / 8);
              break;
            }
          }

          positionalEvaluation -= pieceEvalTables.KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
        }
      }
    }

    return positionalEvaluation;
  }

  int Board::getEvaluationBonus()
  {
    int evaluationBonus = 0;

    if (bitboards[WHITE_BISHOP].countBits() >= 2)
      evaluationBonus += BISHOP_PAIR_BONUS;
    if (bitboards[BLACK_BISHOP].countBits() >= 2)
      evaluationBonus -= BISHOP_PAIR_BONUS;

    if (castlingRights & WHITE_KINGSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (castlingRights & BLACK_KINGSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;
    if (castlingRights & WHITE_QUEENSIDE)
      evaluationBonus += CAN_CASTLE_BONUS;
    if (castlingRights & BLACK_QUEENSIDE)
      evaluationBonus -= CAN_CASTLE_BONUS;

    if (hasCastled & WHITE)
      evaluationBonus += CASTLED_KING_BONUS;
    if (hasCastled & BLACK)
      evaluationBonus -= CASTLED_KING_BONUS;

    for (int i = 0; i < 64; i++)
    {
      int file = i % 8;
      int rank = i / 8;

      if (rank == 0)
      {
        if (bitboards[WHITE_PAWN].file(file).countBits() > 1)
          evaluationBonus -= DOUBLED_PAWN_PENALTY;
        if (bitboards[BLACK_PAWN].file(file).countBits() > 1)
          evaluationBonus += DOUBLED_PAWN_PENALTY;

        if (bitboards[WHITE_PAWN].file(file))
        {
          if (bitboards[BLACK_PAWN].file(file - 1).isEmpty() && bitboards[BLACK_PAWN].file(file + 1).isEmpty())
            evaluationBonus += PASSED_PAWN_BONUS;
          if (bitboards[WHITE_PAWN].file(file - 1).isEmpty() && bitboards[WHITE_PAWN].file(file + 1).isEmpty())
            evaluationBonus -= ISOLATED_PAWN_PENALTY;
        }
        if (bitboards[BLACK_PAWN].file(file))
        {
          if (bitboards[WHITE_PAWN].file(file - 1).isEmpty() && bitboards[WHITE_PAWN].file(file + 1).isEmpty())
            evaluationBonus -= PASSED_PAWN_BONUS;
          if (bitboards[BLACK_PAWN].file(file - 1).isEmpty() && bitboards[BLACK_PAWN].file(file + 1).isEmpty())
            evaluationBonus += ISOLATED_PAWN_PENALTY;
        }
      }

      if (board[i].isEmpty())
        continue;

      if (board[i].piece == WHITE_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (pawns.file(file).isEmpty())
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (bitboards[BLACK_PAWN].file(file).isEmpty())
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (board[i].piece == BLACK_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (pawns.file(file).isEmpty())
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (bitboards[WHITE_PAWN].file(file).isEmpty())
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (board[i].piece == WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && bitboards[BLACK_PAWN].file(file - 1).isEmpty() && bitboards[BLACK_PAWN].file(file + 1).isEmpty())
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (board[i].piece == BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && bitboards[WHITE_PAWN].file(file - 1).isEmpty() && bitboards[WHITE_PAWN].file(file + 1).isEmpty())
          evaluationBonus -= KNIGHT_OUTPOST_BONUS;
        continue;
      }
    }

    return evaluationBonus;
  }

  int Board::negamax(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return quiesce(alpha, beta);

    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    legalMoves = heuristicSortMoves(legalMoves);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
    {
      if (isInCheck(sideToMove))
        return -1000000;
      else
        return 0;
    }

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i]);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation > alpha)
      {
        alpha = evaluation;
      }

      if (alpha >= beta)
      {
        return beta;
      }
    }

    return alpha;
  }

  int Board::quiesce(int alpha, int beta)
  {
    int standPat = getStaticEvaluation();

    if (standPat >= beta)
      return beta;

    if (alpha < standPat)
      alpha = standPat;

    std::vector<Move> legalMoves = getLegalMoves(sideToMove, false);

    legalMoves = heuristicSortMoves(legalMoves);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
    {
      if (isInCheck(sideToMove))
        return -1000000;
      else
        return 0;
    }

    for (int i = 0; i < legalMovesCount; i++)
    {
      if (!(legalMoves[i].flags & CAPTURE))
        continue;

      makeMove(legalMoves[i]);

      int evaluation = -quiesce(-beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation >= beta)
        return beta;

      if (evaluation > alpha)
        alpha = evaluation;
    }

    return alpha;
  }

  Move Board::generateOneDeepMove()
  {
    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

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

    legalMoves[bestMoveIndex].computedEvaluation = bestMoveEvaluation;

    return legalMoves[bestMoveIndex];
  }

  Move Board::generateBestMove(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return generateOneDeepMove();

    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    legalMoves = heuristicSortMoves(legalMoves);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
      return Move(0, 0, 0, 0, 0, 0, 0);

    int bestMoveIndex = 0;
    int bestMoveEvaluation = -1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i]);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

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

    legalMoves[bestMoveIndex].computedEvaluation = bestMoveEvaluation;

    return legalMoves[bestMoveIndex];
  }

  std::vector<Move> Board::heuristicSortMoves(std::vector<Move> moves)
  {
    int legalMovesCount = moves.size();

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

    if (move.flags & CAPTURE)
    {
      evaluation += PIECE_VALUES[move.capturedPiece & 7];
    }

    if (move.flags & PROMOTION)
    {
      evaluation += PIECE_VALUES[move.promotionPiece & 7] - PIECE_VALUES[move.piece & 7];
    }

    if (move.flags & KSIDE_CASTLE)
    {
      evaluation += 50;
    }

    if (move.flags & QSIDE_CASTLE)
    {
      evaluation += 50;
    }

    if (move.flags & EP_CAPTURE)
    {
      evaluation += 100;
    }

    return evaluation;
  }
}
