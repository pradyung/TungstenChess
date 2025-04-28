#include "core/board.hpp"

#include "core/moves_lookup/magic.hpp"

namespace TungstenChess
{
  Board::Board(std::string fen)
  {
    Zobrist::init();
    MagicMoveGen::init();

    resetBoard(fen);
  }

  void Board::resetBoard(std::string fen)
  {
    enum FenParts : uint8_t
    {
      FEN_BOARD = 0,
      FEN_SIDE_TO_MOVE = 1,
      FEN_CASTLING_RIGHTS = 2,
      FEN_EN_PASSANT = 3,
      FEN_HALFMOVE_CLOCK = 4,
      FEN_FULLMOVE_NUMBER = 5
    };

    std::string fenParts[6];

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

    for (Piece i = 0; i < ALL_PIECES + 1; i++)
      m_bitboards[i] = 0;

    m_sideToMove = WHITE;

    Square pieceIndex = 0;
    for (size_t i = 0; i < fenParts[FEN_BOARD].length(); i++)
    {
      if (fen[i] == '/')
        continue;
      else if (isdigit(fen[i]))
      {
        for (int j = 0; j < fen[i] - '0'; j++)
        {
          m_board[pieceIndex] = NO_PIECE;
          pieceIndex++;
        }
      }
      else
      {
        updatePiece(pieceIndex, std::string("PNBRQK..pnbrqk").find(fen[i]) + WHITE_PAWN);

        pieceIndex++;
      }
    }

    m_sideToMove = fenParts[FEN_SIDE_TO_MOVE] == "w" ? WHITE : BLACK;

    if (fenParts[FEN_CASTLING_RIGHTS] != "-")
    {
      for (size_t i = 0; i < fenParts[FEN_CASTLING_RIGHTS].length(); i++)
      {
        switch (fenParts[FEN_CASTLING_RIGHTS][i])
        {
        case 'K':
          m_castlingRights |= WHITE_KINGSIDE;
          break;
        case 'Q':
          m_castlingRights |= WHITE_QUEENSIDE;
          break;
        case 'k':
          m_castlingRights |= BLACK_KINGSIDE;
          break;
        case 'q':
          m_castlingRights |= BLACK_QUEENSIDE;
          break;
        }
      }
    }

    if (fenParts[FEN_EN_PASSANT] != "-")
    {
      m_enPassantFile = fenParts[FEN_EN_PASSANT][0] - 'a';
    }

    m_zobristKey = calculateInitialZobristKey();

    m_positionHistory.push_back(m_zobristKey);

    m_moveHistory.clear();
  }

  ZobristKey Board::calculateInitialZobristKey() const
  {
    ZobristKey zobristKey = 0;

    for (Square i = 0; i < 64; i++)
    {
      if (m_board[i])
      {
        zobristKey ^= Zobrist::pieceKeys[m_board[i], i];
      }
    }

    zobristKey ^= Zobrist::castlingKeys[m_castlingRights];
    zobristKey ^= Zobrist::enPassantKeys[m_enPassantFile];

    if (m_sideToMove == WHITE)
      zobristKey ^= Zobrist::sideKey;

    return zobristKey;
  }
}