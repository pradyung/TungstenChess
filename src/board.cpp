#include "board.hpp" // See for documentation and helper function implementations

namespace TungstenChess
{
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

  void Board::resetBoard(std::string fen)
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

    for (int i = 0; i < ALL_PIECES + 1; i++)
      bitboards[i] = 0;

    sideToMove = WHITE;

    int pieceIndex = 0;
    for (int i = 0; i < fenParts[FEN_BOARD].length(); i++)
    {
      if (fen[i] == '/')
        continue;
      else if (isdigit(fen[i]))
      {
        for (int j = 0; j < fen[i] - '0'; j++)
        {
          board[pieceIndex] = EMPTY;
          pieceIndex++;
        }
      }
      else
      {
        board[pieceIndex] = std::string("PNBRQK..pnbrqk").find(fen[i]) + WHITE_PAWN;

        updatePiece(pieceIndex, board[pieceIndex]);

        pieceIndex++;
      }
    }

    sideToMove = fenParts[FEN_SIDE_TO_MOVE] == "w" ? WHITE : BLACK;

    if (fenParts[FEN_CASTLING_RIGHTS] != "-")
    {
      for (int i = 0; i < fenParts[FEN_CASTLING_RIGHTS].length(); i++)
      {
        if (fenParts[FEN_CASTLING_RIGHTS][i] == 'K')
          castlingRights |= WHITE_KINGSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'Q')
          castlingRights |= WHITE_QUEENSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'k')
          castlingRights |= BLACK_KINGSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'q')
          castlingRights |= BLACK_QUEENSIDE;
      }
    }

    if (fenParts[FEN_EN_PASSANT] != "-")
    {
      enPassantFile = fenParts[FEN_EN_PASSANT][0] - 'a';
    }

    zobristKey = 0;

    calculateInitialZobristKey();

    positionHistory.push_back(zobristKey);

    moveHistory.clear();
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

  Bitboard Board::getPawnMoves(int pieceIndex, Piece color, bool _, bool onlyCaptures)
  {
    Bitboard movesBitboard = 0;

    if (color & WHITE)
    {
      if (!board[pieceIndex - 8] && !onlyCaptures)
      {
        Bitboards::addBit(movesBitboard, pieceIndex - 8);
        if (pieceIndex >= A2 && pieceIndex <= H2 && !board[pieceIndex - 16])
          Bitboards::addBit(movesBitboard, pieceIndex - 16);
      }

      movesBitboard |= (movesLookup.PAWN_CAPTURE_MOVES[WHITE_PAWN][pieceIndex] & (bitboards[BLACK] | (enPassantFile == NO_EP ? 0 : 1ULL << (enPassantFile + 16))));
    }
    else if (color & BLACK)
    {
      if (!board[pieceIndex + 8] && !onlyCaptures)
      {
        Bitboards::addBit(movesBitboard, pieceIndex + 8);
        if (pieceIndex >= A7 && pieceIndex <= H7 && !board[pieceIndex + 16])
          Bitboards::addBit(movesBitboard, pieceIndex + 16);
      }

      movesBitboard |= (movesLookup.PAWN_CAPTURE_MOVES[BLACK_PAWN][pieceIndex] & (bitboards[WHITE] | (enPassantFile == NO_EP ? 0 : 1ULL << (enPassantFile + 40))));
    }

    return movesBitboard;
  }

  Bitboard Board::getKnightMoves(int pieceIndex, Piece color, bool _, bool __)
  {
    return movesLookup.KNIGHT_MOVES[pieceIndex] & ~bitboards[color];
  }

  Bitboard Board::getBishopMoves(int pieceIndex, Piece color, bool _, bool __)
  {
    return magicMoveGen.getBishopMoves(pieceIndex, bitboards[ALL_PIECES]) & ~bitboards[color];
  }

  Bitboard Board::getRookMoves(int pieceIndex, Piece color, bool _, bool __)
  {
    return magicMoveGen.getRookMoves(pieceIndex, bitboards[ALL_PIECES]) & ~bitboards[color];
  }

  Bitboard Board::getQueenMoves(int pieceIndex, Piece color, bool _, bool __)
  {
    Bitboard bishopMoves = magicMoveGen.getBishopMoves(pieceIndex, bitboards[ALL_PIECES]);
    Bitboard rookMoves = magicMoveGen.getRookMoves(pieceIndex, bitboards[ALL_PIECES]);

    return (bishopMoves | rookMoves) & ~bitboards[color];
  }

  Bitboard Board::getKingMoves(int pieceIndex, Piece color, bool includeCastling, bool __)
  {
    Bitboard movesBitboard = movesLookup.KING_MOVES[pieceIndex] & ~bitboards[color];

    if (includeCastling && castlingRights)
    {
      if (color & WHITE)
      {
        if (castlingRights & WHITE_KINGSIDE && !board[F1] && !board[G1] && !isInCheck(WHITE) && !isAttacked(F1, BLACK))
          Bitboards::addBit(movesBitboard, G1);

        if (castlingRights & WHITE_QUEENSIDE && !board[D1] && !board[C1] && !board[B1] && !isInCheck(WHITE) && !isAttacked(D1, BLACK))
          Bitboards::addBit(movesBitboard, C1);
      }
      else if (color & BLACK)
      {
        if (castlingRights & BLACK_KINGSIDE && !board[F8] && !board[G8] && !isInCheck(BLACK) && !isAttacked(F8, WHITE))
          Bitboards::addBit(movesBitboard, G8);

        if (castlingRights & BLACK_QUEENSIDE && !board[D8] && !board[C8] && !board[B8] && !isInCheck(BLACK) && !isAttacked(D8, WHITE))
          Bitboards::addBit(movesBitboard, C8);
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex, Piece color, bool includeCastling)
  {
    Bitboard pseudoLegalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, color, includeCastling);

    Bitboard legalMovesBitboard = 0;

    while (pseudoLegalMovesBitboard)
    {
      int toIndex = __builtin_ctzll(pseudoLegalMovesBitboard);

      Bitboards::removeBit(pseudoLegalMovesBitboard, toIndex);

      MoveFlags flag = quickMakeMove(pieceIndex, toIndex);

      if (!isInCheck(color))
        Bitboards::addBit(legalMovesBitboard, toIndex);

      quickUnmakeMove(pieceIndex, toIndex, flag);
    }

    return legalMovesBitboard;
  }

  Bitboard Board::getAttackingPiecesBitboard(int targetSquare, Piece targetPiece, Piece color, bool onlyCaptures)
  {
    Bitboard attackingPiecesBitboard = 0;

    if (targetPiece)
    {
      if (Bitboard attackingPawns = movesLookup.PAWN_CAPTURE_MOVES[color ^ COLOR][targetSquare] & bitboards[color | PAWN])
        attackingPiecesBitboard |= attackingPawns;
    }
    else
    {
      Bitboard reverseSinglePawnMoveSquare = movesLookup.PAWN_REVERSE_SINGLE_MOVES[color][targetSquare];

      if (Bitboard attackingSingleMovePawns = reverseSinglePawnMoveSquare & bitboards[color | PAWN])
      {
        attackingPiecesBitboard |= attackingSingleMovePawns;
      }
      else if (!(reverseSinglePawnMoveSquare & bitboards[ALL_PIECES]))
      {
        Bitboard reverseDoublePawnMoveSquare = movesLookup.PAWN_REVERSE_DOUBLE_MOVES[color][targetSquare];

        if (Bitboard attackingDoubleMovePawns = reverseDoublePawnMoveSquare & bitboards[color | PAWN])
          attackingPiecesBitboard |= attackingDoubleMovePawns;
      }
    }

    if (Bitboard attackingKnights = movesLookup.KNIGHT_MOVES[targetSquare] & bitboards[color | KNIGHT])
      attackingPiecesBitboard |= attackingKnights;

    if (Bitboard attackingDiagonalSliders = getBishopMoves(targetSquare, color ^ COLOR) & (bitboards[color | BISHOP] | bitboards[color | QUEEN]))
      attackingPiecesBitboard |= attackingDiagonalSliders;

    if (Bitboard attackingOrthogonalSliders = getRookMoves(targetSquare, color ^ COLOR) & (bitboards[color | ROOK] | bitboards[color | QUEEN]))
      attackingPiecesBitboard |= attackingOrthogonalSliders;

    return attackingPiecesBitboard;
  }

  std::vector<Move> Board::getLegalMoves(int color, bool includeCastling)
  {
    std::vector<Move> legalMoves;
    legalMoves.reserve(256);

    Bitboard movablePiecesBitboard = 0;
    Bitboard targetSquaresBitboard = 0;

    if (Bitboard attackingKnights = movesLookup.KNIGHT_MOVES[kingIndices[color | KING]] & bitboards[(color ^ COLOR) | KNIGHT])
    {
      movablePiecesBitboard = bitboards[color | KING];

      if (Bitboards::countBits(attackingKnights) == 1)
        targetSquaresBitboard = attackingKnights;
    }
    else
    {
      Bitboard diagonalMoves = magicMoveGen.getBishopMoves(kingIndices[color | KING], bitboards[ALL_PIECES]);
      Bitboard orthogonalMoves = magicMoveGen.getRookMoves(kingIndices[color | KING], bitboards[ALL_PIECES]);

      Piece opposingColor = color ^ COLOR;

      Bitboard attackingDiagonalSliders = diagonalMoves & (bitboards[opposingColor | BISHOP] | bitboards[opposingColor | QUEEN]);
      Bitboard attackingOrthogonalSliders = orthogonalMoves & (bitboards[opposingColor | ROOK] | bitboards[opposingColor | QUEEN]);

      if (!(attackingDiagonalSliders | attackingOrthogonalSliders))
        movablePiecesBitboard = bitboards[color];
      else if (Bitboards::countBits(attackingDiagonalSliders | attackingOrthogonalSliders) > 1)
        movablePiecesBitboard = bitboards[color | KING];
      else if (attackingDiagonalSliders)
      {
        movablePiecesBitboard = bitboards[color | KING];
        targetSquaresBitboard = diagonalMoves & movesLookup.BISHOP_MASKS[__builtin_ctzll(attackingDiagonalSliders)] | attackingDiagonalSliders;
      }
      else
      {
        movablePiecesBitboard = bitboards[color | KING];
        targetSquaresBitboard = orthogonalMoves & movesLookup.ROOK_MASKS[__builtin_ctzll(attackingOrthogonalSliders)] | attackingOrthogonalSliders;
      }
    }

    while (movablePiecesBitboard)
    {
      int pieceIndex = __builtin_ctzll(movablePiecesBitboard);

      Bitboards::removeBit(movablePiecesBitboard, pieceIndex);

      Bitboard movesBitboard = getLegalPieceMovesBitboard(pieceIndex, color, includeCastling);

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

    while (targetSquaresBitboard)
    {
      int targetSquare = __builtin_ctzll(targetSquaresBitboard);

      Bitboards::removeBit(targetSquaresBitboard, targetSquare);

      Piece targetPiece = board[targetSquare];

      Bitboard attackersBitboard = getAttackingPiecesBitboard(targetSquare, targetPiece, color, true);

      while (attackersBitboard)
      {
        int attackerIndex = __builtin_ctzll(attackersBitboard);

        Bitboards::removeBit(attackersBitboard, attackerIndex);

        MoveFlags flag = quickMakeMove(attackerIndex, targetSquare);

        if (!isInCheck(color))
        {
          legalMoves.push_back(Move(attackerIndex, targetSquare, board[attackerIndex], board[targetSquare], castlingRights, enPassantFile, halfmoveClock, EMPTY));

          if (flag & PROMOTION)
          {
            legalMoves.back().promotionPieceType = QUEEN;
            legalMoves.push_back(Move(attackerIndex, targetSquare, board[attackerIndex], board[targetSquare], castlingRights, enPassantFile, halfmoveClock, KNIGHT));
            legalMoves.push_back(Move(attackerIndex, targetSquare, board[attackerIndex], board[targetSquare], castlingRights, enPassantFile, halfmoveClock, BISHOP));
            legalMoves.push_back(Move(attackerIndex, targetSquare, board[attackerIndex], board[targetSquare], castlingRights, enPassantFile, halfmoveClock, ROOK));
          }
        }

        quickUnmakeMove(attackerIndex, targetSquare, flag);
      }
    }

    return legalMoves;
  }

  bool Board::isAttacked(int square, int color)
  {
    if (movesLookup.PAWN_CAPTURE_MOVES[color ^ COLOR][square] & bitboards[color | PAWN])
      return true;

    else if (movesLookup.KNIGHT_MOVES[square] & bitboards[color | KNIGHT])
      return true;

    else if (movesLookup.KING_MOVES[square] & bitboards[color | KING])
      return true;

    else if (getBishopMoves(square, color ^ COLOR) & (bitboards[color | BISHOP] | bitboards[color | QUEEN]))
      return true;

    else if (getRookMoves(square, color ^ COLOR) & (bitboards[color | ROOK] | bitboards[color | QUEEN]))
      return true;

    return false;
  }

  int Board::getGameStatus(int color)
  {
    if (countRepetitions(zobristKey) >= 3)
      return STALEMATE;

    Bitboard friendlyPiecesBitboard = bitboards[color];

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
        pgn += "..NBRQ"[move.promotionPieceType];
      }
    }

    makeMove(move);

    int gameStatus = getGameStatus(sideToMove);

    if (isInCheck(sideToMove))
      pgn += gameStatus == LOSE ? "#" : "+";

    unmakeMove(move);

    return pgn;
  }

  int Board::countGames(int depth, bool verbose)
  {
    if (depth == 0)
      return 1;

    std::vector<Move> legalMoves = getLegalMoves(sideToMove);

    int legalMovesCount = legalMoves.size();

    int games = 0;

    for (int i = 0; i < legalMovesCount; i++)
    {
      if (depth == 1)
      {
        games++;

        if (verbose)
          std::cout << legalMoves[i].getUCI() << ": 1" << std::endl;

        continue;
      }

      makeMove(legalMoves[i]);

      int newGames = countGames(depth - 1, false);

      if (verbose)
        std::cout << legalMoves[i].getUCI() << ": " << newGames << std::endl;

      games += newGames;

      unmakeMove(legalMoves[i]);
    }

    return games;
  }
}