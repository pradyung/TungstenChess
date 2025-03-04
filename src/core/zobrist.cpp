#include "core/zobrist.hpp"

#include <random>

namespace TungstenChess
{
  void Zobrist::init()
  {
    static once<false> initialized;
    if (initialized)
      return;

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

    for (Piece piece : validPieces)
      for (Square square = 0; square < 64; square++)
        pieceKeys[piece, square] = dis(gen);

    for (int i = 0; i < 16; i++)
      castlingKeys[i] = dis(gen);

    for (int i = 0; i < 9; i++)
      enPassantKeys[i] = dis(gen);

    sideKey = dis(gen);

    for (Piece piece1 : validPieces)
      for (Piece piece2 : validPieces)
        for (Square square = 0; square < 64; square++)
          precomputedPieceCombinationKeys[square | (piece1 << 6) | (piece2 << 11)] = pieceKeys[piece1, square] ^ pieceKeys[piece2, square];
  }

  ZobristKey Zobrist::getPieceCombinationKey(Square square, Square before, Square after)
  {
    return precomputedPieceCombinationKeys[square | (before << 6) | (after << 11)];
  }
}