#include "core/board.hpp"

namespace TungstenChess
{
  Bitboard Board::getPawnMoves(Square pieceIndex, PieceColor color, bool _) const
  {
    Bitboard movesBitboard = 0;

    if (color & WHITE)
    {
      if (!m_board[pieceIndex - 8])
      {
        Bitboards::addBit(movesBitboard, pieceIndex - 8);
        if (pieceIndex >= A2 && pieceIndex <= H2 && !m_board[pieceIndex - 16])
          Bitboards::addBit(movesBitboard, pieceIndex - 16);
      }

      movesBitboard |= (MovesLookup::PAWN_CAPTURE_MOVES[WHITE_PAWN, pieceIndex] & (m_bitboards[BLACK] | (m_enPassantFile == NO_EP ? 0 : 1ULL << (m_enPassantFile + 16))));
    }
    else if (color & BLACK)
    {
      if (!m_board[pieceIndex + 8])
      {
        Bitboards::addBit(movesBitboard, pieceIndex + 8);
        if (pieceIndex >= A7 && pieceIndex <= H7 && !m_board[pieceIndex + 16])
          Bitboards::addBit(movesBitboard, pieceIndex + 16);
      }

      movesBitboard |= (MovesLookup::PAWN_CAPTURE_MOVES[BLACK_PAWN, pieceIndex] & (m_bitboards[WHITE] | (m_enPassantFile == NO_EP ? 0 : 1ULL << (m_enPassantFile + 40))));
    }

    return movesBitboard;
  }

  Bitboard Board::getKnightMoves(Square pieceIndex, PieceColor color, bool _) const
  {
    return MovesLookup::KNIGHT_MOVES[pieceIndex] & ~m_bitboards[color];
  }

  Bitboard Board::getBishopMoves(Square pieceIndex, PieceColor color, bool _) const
  {
    return MagicMoveGen::getBishopMoves(pieceIndex, m_bitboards[ALL_PIECES]) & ~m_bitboards[color];
  }

  Bitboard Board::getRookMoves(Square pieceIndex, PieceColor color, bool _) const
  {
    return MagicMoveGen::getRookMoves(pieceIndex, m_bitboards[ALL_PIECES]) & ~m_bitboards[color];
  }

  Bitboard Board::getQueenMoves(Square pieceIndex, PieceColor color, bool _) const
  {
    Bitboard bishopMoves = MagicMoveGen::getBishopMoves(pieceIndex, m_bitboards[ALL_PIECES]);
    Bitboard rookMoves = MagicMoveGen::getRookMoves(pieceIndex, m_bitboards[ALL_PIECES]);

    return (bishopMoves | rookMoves) & ~m_bitboards[color];
  }

  Bitboard Board::getKingMoves(Square pieceIndex, PieceColor color, bool includeCastling) const
  {
    Bitboard movesBitboard = MovesLookup::KING_MOVES[pieceIndex] & ~m_bitboards[color];

    if (includeCastling && m_castlingRights)
    {
      if (color & WHITE)
      {
        if (m_castlingRights & WHITE_KINGSIDE && !m_board[F1] && !m_board[G1] && !isInCheck(WHITE) && !isAttacked(F1, BLACK))
          Bitboards::addBit(movesBitboard, G1);

        if (m_castlingRights & WHITE_QUEENSIDE && !m_board[D1] && !m_board[C1] && !m_board[B1] && !isInCheck(WHITE) && !isAttacked(D1, BLACK))
          Bitboards::addBit(movesBitboard, C1);
      }
      else if (color & BLACK)
      {
        if (m_castlingRights & BLACK_KINGSIDE && !m_board[F8] && !m_board[G8] && !isInCheck(BLACK) && !isAttacked(F8, WHITE))
          Bitboards::addBit(movesBitboard, G8);

        if (m_castlingRights & BLACK_QUEENSIDE && !m_board[D8] && !m_board[C8] && !m_board[B8] && !isInCheck(BLACK) && !isAttacked(D8, WHITE))
          Bitboards::addBit(movesBitboard, C8);
      }
    }

    return movesBitboard;
  }

  Bitboard Board::getPseudoLegalPieceMoves(Square pieceIndex, PieceColor color, bool includeCastling) const
  {
    return (this->*getPieceMoves[m_board[pieceIndex] & TYPE])(pieceIndex, color, includeCastling);
  }

  Bitboard Board::getPseudoLegalPieceMovesBitboard(Square pieceIndex) const
  {
    return getPseudoLegalPieceMoves(pieceIndex, m_board[pieceIndex] & COLOR);
  }

  Bitboard Board::getLegalPieceMovesBitboard(Square pieceIndex, PieceColor color, bool onlyCaptures)
  {
    Bitboard pseudoLegalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, color, !onlyCaptures);

    if (onlyCaptures)
      pseudoLegalMovesBitboard &= m_bitboards[color ^ COLOR];

    Bitboard legalMovesBitboard = 0;

    while (pseudoLegalMovesBitboard)
    {
      Square toIndex = Bitboards::popBit(pseudoLegalMovesBitboard);

      MoveFlags flag = quickMakeMove(pieceIndex, toIndex);

      if (!isInCheck(color))
        Bitboards::addBit(legalMovesBitboard, toIndex);

      quickUnmakeMove(pieceIndex, toIndex, flag);
    }

    return legalMovesBitboard;
  }

  Bitboard Board::getLegalPieceMovesBitboard(Square pieceIndex)
  {
    return getLegalPieceMovesBitboard(pieceIndex, m_board[pieceIndex] & COLOR);
  }

  Bitboard Board::getAttackingPiecesBitboard(Square targetSquare, Piece targetPiece, PieceColor color) const
  {
    Bitboard attackingPiecesBitboard = 0;

    const Bitboard *friendlyBitboards = &m_bitboards[color];

    if (targetPiece)
    {
      if (Bitboard attackingPawns = MovesLookup::PAWN_CAPTURE_MOVES[color ^ COLOR, targetSquare] & friendlyBitboards[PAWN])
        attackingPiecesBitboard |= attackingPawns;
    }
    else
    {
      Bitboard reverseSinglePawnMoveSquare = MovesLookup::PAWN_REVERSE_SINGLE_MOVES[color, targetSquare];

      if (Bitboard attackingSingleMovePawns = reverseSinglePawnMoveSquare & friendlyBitboards[PAWN])
      {
        attackingPiecesBitboard |= attackingSingleMovePawns;
      }
      else if (!(reverseSinglePawnMoveSquare & m_bitboards[ALL_PIECES]))
      {
        Bitboard reverseDoublePawnMoveSquare = MovesLookup::PAWN_REVERSE_DOUBLE_MOVES[color, targetSquare];

        if (Bitboard attackingDoubleMovePawns = reverseDoublePawnMoveSquare & friendlyBitboards[PAWN])
          attackingPiecesBitboard |= attackingDoubleMovePawns;
      }
    }

    if (Bitboard attackingKnights = MovesLookup::KNIGHT_MOVES[targetSquare] & friendlyBitboards[KNIGHT])
      attackingPiecesBitboard |= attackingKnights;

    if (Bitboard attackingDiagonalSliders = getBishopMoves(targetSquare, color ^ COLOR) & (friendlyBitboards[BISHOP] | friendlyBitboards[QUEEN]))
      attackingPiecesBitboard |= attackingDiagonalSliders;

    if (Bitboard attackingOrthogonalSliders = getRookMoves(targetSquare, color ^ COLOR) & (friendlyBitboards[ROOK] | friendlyBitboards[QUEEN]))
      attackingPiecesBitboard |= attackingOrthogonalSliders;

    return attackingPiecesBitboard;
  }

  int Board::getLegalMoves(MoveArray &legalMoves, PieceColor color, bool onlyCaptures)
  {
    int legalMovesIndex = 0;

    Bitboard movablePiecesBitboard = 0;
    Bitboard targetSquaresBitboard = 0;

    const Bitboard *friendlyBitboards = &m_bitboards[color];
    const Bitboard *enemyBitboards = &m_bitboards[color ^ COLOR];

    int kingIndex = m_kingIndices[color | KING];

    if (Bitboard attackingKnights = MovesLookup::KNIGHT_MOVES[kingIndex] & enemyBitboards[KNIGHT])
    {
      movablePiecesBitboard = friendlyBitboards[KING];

      if (Bitboards::countBits(attackingKnights) == 1)
        targetSquaresBitboard = attackingKnights;
    }
    else
    {
      Bitboard diagonalMoves = MagicMoveGen::getBishopMoves(kingIndex, m_bitboards[ALL_PIECES]);
      Bitboard orthogonalMoves = MagicMoveGen::getRookMoves(kingIndex, m_bitboards[ALL_PIECES]);

      PieceColor enemyColor = color ^ COLOR;

      Bitboard attackingDiagonalSliders = diagonalMoves & (enemyBitboards[BISHOP] | enemyBitboards[QUEEN]);
      Bitboard attackingOrthogonalSliders = orthogonalMoves & (enemyBitboards[ROOK] | enemyBitboards[QUEEN]);

      if (!(attackingDiagonalSliders | attackingOrthogonalSliders))
        movablePiecesBitboard = m_bitboards[color];
      else
      {
        movablePiecesBitboard = friendlyBitboards[KING];
        if (attackingDiagonalSliders)
          targetSquaresBitboard = (diagonalMoves & MovesLookup::BISHOP_MASKS[__builtin_ctzll(attackingDiagonalSliders)]) | attackingDiagonalSliders;
        else if (attackingOrthogonalSliders)
          targetSquaresBitboard = (orthogonalMoves & MovesLookup::ROOK_MASKS[__builtin_ctzll(attackingOrthogonalSliders)]) | attackingOrthogonalSliders;
      }
    }

    while (movablePiecesBitboard)
    {
      Square pieceIndex = Bitboards::popBit(movablePiecesBitboard);

      Bitboard movesBitboard = getLegalPieceMovesBitboard(pieceIndex, color, onlyCaptures);

      while (movesBitboard)
      {
        Square toIndex = Bitboards::popBit(movesBitboard);

        legalMoves[legalMovesIndex++] = Moves::createMove(pieceIndex, toIndex);

        Move &move = legalMoves[legalMovesIndex - 1];

        if (Moves::isPromotion(toIndex, m_board[pieceIndex] & TYPE))
        {
          legalMoves[legalMovesIndex++] = move | KNIGHT_PROMOTION;
          legalMoves[legalMovesIndex++] = move | BISHOP_PROMOTION;
          legalMoves[legalMovesIndex++] = move | ROOK_PROMOTION;
          move |= QUEEN_PROMOTION;
        }
      }
    }

    if (onlyCaptures)
      targetSquaresBitboard &= m_bitboards[color ^ COLOR];

    while (targetSquaresBitboard)
    {
      Square targetSquare = Bitboards::popBit(targetSquaresBitboard);

      Piece targetPiece = m_board[targetSquare];

      Bitboard attackersBitboard = getAttackingPiecesBitboard(targetSquare, targetPiece, color);

      while (attackersBitboard)
      {
        Square attackerIndex = Bitboards::popBit(attackersBitboard);

        MoveFlags flag = quickMakeMove(attackerIndex, targetSquare);

        if (!isInCheck(color))
        {
          legalMoves[legalMovesIndex++] = Moves::createMove(attackerIndex, targetSquare);

          Move &move = legalMoves[legalMovesIndex - 1];

          if (flag & PROMOTION)
          {
            legalMoves[legalMovesIndex++] = move | KNIGHT_PROMOTION;
            legalMoves[legalMovesIndex++] = move | BISHOP_PROMOTION;
            legalMoves[legalMovesIndex++] = move | ROOK_PROMOTION;
            move |= QUEEN_PROMOTION;
          }
        }

        quickUnmakeMove(attackerIndex, targetSquare, flag);
      }
    }

    if (legalMovesIndex < MAX_MOVE_COUNT)
      legalMoves[legalMovesIndex] = NULL_MOVE;

    return legalMovesIndex;
  }

  bool Board::isAttacked(Square square, PieceColor color) const
  {
    PieceColor attackedColor = color ^ COLOR;

    const Bitboard *attackerBitboards = &m_bitboards[color];

    if (MovesLookup::PAWN_CAPTURE_MOVES[attackedColor, square] & attackerBitboards[PAWN])
      return true;

    if (MovesLookup::KNIGHT_MOVES[square] & attackerBitboards[KNIGHT])
      return true;

    if (MovesLookup::KING_MOVES[square] & attackerBitboards[KING])
      return true;

    if (Bitboard diagonalSliders = (attackerBitboards[BISHOP] | attackerBitboards[QUEEN]))
    {
      if (getBishopMoves(square, attackedColor) & diagonalSliders)
        return true;
    }

    if (Bitboard orthogonalSliders = (attackerBitboards[ROOK] | attackerBitboards[QUEEN]))
    {
      if (getRookMoves(square, attackedColor) & orthogonalSliders)
        return true;
    }

    return false;
  }

  bool Board::isInCheck(PieceColor color) const
  {
    return isAttacked(m_kingIndices[color | KING], color ^ COLOR);
  }

  uint8_t Board::countRepetitions(ZobristKey key) const
  {
    uint8_t count = 0;

    for (size_t i = 0; i < m_positionHistory.size(); i++)
      if (m_positionHistory[i] == key)
        count++;

    return count;
  }

  Board::GameStatus Board::getGameStatus(PieceColor color)
  {
    if (countRepetitions(m_zobristKey) >= 3)
      return STALEMATE;

    Bitboard friendlyPiecesBitboard = m_bitboards[color];

    while (friendlyPiecesBitboard)
    {
      Square pieceIndex = Bitboards::popBit(friendlyPiecesBitboard);

      if (getLegalPieceMovesBitboard(pieceIndex))
        return m_halfmoveClock >= 100 ? STALEMATE : NO_MATE;
    }

    return isInCheck(color) ? LOSE : STALEMATE;
  }

  uint Board::countGames(uint8_t depth, bool verbose)
  {
    if (depth == 0)
      return 1;

    MoveArray legalMoves;
    int legalMovesCount = getLegalMoves(legalMoves, m_sideToMove);

    uint games = 0;

    for (int i = 0; i < legalMovesCount; i++)
    {
      if (depth == 1)
      {
        games++;

        if (verbose)
          std::cout << Moves::getUCI(legalMoves[i]) << ": 1" << std::endl;

        continue;
      }

      UnmoveData unmoveData = makeMove(legalMoves[i]);

      uint newGames = countGames(depth - 1, false);

      if (verbose)
        std::cout << Moves::getUCI(legalMoves[i]) << ": " << newGames << std::endl;

      games += newGames;

      unmakeMove(legalMoves[i], unmoveData);
    }

    return games;
  }

  Move Board::generateMoveFromUCI(std::string uci) const
  {
    Square from = (uci[0] - 'a') + (8 - uci[1] + '0') * 8;
    Square to = (uci[2] - 'a') + (8 - uci[3] + '0') * 8;

    PieceType promotionPieceType = NO_TYPE;

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

    return Moves::createMove(from, to, promotionPieceType);
  }

  std::string Board::getMovePGN(Move move)
  {
    std::string pgn = "";

    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;
    PieceType promotionPieceType = move >> 12;

    Piece piece = m_board[from];

    uint8_t flags = Moves::getMoveFlags(from, to, piece & TYPE, m_board[to]);

    if (flags & CASTLE)
    {
      if (flags & KSIDE_CASTLE)
        pgn += "O-O";
      else
        pgn += "O-O-O";
    }
    else
    {
      PieceType pieceType = piece & TYPE;

      if (pieceType != PAWN)
      {
        pgn += "..NBRQK"[pieceType];

        Bitboard sameTypePieces = m_bitboards[piece] & ~(1ULL << from);
        Bitboard ambiguousPieces = 0;

        while (sameTypePieces)
        {
          Square pieceIndex = Bitboards::popBit(sameTypePieces);

          if (Bitboards::hasBit(getLegalPieceMovesBitboard(pieceIndex), to))
            Bitboards::addBit(ambiguousPieces, pieceIndex);
        }

        if (ambiguousPieces)
        {
          if (Bitboards::file(ambiguousPieces, from % 8))
          {
            if (Bitboards::rank(ambiguousPieces, from / 8))
              pgn += 'a' + (from % 8);
            pgn += '8' - (from / 8);
          }
          else
            pgn += 'a' + (from % 8);
        }
      }

      if (flags & (CAPTURE | EP_CAPTURE))
      {
        if (pieceType == PAWN)
        {
          pgn += 'a' + (from % 8);
        }
        pgn += 'x';
      }

      pgn += 'a' + (to % 8);
      pgn += '8' - (to / 8);

      if (flags & EP_CAPTURE)
        pgn += " ep";
      else if (flags & PROMOTION)
      {
        pgn += "=";
        pgn += "..NBRQ"[promotionPieceType];
      }
    }

    UnmoveData unmoveData = makeMove(move);

    Board::GameStatus gameStatus = getGameStatus(m_sideToMove);

    if (isInCheck(m_sideToMove))
      pgn += gameStatus == LOSE ? "#" : "+";

    unmakeMove(move, unmoveData);

    return pgn;
  }
}