#include "Headers/zobrist.hpp"

namespace Chess
{
  const int Zobrist::PIECE_INDICES[12] = {9, 10, 11, 12, 13, 14, 17, 18, 19, 20, 21, 22};

  Zobrist::Zobrist()
  {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

    for (int i = 0; i < 64; i++)
    {
      for (int j = 0; j < 12; j++)
      {
        pieceKeys[i][PIECE_INDICES[j]] = dis(gen);
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
}