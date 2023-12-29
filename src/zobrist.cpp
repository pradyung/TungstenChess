#include "Headers/zobrist.hpp"

namespace Chess
{
  Zobrist::Zobrist()
  {
    pieceKeys = new ZobristKey *[64];
    for (int i = 0; i < 64; i++)
    {
      pieceKeys[i] = new ZobristKey[PIECE_NUMBER];
    }

    castlingKeys = new ZobristKey[16];

    enPassantKeys = new ZobristKey[9];

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

    for (int i = 0; i < 64; i++)
    {
      for (int j = 0; j < 12; j++)
      {
        pieceKeys[i][Piece::PIECE_INDICES[j]] = dis(gen);
      }
    }

    for (int i = 0; i < 16; i++)
    {
      castlingKeys[i] = dis(gen);
    }

    for (int i = 0; i < 9; i++)
    {
      enPassantKeys[i] = dis(gen);
    }

    sideKey = dis(gen);
  }

  Zobrist::~Zobrist()
  {
    for (int i = 0; i < 64; i++)
    {
      delete[] pieceKeys[i];
    }

    delete[] pieceKeys;
    delete[] castlingKeys;
    delete[] enPassantKeys;
  }
}