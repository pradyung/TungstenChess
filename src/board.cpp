#include "Headers/board.hpp" // See for documentation and helper function implementations

namespace Chess
{
  Board::Board(std::string fen, BotSettings botSettings) : botSettings(botSettings)
  {
    std::string fenParts[FEN_LENGTH];

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
    enPassantFile = NO_EP;

    sideToMove = WHITE;

    int pieceIndex = 0;
    for (int i = 0; i < fenParts[BOARD].length(); i++)
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

    if (fenParts[BOARD] != "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
      openings.inOpeningBook = false;

    sideToMove = fenParts[SIDE_TO_MOVE] == "w" ? WHITE : BLACK;

    if (fenParts[CASTLING_RIGHTS] != "-")
    {
      for (int i = 0; i < fenParts[CASTLING_RIGHTS].length(); i++)
      {
        if (fenParts[CASTLING_RIGHTS][i] == 'K')
          castlingRights |= WHITE_KINGSIDE;
        else if (fenParts[CASTLING_RIGHTS][i] == 'Q')
          castlingRights |= WHITE_QUEENSIDE;
        else if (fenParts[CASTLING_RIGHTS][i] == 'k')
          castlingRights |= BLACK_KINGSIDE;
        else if (fenParts[CASTLING_RIGHTS][i] == 'q')
          castlingRights |= BLACK_QUEENSIDE;
      }
    }

    if (fenParts[EN_PASSANT] != "-")
    {
      enPassantFile = fenParts[EN_PASSANT][0] - 'a';
    }

    calculateInitialZobristKey();

    positionHistory.push_back(zobristKey);
  }

  void Board::calculateInitialZobristKey()
  {
    for (int i = 0; i < 64; i++)
    {
      if (board[i])
      {
        zobristKey ^= zobrist.pieceKeys[i][board[i]];
      }
    }

    zobristKey ^= zobrist.castlingKeys[castlingRights];
    zobristKey ^= zobrist.enPassantKeys[enPassantFile];

    if (sideToMove == WHITE)
      zobristKey ^= zobrist.sideKey;
  }

  void Board::makeMove(Move move, bool speculative)
  {
    switchSideToMove();

    if (!speculative && openings.inOpeningBook)
      openings.inOpeningBook = openings.addMove(move.toInt());

    auto [from, to, piece, capturedPiece, promotionPieceType, castlingRights, enPassantFile, flags] = move;

    int pieceType = piece & TYPE, pieceColor = piece & COLOR;
    int capturedPieceType = capturedPiece & TYPE, capturedPieceColor = capturedPiece & COLOR;

    movePiece(from, to, (flags & PROMOTION) ? (promotionPieceType | pieceColor) : EMPTY);

    updateEnPassantFile(flags & PAWN_DOUBLE ? to % 8 : NO_EP);

    if (pieceType == KING)
      removeCastlingRights(pieceColor, BOTHSIDES);

    if (pieceType == ROOK && (from == A8 || from == H8 || from == A1 || from == H1))
      removeCastlingRights(pieceColor, from % 8 == 0 ? QUEENSIDE : KINGSIDE);
    if (capturedPieceType == ROOK && (to == A8 || to == H8 || to == A1 || to == H1))
      removeCastlingRights(capturedPieceColor, to % 8 == 0 ? QUEENSIDE : KINGSIDE);

    if (flags & EP_CAPTURE)
      updatePiece(to + (piece & WHITE ? 8 : -8), EMPTY);

    if (flags & CASTLE)
    {
      hasCastled |= pieceColor;

      if (flags & KSIDE_CASTLE)
        movePiece(to + 1, to - 1);
      else
        movePiece(to - 2, to + 1);
    }

    positionHistory.push_back(zobristKey);
  }

  void Board::unmakeMove(Move move)
  {
    positionHistory.pop_back();

    switchSideToMove();

    unmovePiece(move.from, move.to, move.piece, move.capturedPiece);

    if (move.flags & CASTLE)
    {
      hasCastled &= ~(move.piece & COLOR);

      if (move.flags & KSIDE_CASTLE)
        unmovePiece(move.to + 1, move.to - 1);
      else
        unmovePiece(move.to - 2, move.to + 1);
    }

    updateEnPassantFile(move.enPassantFile);
    updateCastlingRights(move.castlingRights);

    if (move.flags & EP_CAPTURE)
      updatePiece((move.piece & WHITE) ? move.to + 8 : move.to - 8, move.piece ^ COLOR);
  }

  Bitboard Board::getPawnMoves(int pieceIndex, bool _, bool onlyCaptures)
  {
    Bitboard movesBitboard = Bitboard();

    int piece = board[pieceIndex];

    if (piece == WHITE_PAWN)
    {
      if (!board[pieceIndex - 8] && !onlyCaptures)
      {
        Bitboards::addBit(movesBitboard, pieceIndex - 8);
        if (pieceIndex >= A2 && pieceIndex <= H2 && !board[pieceIndex - 16])
          Bitboards::addBit(movesBitboard, pieceIndex - 16);
      }

      movesBitboard |= (movesLookup.WHITE_PAWN_CAPTURE_MOVES[pieceIndex] & (getEnemyPiecesBitboard(WHITE) | (enPassantFile == NO_EP ? 0 : 1ULL << (enPassantFile + 16))));
    }
    else if (piece == BLACK_PAWN)
    {
      if (!board[pieceIndex + 8] && !onlyCaptures)
      {
        Bitboards::addBit(movesBitboard, pieceIndex + 8);
        if (pieceIndex >= A7 && pieceIndex <= H7 && !board[pieceIndex + 16])
          Bitboards::addBit(movesBitboard, pieceIndex + 16);
      }

      movesBitboard |= (movesLookup.BLACK_PAWN_CAPTURE_MOVES[pieceIndex] & (getEnemyPiecesBitboard(BLACK) | (enPassantFile == NO_EP ? 0 : 1ULL << (enPassantFile + 40))));
    }

    return movesBitboard;
  }

  Bitboard Board::getKnightMoves(int pieceIndex, bool _, bool __)
  {
    return Bitboard(movesLookup.KNIGHT_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR));
  }

  Bitboard Board::getBishopMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    Bitboard maskedBlockers = movesLookup.BISHOP_MASKS[pieceIndex] & blockersBitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.BISHOP_MAGICS[pieceIndex]) >> magicMoveGen.BISHOP_SHIFTS[pieceIndex];

    return Bitboard(magicMoveGen.BISHOP_LOOKUP_TABLES[pieceIndex][magicIndex] & ~friendlyPiecesBitboard);
  }

  Bitboard Board::getRookMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    Bitboard maskedBlockers = movesLookup.ROOK_MASKS[pieceIndex] & blockersBitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.ROOK_MAGICS[pieceIndex]) >> magicMoveGen.ROOK_SHIFTS[pieceIndex];

    return Bitboard(magicMoveGen.ROOK_LOOKUP_TABLES[pieceIndex][magicIndex] & ~friendlyPiecesBitboard);
  }

  Bitboard Board::getQueenMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    Bitboard bishopMaskedBlockers = movesLookup.BISHOP_MASKS[pieceIndex] & blockersBitboard;
    Bitboard rookMaskedBlockers = movesLookup.ROOK_MASKS[pieceIndex] & blockersBitboard;

    int bishopMagicIndex = (bishopMaskedBlockers * magicMoveGen.BISHOP_MAGICS[pieceIndex]) >> magicMoveGen.BISHOP_SHIFTS[pieceIndex];
    int rookMagicIndex = (rookMaskedBlockers * magicMoveGen.ROOK_MAGICS[pieceIndex]) >> magicMoveGen.ROOK_SHIFTS[pieceIndex];

    Bitboard bishopMoves = magicMoveGen.BISHOP_LOOKUP_TABLES[pieceIndex][bishopMagicIndex];
    Bitboard rookMoves = magicMoveGen.ROOK_LOOKUP_TABLES[pieceIndex][rookMagicIndex];

    return Bitboard((bishopMoves | rookMoves) & ~friendlyPiecesBitboard);
  }

  Bitboard Board::getKingMoves(int pieceIndex, bool includeCastling, bool __)
  {
    Bitboard movesBitboard = Bitboard(movesLookup.KING_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR));

    int piece = board[pieceIndex];

    if (includeCastling && castlingRights)
    {
      if (piece == WHITE_KING)
      {
        if (castlingRights & WHITE_KINGSIDE && !board[F1] && !board[G1] && !isInCheck(WHITE) && !isAttacked(F1, BLACK))
          Bitboards::addBit(movesBitboard, G1);

        if (castlingRights & WHITE_QUEENSIDE && !board[D1] && !board[C1] && !board[B1] && !isInCheck(WHITE) && !isAttacked(D1, BLACK))
          Bitboards::addBit(movesBitboard, C1);
      }
      else if (piece == BLACK_KING)
      {
        if (castlingRights & BLACK_KINGSIDE && !board[F8] && !board[G8] && !isInCheck(BLACK) && !isAttacked(F8, WHITE))
          Bitboards::addBit(movesBitboard, G8);

        if (castlingRights & BLACK_QUEENSIDE && !board[D8] && !board[C8] && !board[B8] && !isInCheck(BLACK) && !isAttacked(D8, WHITE))
          Bitboards::addBit(movesBitboard, C8);
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex, bool includeCastling)
  {
    Bitboard pseudoLegalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, includeCastling);

    Bitboard legalMovesBitboard = Bitboard();

    while (pseudoLegalMovesBitboard)
    {
      int toIndex = __builtin_ctzll(pseudoLegalMovesBitboard);

      Bitboards::removeBit(pseudoLegalMovesBitboard, toIndex);

      Move move = Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, QUEEN);

      makeMove(move, true);

      if (!isInCheck(board[toIndex] & COLOR))
        Bitboards::addBit(legalMovesBitboard, toIndex);

      unmakeMove(move);
    }

    return legalMovesBitboard;
  }

  std::vector<Move> Board::getLegalMoves(int color, bool includeCastling)
  {
    std::vector<Move> legalMoves;
    legalMoves.reserve(256);

    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(color);

    while (friendlyPiecesBitboard)
    {
      int pieceIndex = __builtin_ctzll(friendlyPiecesBitboard);

      friendlyPiecesBitboard &= ~(1ULL << pieceIndex);

      Bitboard movesBitboard = getLegalPieceMovesBitboard(pieceIndex, includeCastling);

      while (movesBitboard)
      {
        int toIndex = __builtin_ctzll(movesBitboard);

        Bitboards::removeBit(movesBitboard, toIndex);

        legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, EMPTY));

        if (legalMoves.back().flags & PROMOTION)
        {
          legalMoves.back().promotionPieceType = QUEEN;
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, KNIGHT));
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, BISHOP));
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, ROOK));
        }
      }
    }

    return legalMoves;
  }

  bool Board::isAttacked(int square, int color)
  {
    int piece = board[square];

    if (!piece)
      board[square] = (color | PAWN) ^ COLOR;

    bool attacked = false;

    if (piece & WHITE && movesLookup.WHITE_PAWN_CAPTURE_MOVES[square] & bitboards[BLACK_PAWN])
      attacked = true;

    else if (piece & BLACK && movesLookup.BLACK_PAWN_CAPTURE_MOVES[square] & bitboards[WHITE_PAWN])
      attacked = true;

    else if (movesLookup.KNIGHT_MOVES[square] & bitboards[(color) | KNIGHT])
      attacked = true;

    else if (movesLookup.KING_MOVES[square] & bitboards[(color) | KING])
      attacked = true;

    else if (getBishopMoves(square) & (bitboards[(color) | BISHOP] | bitboards[(color) | QUEEN]))
      attacked = true;

    else if (getRookMoves(square) & (bitboards[(color) | ROOK] | bitboards[(color) | QUEEN]))
      attacked = true;

    board[square] = piece;
    return attacked;
  }

  int Board::getGameStatus(int color)
  {
    if (countRepetitions(zobristKey) >= 3)
      return STALEMATE;

    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(color);

    while (friendlyPiecesBitboard)
    {
      int pieceIndex = __builtin_ctzll(friendlyPiecesBitboard);

      Bitboards::removeBit(friendlyPiecesBitboard, pieceIndex);

      if (getLegalPieceMovesBitboard(pieceIndex))
        return NO_MATE;
    }

    return isInCheck(color) ? LOSE : STALEMATE;
  }

  Move Board::generateMoveFromInt(MoveInt moveInt)
  {
    int from = moveInt & 0x3f;
    int to = (moveInt >> 6) & 0x3f;

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

    int promotionPieceType = EMPTY;

    if (uci.length() == 5)
    {
      switch (uci[4])
      {
      case 'q':
        promotionPieceType = QUEEN;
        break;
      case 'r':
        promotionPieceType = ROOK;
        break;
      case 'b':
        promotionPieceType = BISHOP;
        break;
      case 'n':
        promotionPieceType = KNIGHT;
        break;
      }
    }

    return Move(from, to, piece, capturedPiece, castlingRights, enPassantFile, promotionPieceType);
  }

  std::string Board::getMovePGN(Move move)
  {
    std::string pgn = "";

    if (move.flags & CASTLE)
    {
      if (move.flags & KSIDE_CASTLE)
        pgn += "O-O";
      else
        pgn += "O-O-O";
    }
    else
    {
      int pieceType = move.piece & TYPE;

      if (pieceType != PAWN)
      {
        pgn += "..NBRQK"[pieceType];

        Bitboard sameTypePieces = bitboards[move.piece] & ~Bitboard(1ULL << move.from);
        Bitboard ambiguousPieces = Bitboard();

        while (sameTypePieces)
        {
          int pieceIndex = __builtin_ctzll(sameTypePieces);

          Bitboards::removeBit(sameTypePieces, pieceIndex);

          if (Bitboards::hasBit(getLegalPieceMovesBitboard(pieceIndex), move.to))
            Bitboards::addBit(ambiguousPieces, pieceIndex);
        }

        if (ambiguousPieces)
        {
          if (Bitboards::file(ambiguousPieces, move.from % 8))
          {
            if (Bitboards::rank(ambiguousPieces, move.from / 8))
              pgn += 'a' + (move.from % 8);
            pgn += '8' - (move.from / 8);
          }
          else
            pgn += 'a' + (move.from % 8);
        }
      }

      if (move.flags & (CAPTURE | EP_CAPTURE))
      {
        if (pieceType == PAWN)
        {
          pgn += 'a' + (move.from % 8);
        }
        pgn += 'x';
      }

      pgn += 'a' + (move.to % 8);
      pgn += '8' - (move.to / 8);

      if (move.flags & EP_CAPTURE)
        pgn += " e.p.";
      else if (move.flags & PROMOTION)
      {
        pgn += "=";
        pgn += ".NBRQ"[move.promotionPieceType];
      }
    }

    makeMove(move, true);

    int gameStatus = getGameStatus(sideToMove);

    if (isInCheck(sideToMove))
    {
      if (gameStatus == LOSE)
        pgn += (sideToMove == WHITE ? "#" : "#+");
      else
        pgn += "+";
    }

    unmakeMove(move);

    return pgn;
  }

  Move Board::generateBotMove()
  {
    if (openings.inOpeningBook && botSettings.useOpeningBook)
    {
      MoveInt moveInt = openings.getNextMove();

      if (moveInt != INVALID_MOVE)
      {
        Move bestMove = generateMoveFromInt(moveInt);

        if (botSettings.logSearchInfo)
          std::cout << "Book move: " << (botSettings.logPGNMoves ? getMovePGN(bestMove) : bestMove.getUCI()) << std::endl;

        return bestMove;
      }

      openings.inOpeningBook = false;
    }

    positionsEvaluated = 0;

    auto start = std::chrono::high_resolution_clock::now();

    Move bestMove = botSettings.fixedDepthSearch ? generateBestMove(botSettings.maxSearchDepth) : iterativeDeepening(botSettings.maxSearchTime, start);

    if (botSettings.logSearchInfo)
      std::cout << "Move: " << (botSettings.logPGNMoves ? getMovePGN(bestMove) : bestMove.getUCI()) << ", "
                << "Depth: " << depthSearched << ", "
                << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << " ms, "
                << "Positions evaluated: " << positionsEvaluated << std::endl;

    return bestMove;
  }

  int Board::getStaticEvaluation()
  {
    positionsEvaluated++;

    int gameStatus = getGameStatus(sideToMove);

    if (gameStatus != NO_MATE)
    {
      if (gameStatus == LOSE)
        return NEGATIVE_INFINITY;
      else
        return -STALEMATE_PENALTY;
    }

    int staticEvaluation = getMaterialEvaluation() + getPositionalEvaluation() + getEvaluationBonus();

    return sideToMove == WHITE ? staticEvaluation : -staticEvaluation;
  }

  int Board::getMaterialEvaluation()
  {
    int materialEvaluation = 0;

    materialEvaluation += Bitboards::countBits(bitboards[WHITE_PAWN]) * PIECE_VALUES[PAWN];
    materialEvaluation += Bitboards::countBits(bitboards[WHITE_KNIGHT]) * PIECE_VALUES[KNIGHT];
    materialEvaluation += Bitboards::countBits(bitboards[WHITE_BISHOP]) * PIECE_VALUES[BISHOP];
    materialEvaluation += Bitboards::countBits(bitboards[WHITE_ROOK]) * PIECE_VALUES[ROOK];
    materialEvaluation += Bitboards::countBits(bitboards[WHITE_QUEEN]) * PIECE_VALUES[QUEEN];

    materialEvaluation -= Bitboards::countBits(bitboards[BLACK_PAWN]) * PIECE_VALUES[PAWN];
    materialEvaluation -= Bitboards::countBits(bitboards[BLACK_KNIGHT]) * PIECE_VALUES[KNIGHT];
    materialEvaluation -= Bitboards::countBits(bitboards[BLACK_BISHOP]) * PIECE_VALUES[BISHOP];
    materialEvaluation -= Bitboards::countBits(bitboards[BLACK_ROOK]) * PIECE_VALUES[ROOK];
    materialEvaluation -= Bitboards::countBits(bitboards[BLACK_QUEEN]) * PIECE_VALUES[QUEEN];

    return materialEvaluation;
  }

  int Board::getPositionalEvaluation()
  {
    int positionalEvaluation = 0;

    Bitboard whitePieces = bitboards[WHITE_KNIGHT] | bitboards[WHITE_BISHOP] | bitboards[WHITE_ROOK] | bitboards[WHITE_QUEEN];
    Bitboard blackPieces = bitboards[BLACK_KNIGHT] | bitboards[BLACK_BISHOP] | bitboards[BLACK_ROOK] | bitboards[BLACK_QUEEN];

    Bitboard allPieces = whitePieces | bitboards[WHITE_PAWN] | blackPieces | bitboards[BLACK_PAWN];

    while (allPieces)
    {
      int pieceIndex = __builtin_ctzll(allPieces);

      Bitboards::removeBit(allPieces, pieceIndex);

      positionalEvaluation += getPiecePositionalEvaluation(pieceIndex);
    }

    {
      int whiteKingIndex = kingIndices[WHITE_KING];

      Bitboard enemyPieces = blackPieces | bitboards[BLACK_PAWN];
      Bitboard friendlyPieces = whitePieces;

      float endgameScore = Bitboards::countBits(enemyPieces) / 16.0;

      positionalEvaluation += KING_EVAL_TABLE[whiteKingIndex] * endgameScore;

      positionalEvaluation += KING_ENDGAME_EVAL_TABLE[whiteKingIndex] * (1 - endgameScore);

      if (Bitboards::countBits(friendlyPieces) <= 3 && Bitboards::countBits(friendlyPieces) >= 1)
      {
        int kingsDistance = abs(whiteKingIndex % 8 - kingIndices[BLACK_KING] % 8) + abs(whiteKingIndex / 8 - kingIndices[BLACK_KING] / 8);

        positionalEvaluation += KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
      }
    }

    {
      int blackKingIndex = kingIndices[BLACK_KING];

      Bitboard enemyPieces = whitePieces | bitboards[WHITE_PAWN];
      Bitboard friendlyPieces = blackPieces;

      float endgameScore = Bitboards::countBits(enemyPieces) / 16.0;

      positionalEvaluation -= KING_EVAL_TABLE[63 - blackKingIndex] * endgameScore;

      positionalEvaluation -= KING_ENDGAME_EVAL_TABLE[63 - blackKingIndex] * (1 - endgameScore);

      if (Bitboards::countBits(friendlyPieces) <= 3 && Bitboards::countBits(friendlyPieces) >= 1)
      {
        int kingsDistance = abs(blackKingIndex % 8 - kingIndices[WHITE_KING] % 8) + abs(blackKingIndex / 8 - kingIndices[WHITE_KING] / 8);

        positionalEvaluation -= KINGS_DISTANCE_EVAL_TABLE[kingsDistance];
      }
    }

    return positionalEvaluation;
  }

  int Board::getEvaluationBonus()
  {
    int evaluationBonus = 0;

    if (Bitboards::countBits(bitboards[WHITE_BISHOP]) >= 2)
      evaluationBonus += BISHOP_PAIR_BONUS;
    if (Bitboards::countBits(bitboards[BLACK_BISHOP]) >= 2)
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
        if (Bitboards::countBits(Bitboards::file(bitboards[WHITE_PAWN], file)) > 1)
          evaluationBonus -= DOUBLED_PAWN_PENALTY;
        if (Bitboards::countBits(Bitboards::file(bitboards[BLACK_PAWN], file)) > 1)
          evaluationBonus += DOUBLED_PAWN_PENALTY;

        if (Bitboards::file(bitboards[WHITE_PAWN], file))
        {
          if (!Bitboards::file(bitboards[BLACK_PAWN], file - 1) && !Bitboards::file(bitboards[BLACK_PAWN], file + 1))
            evaluationBonus += PASSED_PAWN_BONUS;
          if (!Bitboards::file(bitboards[WHITE_PAWN], file - 1) && !Bitboards::file(bitboards[WHITE_PAWN], file + 1))
            evaluationBonus -= ISOLATED_PAWN_PENALTY;
        }
        if (Bitboards::file(bitboards[BLACK_PAWN], file))
        {
          if (!Bitboards::file(bitboards[WHITE_PAWN], file - 1) && !Bitboards::file(bitboards[WHITE_PAWN], file + 1))
            evaluationBonus -= PASSED_PAWN_BONUS;
          if (!Bitboards::file(bitboards[BLACK_PAWN], file - 1) && !Bitboards::file(bitboards[BLACK_PAWN], file + 1))
            evaluationBonus += ISOLATED_PAWN_PENALTY;
        }
      }

      if (!board[i])
        continue;

      if (board[i] == WHITE_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (!Bitboards::file(pawns, file))
          evaluationBonus += ROOK_ON_OPEN_FILE_BONUS;
        else if (!Bitboards::file(bitboards[BLACK_PAWN], file))
          evaluationBonus += ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }
      if (board[i] == BLACK_ROOK)
      {
        Bitboard pawns = bitboards[WHITE_PAWN] | bitboards[BLACK_PAWN];

        if (!Bitboards::file(pawns, file))
          evaluationBonus -= ROOK_ON_OPEN_FILE_BONUS;
        else if (!Bitboards::file(bitboards[WHITE_PAWN], file))
          evaluationBonus -= ROOK_ON_SEMI_OPEN_FILE_BONUS;

        continue;
      }

      if (board[i] == WHITE_KNIGHT)
      {
        if (file > 0 && file < 7 && !Bitboards::file(bitboards[BLACK_PAWN], file - 1) && !Bitboards::file(bitboards[BLACK_PAWN], file + 1))
          evaluationBonus += KNIGHT_OUTPOST_BONUS;
        continue;
      }
      if (board[i] == BLACK_KNIGHT)
      {
        if (file > 0 && file < 7 && !Bitboards::file(bitboards[WHITE_PAWN], file - 1) && !Bitboards::file(bitboards[WHITE_PAWN], file + 1))
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

    int bestMoveIndex = 0;
    int bestMoveEvaluation = POSITIVE_INFINITY;

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
      return quiesce(botSettings.quiesceDepth, alpha, beta);

    if (countRepetitions(zobristKey) >= 3)
      return -STALEMATE_PENALTY;

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
      return isInCheck(sideToMove) ? NEGATIVE_INFINITY : -STALEMATE_PENALTY;

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

    if (standPat > alpha)
      alpha = standPat;

    if (alpha >= beta)
      return beta;

    if (countRepetitions(zobristKey) >= 3)
      return -STALEMATE_PENALTY;

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove, false);

    int legalMovesCount = legalMoves.size();

    if (legalMovesCount == 0)
      return standPat;

    for (int i = 0; i < legalMovesCount; i++)
    {
      if (!(legalMoves[i].flags & ~PAWN_DOUBLE))
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
    depthSearched = depth;

    if (depth == 0)
      return generateOneDeepMove();

    std::vector<Move> legalMoves = getSortedLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    int bestMoveIndex = 0;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i], true);
      int evaluation = -negamax(depth - 1, -beta, -alpha);
      unmakeMove(legalMoves[i]);

      if (evaluation > alpha)
      {
        alpha = evaluation;
        bestMoveIndex = i;
      }

      if (alpha >= beta)
        break;
    }

    return legalMoves[bestMoveIndex];
  }

  Move Board::iterativeDeepening(int time, std::chrono::time_point<std::chrono::high_resolution_clock> start)
  {
    int depth = botSettings.minSearchDepth;

    Move bestMove = generateBestMove(depth);

    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < time)
    {
      depth++;

      Move newBestMove = generateBestMove(depth);

      bestMove = newBestMove;
    }

    return bestMove;
  }

  std::vector<Move> Board::heuristicSortMoves(std::vector<Move> moves)
  {
    std::sort(moves.begin(), moves.end(), [this](Move a, Move b)
              { return heuristicEvaluation(a) > heuristicEvaluation(b); });

    return moves;
  }

  int Board::heuristicEvaluation(Move move)
  {
    int evaluation = 0;

    evaluation += PIECE_VALUES[move.capturedPiece & TYPE] * (move.flags & CAPTURE);
    evaluation += PIECE_VALUES[move.promotionPieceType] * (move.flags & PROMOTION);

    evaluation += getPiecePositionalEvaluation(move.to, true) - getPiecePositionalEvaluation(move.from, true);

    return evaluation;
  }

  int Board::countGames(int depth)
  {
    if (depth == 0)
      return 1;

    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    if (depth == 1 || legalMovesCount == 0)
      return legalMovesCount;

    int games = 0;

    for (int i = 0; i < legalMovesCount; i++)
    {
      makeMove(legalMoves[i], true);

      games += countGames(depth - 1);

      unmakeMove(legalMoves[i]);
    }

    return games;
  }
}