#include "Headers/piece.hpp"

namespace Chess
{
  const int Piece::PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

  const int Piece::PIECE_INDICES[12] = {9, 10, 11, 12, 13, 14, 17, 18, 19, 20, 21, 22};

  Piece::Piece()
  {
    piece = EMPTY;
  }

  Piece::Piece(int type, int color)
  {
    int piece = type | color;
  }

  Piece::Piece(char fen)
  {
    int pieceColor = isupper(fen) ? WHITE : BLACK;
    std::string pieceChars = ".pnbrqk";

    int pieceType = pieceChars.find(tolower(fen));

    piece = pieceType | pieceColor;
  }

  Piece::Piece(int piece)
  {
    this->piece = piece;
  }

  bool Piece::isEmpty()
  {
    return piece == EMPTY;
  }

  int Piece::getPieceType()
  {
    return piece & 7;
  }

  int Piece::getPieceColor()
  {
    return piece & 24;
  }

  char Piece::getPieceChar()
  {
    std::string pieceChars = ".pnbrqk";
    char pieceChar = pieceChars[getPieceType()];
    return getPieceColor() == WHITE ? toupper(pieceChar) : tolower(pieceChar);
  }

  int *Piece::generateIntArray(Piece board[64])
  {
    int *intBoard = new int[64];

    for (int i = 0; i < 64; i++)
    {
      intBoard[i] = board[i].piece;
    }

    return intBoard;
  }
}