#include "core/zobrist.hpp"

#include <random>

namespace TungstenChess
{
  void Zobrist::init()
  {
    static utils::once<false> initialized;
    if (initialized)
      return;

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<ZobristKey> dis(0, 0xFFFFFFFFFFFFFFFF);

    for (Piece piece : validPieces)
      for (Square square = 0; square < 64; square++)
        pieceKeys.at(piece, square) = dis(gen);

    for (int i = 0; i < 16; i++)
      castlingKeys[i] = dis(gen);

    for (int i = 0; i < 9; i++)
      enPassantKeys[i] = dis(gen);

    sideKey = dis(gen);

    for (Square square = 0; square < 64; square++)
      for (Piece piece1 : validPieces)
        for (Piece piece2 : validPieces)
          precomputedPieceCombinationKeys[square | (piece1 << 6) | (piece2 << 11)] = pieceKeys.at(piece1, square) ^ pieceKeys.at(piece2, square);
  }

  ZobristKey Zobrist::getPieceCombinationKey(Square square, Square before, Square after)
  {
    return precomputedPieceCombinationKeys[square | (before << 6) | (after << 11)];
  }
}