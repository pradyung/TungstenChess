#include "board.hpp" // See for documentation and helper function implementations

namespace TungstenChess
{
  void Board::calculateInitialZobristKey()
  {
    for (int i = 0; i < 64; i++)
    {
      if (m_board[i])
      {
        m_zobristKey ^= Zobrist::pieceKeys[i][m_board[i]];
      }
    }

    m_zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
    m_zobristKey ^= Zobrist::enPassantKeys[m_enPassantFile];

    if (m_sideToMove == WHITE)
      m_zobristKey ^= Zobrist::sideKey;
  }

  void Board::resetBoard(std::string fen)
  {
    std::string fenParts[NUM_FEN_PARTS];

    int fenPartIndex = 0;

    for (size_t i = 0; i < fen.length(); i++)
    {
      if (fen[i] == ' ')
      {
        fenPartIndex++;
        continue;
      }

      fenParts[fenPartIndex] += fen[i];
    }

    m_castlingRights = 0;
    m_enPassantFile = NO_EP;

    for (int i = 0; i < ALL_PIECES + 1; i++)
      m_bitboards[i] = 0;

    m_sideToMove = WHITE;

    int pieceIndex = 0;
    for (size_t i = 0; i < fenParts[FEN_BOARD].length(); i++)
    {
      if (fen[i] == '/')
        continue;
      else if (isdigit(fen[i]))
      {
        for (int j = 0; j < fen[i] - '0'; j++)
        {
          m_board[pieceIndex] = EMPTY;
          pieceIndex++;
        }
      }
      else
      {
        m_board[pieceIndex] = std::string("PNBRQK..pnbrqk").find(fen[i]) + WHITE_PAWN;

        updatePiece(pieceIndex, m_board[pieceIndex]);

        pieceIndex++;
      }
    }

    m_sideToMove = fenParts[FEN_SIDE_TO_MOVE] == "w" ? WHITE : BLACK;

    if (fenParts[FEN_CASTLING_RIGHTS] != "-")
    {
      for (size_t i = 0; i < fenParts[FEN_CASTLING_RIGHTS].length(); i++)
      {
        if (fenParts[FEN_CASTLING_RIGHTS][i] == 'K')
          m_castlingRights |= WHITE_KINGSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'Q')
          m_castlingRights |= WHITE_QUEENSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'k')
          m_castlingRights |= BLACK_KINGSIDE;
        else if (fenParts[FEN_CASTLING_RIGHTS][i] == 'q')
          m_castlingRights |= BLACK_QUEENSIDE;
      }
    }

    if (fenParts[FEN_EN_PASSANT] != "-")
    {
      m_enPassantFile = fenParts[FEN_EN_PASSANT][0] - 'a';
    }

    m_zobristKey = 0;

    calculateInitialZobristKey();

    m_positionHistory.push_back(m_zobristKey);

    m_moveHistory.clear();
  }

  void Board::makeMove(Move move)
  {
    switchSideToMove();

    m_halfmoveClock++;

    if (m_board[move.to] || move.piece == PAWN)
      m_halfmoveClock = 0;

    m_moveHistory.push_back(move.toInt());

    auto [from, to, piece, capturedPiece, promotionPieceType, castlingRights, enPassantFile, halfmoveClock, flags] = move;

    PieceType pieceType = piece & TYPE;
    PieceColor pieceColor = piece & COLOR;
    PieceType capturedPieceType = capturedPiece & TYPE;
    PieceColor capturedPieceColor = capturedPiece & COLOR;

    movePiece(from, to, (flags & PROMOTION) ? (promotionPieceType | pieceColor) : EMPTY);

    updateEnPassantFile(flags & PAWN_DOUBLE ? to % 8 : NO_EP);

    if (pieceType == KING)
      removeCastlingRights(pieceColor, BOTHSIDES);

    if (pieceType == ROOK && ((pieceColor == WHITE && (from == A1 || from == H1)) || (pieceColor == BLACK && (from == A8 || from == H8))))
      removeCastlingRights(pieceColor, from % 8 == 0 ? QUEENSIDE : KINGSIDE);
    if (capturedPieceType == ROOK && ((capturedPieceColor == WHITE && (to == A1 || to == H1)) || (capturedPieceColor == BLACK && (to == A8 || to == H8))))
      removeCastlingRights(capturedPieceColor, to % 8 == 0 ? QUEENSIDE : KINGSIDE);

    if (flags & EP_CAPTURE)
      updatePiece(to + (piece & WHITE ? 8 : -8), EMPTY);

    if (flags & CASTLE)
    {
      m_hasCastled |= pieceColor;

      if (flags & KSIDE_CASTLE)
        movePiece(to + 1, to - 1);
      else
        movePiece(to - 2, to + 1);
    }

    m_positionHistory.push_back(m_zobristKey);
  }

  void Board::unmakeMove(Move move)
  {
    m_positionHistory.pop_back();
    m_moveHistory.pop_back();

    switchSideToMove();

    m_halfmoveClock = move.halfmoveClock;

    unmovePiece(move.from, move.to, move.piece, move.capturedPiece);

    if (move.flags & CASTLE)
    {
      m_hasCastled &= ~(move.piece & COLOR);

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

  Bitboard Board::getPawnMoves(int pieceIndex, PieceColor color, bool _)
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

      movesBitboard |= (MovesLookup::PAWN_CAPTURE_MOVES[WHITE_PAWN][pieceIndex] & (m_bitboards[BLACK] | (m_enPassantFile == NO_EP ? 0 : 1ULL << (m_enPassantFile + 16))));
    }
    else if (color & BLACK)
    {
      if (!m_board[pieceIndex + 8])
      {
        Bitboards::addBit(movesBitboard, pieceIndex + 8);
        if (pieceIndex >= A7 && pieceIndex <= H7 && !m_board[pieceIndex + 16])
          Bitboards::addBit(movesBitboard, pieceIndex + 16);
      }

      movesBitboard |= (MovesLookup::PAWN_CAPTURE_MOVES[BLACK_PAWN][pieceIndex] & (m_bitboards[WHITE] | (m_enPassantFile == NO_EP ? 0 : 1ULL << (m_enPassantFile + 40))));
    }

    return movesBitboard;
  }

  Bitboard Board::getKnightMoves(int pieceIndex, PieceColor color, bool _)
  {
    return MovesLookup::KNIGHT_MOVES[pieceIndex] & ~m_bitboards[color];
  }

  Bitboard Board::getBishopMoves(int pieceIndex, PieceColor color, bool _)
  {
    return MagicMoveGen::getBishopMoves(pieceIndex, m_bitboards[ALL_PIECES]) & ~m_bitboards[color];
  }

  Bitboard Board::getRookMoves(int pieceIndex, PieceColor color, bool _)
  {
    return MagicMoveGen::getRookMoves(pieceIndex, m_bitboards[ALL_PIECES]) & ~m_bitboards[color];
  }

  Bitboard Board::getQueenMoves(int pieceIndex, PieceColor color, bool _)
  {
    Bitboard bishopMoves = MagicMoveGen::getBishopMoves(pieceIndex, m_bitboards[ALL_PIECES]);
    Bitboard rookMoves = MagicMoveGen::getRookMoves(pieceIndex, m_bitboards[ALL_PIECES]);

    return (bishopMoves | rookMoves) & ~m_bitboards[color];
  }

  Bitboard Board::getKingMoves(int pieceIndex, PieceColor color, bool includeCastling)
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

  Bitboard Board::getLegalPieceMovesBitboard(int pieceIndex, PieceColor color, bool onlyCaptures)
  {
    Bitboard pseudoLegalMovesBitboard = getPseudoLegalPieceMoves(pieceIndex, color, !onlyCaptures);

    if (onlyCaptures)
      pseudoLegalMovesBitboard &= m_bitboards[color ^ COLOR];

    Bitboard legalMovesBitboard = 0;

    while (pseudoLegalMovesBitboard)
    {
      int toIndex = Bitboards::popBit(pseudoLegalMovesBitboard);

      MoveFlags flag = quickMakeMove(pieceIndex, toIndex);

      if (!isInCheck(color))
        Bitboards::addBit(legalMovesBitboard, toIndex);

      quickUnmakeMove(pieceIndex, toIndex, flag);
    }

    return legalMovesBitboard;
  }

  Bitboard Board::getAttackingPiecesBitboard(int targetSquare, Piece targetPiece, PieceColor color)
  {
    Bitboard attackingPiecesBitboard = 0;

    if (targetPiece)
    {
      if (Bitboard attackingPawns = MovesLookup::PAWN_CAPTURE_MOVES[color ^ COLOR][targetSquare] & m_bitboards[color | PAWN])
        attackingPiecesBitboard |= attackingPawns;
    }
    else
    {
      Bitboard reverseSinglePawnMoveSquare = MovesLookup::PAWN_REVERSE_SINGLE_MOVES[color][targetSquare];

      if (Bitboard attackingSingleMovePawns = reverseSinglePawnMoveSquare & m_bitboards[color | PAWN])
      {
        attackingPiecesBitboard |= attackingSingleMovePawns;
      }
      else if (!(reverseSinglePawnMoveSquare & m_bitboards[ALL_PIECES]))
      {
        Bitboard reverseDoublePawnMoveSquare = MovesLookup::PAWN_REVERSE_DOUBLE_MOVES[color][targetSquare];

        if (Bitboard attackingDoubleMovePawns = reverseDoublePawnMoveSquare & m_bitboards[color | PAWN])
          attackingPiecesBitboard |= attackingDoubleMovePawns;
      }
    }

    if (Bitboard attackingKnights = MovesLookup::KNIGHT_MOVES[targetSquare] & m_bitboards[color | KNIGHT])
      attackingPiecesBitboard |= attackingKnights;

    if (Bitboard attackingDiagonalSliders = getBishopMoves(targetSquare, color ^ COLOR) & (m_bitboards[color | BISHOP] | m_bitboards[color | QUEEN]))
      attackingPiecesBitboard |= attackingDiagonalSliders;

    if (Bitboard attackingOrthogonalSliders = getRookMoves(targetSquare, color ^ COLOR) & (m_bitboards[color | ROOK] | m_bitboards[color | QUEEN]))
      attackingPiecesBitboard |= attackingOrthogonalSliders;

    return attackingPiecesBitboard;
  }

  std::vector<Move> Board::getLegalMoves(PieceColor color, bool onlyCaptures)
  {
    std::vector<Move> legalMoves;
    legalMoves.reserve(256);

    Bitboard movablePiecesBitboard = 0;
    Bitboard targetSquaresBitboard = 0;

    if (Bitboard attackingKnights = MovesLookup::KNIGHT_MOVES[m_kingIndices[color | KING]] & m_bitboards[(color ^ COLOR) | KNIGHT])
    {
      movablePiecesBitboard = m_bitboards[color | KING];

      if (Bitboards::countBits(attackingKnights) == 1)
        targetSquaresBitboard = attackingKnights;
    }
    else
    {
      Bitboard diagonalMoves = MagicMoveGen::getBishopMoves(m_kingIndices[color | KING], m_bitboards[ALL_PIECES]);
      Bitboard orthogonalMoves = MagicMoveGen::getRookMoves(m_kingIndices[color | KING], m_bitboards[ALL_PIECES]);

      PieceColor opposingColor = color ^ COLOR;

      Bitboard attackingDiagonalSliders = diagonalMoves & (m_bitboards[opposingColor | BISHOP] | m_bitboards[opposingColor | QUEEN]);
      Bitboard attackingOrthogonalSliders = orthogonalMoves & (m_bitboards[opposingColor | ROOK] | m_bitboards[opposingColor | QUEEN]);

      if (!(attackingDiagonalSliders | attackingOrthogonalSliders))
        movablePiecesBitboard = m_bitboards[color];
      else
      {
        movablePiecesBitboard = m_bitboards[color | KING];
        if (attackingDiagonalSliders)
          targetSquaresBitboard = (diagonalMoves & MovesLookup::BISHOP_MASKS[__builtin_ctzll(attackingDiagonalSliders)]) | attackingDiagonalSliders;
        else if (attackingOrthogonalSliders)
          targetSquaresBitboard = (orthogonalMoves & MovesLookup::ROOK_MASKS[__builtin_ctzll(attackingOrthogonalSliders)]) | attackingOrthogonalSliders;
      }
    }

    if (onlyCaptures)
      targetSquaresBitboard &= m_bitboards[color ^ COLOR];

    while (movablePiecesBitboard)
    {
      int pieceIndex = Bitboards::popBit(movablePiecesBitboard);

      Bitboard movesBitboard = getLegalPieceMovesBitboard(pieceIndex, color, onlyCaptures);

      while (movesBitboard)
      {
        int toIndex = Bitboards::popBit(movesBitboard);

        legalMoves.push_back(Move(pieceIndex, toIndex, m_board[pieceIndex], m_board[toIndex], m_castlingRights, m_enPassantFile, m_halfmoveClock, EMPTY));

        Move &move = legalMoves.back();

        if (move.flags & PROMOTION)
        {
          move.promotionPieceType = QUEEN;
          legalMoves.push_back(Move(move, KNIGHT));
          legalMoves.push_back(Move(move, BISHOP));
          legalMoves.push_back(Move(move, ROOK));
        }
      }
    }

    while (targetSquaresBitboard)
    {
      int targetSquare = Bitboards::popBit(targetSquaresBitboard);

      Piece targetPiece = m_board[targetSquare];

      Bitboard attackersBitboard = getAttackingPiecesBitboard(targetSquare, targetPiece, color);

      while (attackersBitboard)
      {
        int attackerIndex = Bitboards::popBit(attackersBitboard);

        MoveFlags flag = quickMakeMove(attackerIndex, targetSquare);

        if (!isInCheck(color))
        {
          legalMoves.push_back(Move(attackerIndex, targetSquare, m_board[attackerIndex], m_board[targetSquare], m_castlingRights, m_enPassantFile, m_halfmoveClock, EMPTY));

          Move &move = legalMoves.back();

          if (flag & PROMOTION)
          {
            move.promotionPieceType = QUEEN;
            legalMoves.push_back(Move(move, KNIGHT));
            legalMoves.push_back(Move(move, BISHOP));
            legalMoves.push_back(Move(move, ROOK));
          }
        }

        quickUnmakeMove(attackerIndex, targetSquare, flag);
      }
    }

    return legalMoves;
  }

  bool Board::isAttacked(int square, PieceColor color)
  {
    PieceColor opposingColor = color ^ COLOR;

    if (MovesLookup::PAWN_CAPTURE_MOVES[opposingColor][square] & m_bitboards[color | PAWN])
      return true;

    else if (MovesLookup::KNIGHT_MOVES[square] & m_bitboards[color | KNIGHT])
      return true;

    else if (MovesLookup::KING_MOVES[square] & m_bitboards[color | KING])
      return true;

    else if (getBishopMoves(square, opposingColor) & (m_bitboards[color | BISHOP] | m_bitboards[color | QUEEN]))
      return true;

    else if (getRookMoves(square, opposingColor) & (m_bitboards[color | ROOK] | m_bitboards[color | QUEEN]))
      return true;

    return false;
  }

  int Board::getGameStatus(PieceColor color)
  {
    if (countRepetitions(m_zobristKey) >= 3)
      return STALEMATE;

    Bitboard friendlyPiecesBitboard = m_bitboards[color];

    while (friendlyPiecesBitboard)
    {
      int pieceIndex = Bitboards::popBit(friendlyPiecesBitboard);

      if (getLegalPieceMovesBitboard(pieceIndex))
        return m_halfmoveClock >= 100 ? STALEMATE : NO_MATE;
    }

    return isInCheck(color) ? LOSE : STALEMATE;
  }

  Move Board::generateMoveFromUCI(std::string uci)
  {
    int from = (uci[0] - 'a') + (8 - uci[1] + '0') * 8;
    int to = (uci[2] - 'a') + (8 - uci[3] + '0') * 8;

    Piece piece = m_board[from];
    Piece capturedPiece = m_board[to];

    PieceType promotionPieceType = EMPTY;

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

    return Move(from, to, piece, capturedPiece, m_castlingRights, m_enPassantFile, m_halfmoveClock, promotionPieceType);
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
      PieceType pieceType = move.piece & TYPE;

      if (pieceType != PAWN)
      {
        pgn += "..NBRQK"[pieceType];

        Bitboard sameTypePieces = m_bitboards[move.piece] & ~(1ULL << move.from);
        Bitboard ambiguousPieces = 0;

        while (sameTypePieces)
        {
          int pieceIndex = Bitboards::popBit(sameTypePieces);

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

    int gameStatus = getGameStatus(m_sideToMove);

    if (isInCheck(m_sideToMove))
      pgn += gameStatus == LOSE ? "#" : "+";

    unmakeMove(move);

    return pgn;
  }

  int Board::countGames(int depth, bool verbose)
  {
    if (depth == 0)
      return 1;

    std::vector<Move> legalMoves = getLegalMoves(m_sideToMove);

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