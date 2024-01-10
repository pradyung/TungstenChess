#include "Headers/board.hpp"

namespace Chess
{
  Board::Board(std::string fen) : magicMoveGen(MagicMoveGen(movesLookup))
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

    castlingRights = 0;
    enPassantFile = -1;

    sideToMove = WHITE;

    int pieceIndex = 0;
    for (int i = 0; i < fenParts[0].length(); i++)
    {
      if (fen[i] == '/')
        continue;
      else if (isdigit(fen[i]))
        pieceIndex += fen[i] - '0';
      else
      {
        board[pieceIndex] = std::string("PNBRQK..pnbrqk").find(fen[i]) + WHITE_PAWN;

        updatePiece(pieceIndex, board[pieceIndex]);

        pieceIndex++;
      }
    }

    if (fenParts[0] != "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
      inOpeningBook = false;

    sideToMove = fenParts[1] == "w" ? WHITE : BLACK;

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

    if (fenParts[3] != "-")
    {
      enPassantFile = fenParts[3][0] - 'a';
    }

    zobristKey = getInitialZobristKey();

    positionHistory.push_back(zobristKey);
  }

  ZobristKey Board::getInitialZobristKey()
  {
    ZobristKey hash = 0;

    for (int i = 0; i < 64; i++)
    {
      if (board[i])
      {
        hash ^= zobrist.pieceKeys[i][board[i]];
      }
    }

    hash ^= zobrist.castlingKeys[castlingRights];
    hash ^= zobrist.enPassantKeys[enPassantFile];

    if (sideToMove == WHITE)
      hash ^= zobrist.sideKey;

    return hash;
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
    int promotionPiece = move.promotionPiece & TYPE;

    int pieceType = piece & TYPE;
    int pieceColor = piece & COLOR;

    int capturedPieceType = capturedPiece & TYPE;
    int capturedPieceColor = capturedPiece & COLOR;

    movePiece(from, to);

    enPassantFile = -1;

    if (pieceType == KING)
      removeCastlingRights(pieceColor, CASTLING);

    if (pieceType == ROOK && (from == A8 || from == H8 || from == A1 || from == H1))
      removeCastlingRights(pieceColor, from % 8 == 0 ? QUEENSIDE : KINGSIDE);
    if (capturedPieceType == ROOK && (to == A8 || to == H8 || to == A1 || to == H1))
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

    positionHistory.push_back(zobristKey);
  }

  void Board::unmakeMove(Move move)
  {
    positionHistory.pop_back();

    switchSideToMove();

    int from = move.from;
    int to = move.to;
    int piece = move.piece;
    int capturedPiece = move.capturedPiece;
    int promotionPiece = move.promotionPiece;

    unmovePiece(from, to, piece, capturedPiece);

    if (move.flags & KSIDE_CASTLE)
    {
      unmovePiece(to + 1, to - 1);
      hasCastled &= ~(piece & COLOR);
    }

    else if (move.flags & QSIDE_CASTLE)
    {
      unmovePiece(to - 2, to + 1);
      hasCastled &= ~(piece & COLOR);
    }

    updateEnPassantFile(move.enPassantFile);
    updateCastlingRights(move.castlingRights);

    kingIndices[piece] = from;

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

    int piece = board[pieceIndex];

    if (piece == WHITE_PAWN)
    {
      if (!board[pieceIndex - 8] && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex - 8);
        if (pieceIndex >= A2 && pieceIndex <= H2 && !board[pieceIndex - 16])
        {
          movesBitboard.addBit(pieceIndex - 16);
        }
      }

      if (pieceIndex % 8 != 0 && (board[pieceIndex - 9] & BLACK))
      {
        movesBitboard.addBit(pieceIndex - 9);
      }
      if (pieceIndex % 8 != 7 && (board[pieceIndex - 7] & BLACK))
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
    else if (piece == BLACK_PAWN)
    {
      if (!board[pieceIndex + 8] && !onlyCaptures)
      {
        movesBitboard.addBit(pieceIndex + 8);
        if (pieceIndex >= A7 && pieceIndex <= H7 && !board[pieceIndex + 16])
        {
          movesBitboard.addBit(pieceIndex + 16);
        }
      }

      if (pieceIndex % 8 != 0 && (board[pieceIndex + 7] & WHITE))
      {
        movesBitboard.addBit(pieceIndex + 7);
      }
      if (pieceIndex % 8 != 7 && (board[pieceIndex + 9] & WHITE))
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

  Bitboard Board::getKnightMoves(int pieceIndex, bool _, bool __)
  {
    return Bitboard(movesLookup.KNIGHT_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR).bitboard);
  }

  Bitboard Board::getBishopMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    BitboardInt maskedBlockers = movesLookup.BISHOP_MASKS[pieceIndex] & blockersBitboard.bitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.BISHOP_MAGICS[pieceIndex]) >> magicMoveGen.BISHOP_SHIFTS[pieceIndex];

    return Bitboard(magicMoveGen.BISHOP_LOOKUP_TABLES[pieceIndex][magicIndex]) & ~friendlyPiecesBitboard;
  }

  Bitboard Board::getRookMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    BitboardInt maskedBlockers = movesLookup.ROOK_MASKS[pieceIndex] & blockersBitboard.bitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.ROOK_MAGICS[pieceIndex]) >> magicMoveGen.ROOK_SHIFTS[pieceIndex];

    return Bitboard(magicMoveGen.ROOK_LOOKUP_TABLES[pieceIndex][magicIndex]) & ~friendlyPiecesBitboard;
  }

  Bitboard Board::getQueenMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    BitboardInt bishopMaskedBlockers = movesLookup.BISHOP_MASKS[pieceIndex] & blockersBitboard.bitboard;
    BitboardInt rookMaskedBlockers = movesLookup.ROOK_MASKS[pieceIndex] & blockersBitboard.bitboard;

    int bishopMagicIndex = (bishopMaskedBlockers * magicMoveGen.BISHOP_MAGICS[pieceIndex]) >> magicMoveGen.BISHOP_SHIFTS[pieceIndex];
    int rookMagicIndex = (rookMaskedBlockers * magicMoveGen.ROOK_MAGICS[pieceIndex]) >> magicMoveGen.ROOK_SHIFTS[pieceIndex];

    Bitboard bishopMoves = Bitboard(magicMoveGen.BISHOP_LOOKUP_TABLES[pieceIndex][bishopMagicIndex]);
    Bitboard rookMoves = Bitboard(magicMoveGen.ROOK_LOOKUP_TABLES[pieceIndex][rookMagicIndex]);

    return (bishopMoves | rookMoves) & ~friendlyPiecesBitboard;
  }

  Bitboard Board::getKingMoves(int pieceIndex, bool includeCastling, bool __)
  {
    Bitboard movesBitboard = Bitboard(movesLookup.KING_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR).bitboard);

    int piece = board[pieceIndex];

    if (includeCastling && castlingRights)
    {
      if (piece == WHITE_KING)
      {
        if (castlingRights & WHITE_KINGSIDE)
        {
          if (!board[F1] && !board[G1] && !isInCheck(WHITE) && !isAttacked(F1, BLACK))
          {
            movesBitboard.addBit(G1);
          }
        }
        if (castlingRights & WHITE_QUEENSIDE)
        {
          if (!board[D1] && !board[C1] && !board[B1] && !isInCheck(WHITE) && !isAttacked(D1, BLACK))
          {
            movesBitboard.addBit(C1);
          }
        }
      }
      else if (piece == BLACK_KING)
      {
        if (castlingRights & BLACK_KINGSIDE)
        {
          if (!board[F8] && !board[G8] && !isInCheck(BLACK) && !isAttacked(F8, WHITE))
          {
            movesBitboard.addBit(G8);
          }
        }
        if (castlingRights & BLACK_QUEENSIDE)
        {
          if (!board[D8] && !board[C8] && !board[B8] && !isInCheck(BLACK) && !isAttacked(D8, WHITE))
          {
            movesBitboard.addBit(B8);
          }
        }
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex, bool includeCastling)
  {
    Bitboard pseudoLegalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, includeCastling);

    Bitboard legalMovesBitboard = Bitboard();

    while (pseudoLegalMovesBitboard.bitboard)
    {
      int toIndex = __builtin_ctzll(pseudoLegalMovesBitboard.bitboard);

      pseudoLegalMovesBitboard.removeBit(toIndex);

      Move move = Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, QUEEN);

      makeMove(move, true);

      if (!isInCheck(board[toIndex] & COLOR))
      {
        legalMovesBitboard.addBit(toIndex);
      }

      unmakeMove(move);
    }

    return legalMovesBitboard;
  }

  std::vector<Move> Board::getLegalMoves(int color, bool includeCastling)
  {
    std::vector<Move> legalMoves;

    for (int i = 0; i < 64; i++)
    {
      if (!board[i] || !(board[i] & color))
        continue;

      Bitboard movesBitboard = getLegalPieceMovesBitboard(i, includeCastling);

      for (int j = 0; j < 64; j++)
      {
        if (movesBitboard.hasBit(j))
        {
          legalMoves.push_back(Move(i, j, board[i], board[j], castlingRights, enPassantFile));

          if (legalMoves.back().flags & PROMOTION)
          {
            legalMoves.back().promotionPiece = QUEEN;
            legalMoves.push_back(Move(i, j, board[i], board[j], castlingRights, enPassantFile, KNIGHT));
            legalMoves.push_back(Move(i, j, board[i], board[j], castlingRights, enPassantFile, BISHOP));
            legalMoves.push_back(Move(i, j, board[i], board[j], castlingRights, enPassantFile, ROOK));
          }
        }
      }
    }

    return legalMoves;
  }

  bool Board::isInCheck(int color)
  {
    return isAttacked(kingIndices[color | KING], color ^ COLOR);
  }

  bool Board::isAttacked(int square, int color)
  {
    int piece = board[square];

    if (!piece)
      board[square] = (color | PAWN) ^ COLOR;

    bool attacked = false;

    if (piece & WHITE && movesLookup.REVERSE_BLACK_PAWN_CAPTURE_MOVES[square] & bitboards[BLACK_PAWN].bitboard)
      attacked = true;

    else if (piece & BLACK && movesLookup.REVERSE_WHITE_PAWN_CAPTURE_MOVES[square] & bitboards[WHITE_PAWN].bitboard)
      attacked = true;

    else if (movesLookup.KNIGHT_MOVES[square] & bitboards[(color) | KNIGHT].bitboard)
      attacked = true;

    else if (movesLookup.KING_MOVES[square] & bitboards[(color) | KING].bitboard)
      attacked = true;

    else if (getBishopMoves(square) & (bitboards[(color) | BISHOP].bitboard | bitboards[(color) | QUEEN].bitboard))
      attacked = true;

    else if (getRookMoves(square) & (bitboards[(color) | ROOK].bitboard | bitboards[(color) | QUEEN].bitboard))
      attacked = true;

    board[square] = piece;
    return attacked;
  }

  int Board::getGameStatus(int color)
  {
    if (countRepetitions(zobristKey) >= 3)
      return STALEMATE;

    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(color);

    while (friendlyPiecesBitboard.bitboard)
    {
      int pieceIndex = __builtin_ctzll(friendlyPiecesBitboard.bitboard);

      friendlyPiecesBitboard.removeBit(pieceIndex);

      if (getLegalPieceMovesBitboard(pieceIndex).bitboard)
        return NO_MATE;
    }

    return isInCheck(color) ? LOSE : STALEMATE;
  }

  Move Board::generateMoveFromInt(int moveInt)
  {
    int from = moveInt & 0x3f ^ 0x38;
    int to = (moveInt >> 6) & 0x3f ^ 0x38;

    int piece = board[from];
    int capturedPiece = board[to];

    return Move(from, to, piece, capturedPiece, castlingRights, enPassantFile);
  }

  Move Board::generateMoveFromUCI(std::string uci)
  {
    int from = (uci[0] - 'a') + (8 - uci[1] + '0') * 8;
    int to = (uci[2] - 'a') + (8 - uci[3] + '0') * 8;

    int piece = board[from];
    int capturedPiece = board[to];

    int promotionPiece = EMPTY;

    if (uci.length() == 5)
    {
      switch (uci[4])
      {
      case 'q':
        promotionPiece = QUEEN;
        break;
      case 'r':
        promotionPiece = ROOK;
        break;
      case 'b':
        promotionPiece = BISHOP;
        break;
      case 'n':
        promotionPiece = KNIGHT;
        break;
      }
    }

    return Move(from, to, piece, capturedPiece, castlingRights, enPassantFile, promotionPiece);
  }

  Move Board::generateBotMove()
  {
    if (inOpeningBook)
    {
      int moveInt = openings.getNextMove();

      if (moveInt != -1)
        return generateMoveFromInt(moveInt);

      inOpeningBook = false;
    }

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

    return sideToMove == WHITE ? staticEvaluation : -staticEvaluation;
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
      switch (board[i])
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

      if (board[i] == WHITE_KING)
      {
        Bitboard enemyPieces = bitboards[BLACK_PAWN] | bitboards[BLACK_KNIGHT] | bitboards[BLACK_BISHOP] | bitboards[BLACK_ROOK] | bitboards[BLACK_QUEEN];
        Bitboard friendlyPieces = bitboards[WHITE_KNIGHT] | bitboards[WHITE_BISHOP] | bitboards[WHITE_ROOK] | bitboards[WHITE_QUEEN];

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation += pieceEvalTables.KING_EVAL_TABLE[i] * endgameScore;

        positionalEvaluation += pieceEvalTables.KING_ENDGAME_EVAL_TABLE[i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = abs(i % 8 - kingIndices[BLACK_KING] % 8) + abs(i / 8 - kingIndices[BLACK_KING] / 8);

          positionalEvaluation += pieceEvalTables.KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
        }
      }
      else if (board[i] == BLACK_KING)
      {
        Bitboard enemyPieces = bitboards[WHITE_PAWN] | bitboards[WHITE_KNIGHT] | bitboards[WHITE_BISHOP] | bitboards[WHITE_ROOK] | bitboards[WHITE_QUEEN];
        Bitboard friendlyPieces = bitboards[BLACK_KNIGHT] | bitboards[BLACK_BISHOP] | bitboards[BLACK_ROOK] | bitboards[BLACK_QUEEN];

        float endgameScore = enemyPieces.countBits() / 16.0;

        positionalEvaluation -= pieceEvalTables.KING_EVAL_TABLE[63 - i] * endgameScore;

        positionalEvaluation -= pieceEvalTables.KING_ENDGAME_EVAL_TABLE[63 - i] * (1 - endgameScore);

        if (friendlyPieces.countBits() <= 3 && friendlyPieces.countBits() >= 1)
        {
          int kingsDistance = abs(i % 8 - kingIndices[WHITE_KING] % 8) + abs(i / 8 - kingIndices[WHITE_KING] / 8);

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

      if (!board[i])
        continue;

      if (board[i] == WHITE_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (pawns.file(file).isEmpty())
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (bitboards[BLACK_PAWN].file(file).isEmpty())
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (board[i] == BLACK_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (pawns.file(file).isEmpty())
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (bitboards[WHITE_PAWN].file(file).isEmpty())
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (board[i] == WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && bitboards[BLACK_PAWN].file(file - 1).isEmpty() && bitboards[BLACK_PAWN].file(file + 1).isEmpty())
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (board[i] == BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && bitboards[WHITE_PAWN].file(file - 1).isEmpty() && bitboards[WHITE_PAWN].file(file + 1).isEmpty())
          evaluationBonus -= KNIGHT_OUTPOST_BONUS;
        continue;
      }
    }

    return evaluationBonus;
  }

  Move Board::generateOneDeepMove()
  {
    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
      return Move();

    int bestMoveIndex = 0;
    int bestMoveEvaluation = 1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i], true);

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

  int Board::negamax(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return quiesce(QUIESCE_DEPTH, alpha, beta);

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove);

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
      makeMove(legalMoves[i], true);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation > alpha)
        alpha = evaluation;

      if (alpha >= beta)
        return beta;
    }

    return alpha;
  }

  int Board::quiesce(int depth, int alpha, int beta)
  {
    int standPat = getStaticEvaluation();

    if (depth == 0)
      return standPat;

    if (standPat >= beta)
      return beta;

    if (alpha < standPat)
      alpha = standPat;

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove, false);

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

      makeMove(legalMoves[i], true);

      int evaluation = -quiesce(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation > alpha)
        alpha = evaluation;

      if (alpha >= beta)
        return beta;
    }

    return alpha;
  }

  Move Board::generateBestMove(int depth, int alpha, int beta)
  {
    if (depth == 0)
      return generateOneDeepMove();

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
      return Move();

    int bestMoveIndex = 0;
    int bestMoveEvaluation = -1000000;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i], true);

      int evaluation = -negamax(depth - 1, -beta, -alpha);

      unmakeMove(legalMoves[i]);

      if (evaluation > bestMoveEvaluation)
      {
        bestMoveEvaluation = evaluation;
        bestMoveIndex = i;
      }

      if (evaluation > alpha)
        alpha = evaluation;

      if (alpha >= beta)
        break;
    }

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
      evaluation += PIECE_VALUES[move.capturedPiece & TYPE];
    }

    if (move.flags & PROMOTION)
    {
      evaluation += PIECE_VALUES[move.promotionPiece & TYPE] - PIECE_VALUES[move.piece & TYPE];
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
