#include "board.hpp" // See for documentation and helper function implementations

namespace TungstenChess
{
  Board::Board(std::string fen) : isDefaultStartPosition(fen == START_FEN)
  {
    std::string fenParts[NUM_FEN_PARTS];

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

  void Board::makeMove(Move move)
  {
    switchSideToMove();

    halfmoveClock++;

    if (board[move.to] || move.piece == PAWN)
      halfmoveClock = 0;

    moveHistory.push_back(move.toInt());

    auto [from, to, piece, capturedPiece, promotionPieceType, castlingRights, enPassantFile, halfmoveClock, flags] = move;

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
    moveHistory.pop_back();

    switchSideToMove();

    halfmoveClock = move.halfmoveClock;

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
    Bitboard movesBitboard = 0;

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
    return movesLookup.KNIGHT_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
  }

  Bitboard Board::getBishopMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    Bitboard maskedBlockers = movesLookup.BISHOP_MASKS[pieceIndex] & blockersBitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.BISHOP_MAGICS[pieceIndex]) >> magicMoveGen.BISHOP_SHIFTS[pieceIndex];

    return magicMoveGen.BISHOP_LOOKUP_TABLES[pieceIndex][magicIndex] & ~friendlyPiecesBitboard;
  }

  Bitboard Board::getRookMoves(int pieceIndex, bool _, bool __)
  {
    Bitboard friendlyPiecesBitboard = getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);
    Bitboard blockersBitboard = friendlyPiecesBitboard | getEnemyPiecesBitboard(board[pieceIndex] & COLOR);

    Bitboard maskedBlockers = movesLookup.ROOK_MASKS[pieceIndex] & blockersBitboard;

    int magicIndex = (maskedBlockers * magicMoveGen.ROOK_MAGICS[pieceIndex]) >> magicMoveGen.ROOK_SHIFTS[pieceIndex];

    return magicMoveGen.ROOK_LOOKUP_TABLES[pieceIndex][magicIndex] & ~friendlyPiecesBitboard;
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

    return (bishopMoves | rookMoves) & ~friendlyPiecesBitboard;
  }

  Bitboard Board::getKingMoves(int pieceIndex, bool includeCastling, bool __)
  {
    Bitboard movesBitboard = movesLookup.KING_MOVES[pieceIndex] & ~getFriendlyPiecesBitboard(board[pieceIndex] & COLOR);

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

    Bitboard legalMovesBitboard = 0;

    while (pseudoLegalMovesBitboard)
    {
      int toIndex = __builtin_ctzll(pseudoLegalMovesBitboard);

      Bitboards::removeBit(pseudoLegalMovesBitboard, toIndex);

      Move move = Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, halfmoveClock, QUEEN);

      makeMove(move);

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

        legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, halfmoveClock, EMPTY));

        if (legalMoves.back().flags & PROMOTION)
        {
          legalMoves.back().promotionPieceType = QUEEN;
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, halfmoveClock, KNIGHT));
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, halfmoveClock, BISHOP));
          legalMoves.push_back(Move(pieceIndex, toIndex, board[pieceIndex], board[toIndex], castlingRights, enPassantFile, halfmoveClock, ROOK));
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
        return halfmoveClock >= 100 ? STALEMATE : NO_MATE;
    }

    return isInCheck(color) ? LOSE : STALEMATE;
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

    return Move(from, to, piece, capturedPiece, castlingRights, enPassantFile, halfmoveClock, promotionPieceType);
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

        Bitboard sameTypePieces = bitboards[move.piece] & ~(1ULL << move.from);
        Bitboard ambiguousPieces = 0;

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

    makeMove(move);

    int gameStatus = getGameStatus(sideToMove);

    if (isInCheck(sideToMove))
      pgn += gameStatus == LOSE ? "#" : "+";

    unmakeMove(move);

    return pgn;
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
      makeMove(legalMoves[i]);

      games += countGames(depth - 1);

      unmakeMove(legalMoves[i]);
    }

    return games;
  }
}