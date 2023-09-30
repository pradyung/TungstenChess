#include "Headers/piece.hpp"

namespace Chess
{
  const int Piece::PIECE_VALUES[7] = {0, 100, 300, 300, 500, 900, 0};

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
}