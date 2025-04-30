#pragma once

#include <cstdint>
#include <string>

#include "utils/types.hpp"
#include "utils/utils.hpp"

#define NULL_MOVE 0

namespace TungstenChess
{
  typedef uint16_t Move;
  typedef utils::auxiliary_stack<Move> MoveStack;
  typedef MoveStack::dynamic_top_allocation MoveAllocation;

  enum MoveFlags : uint8_t
  {
    NORMAL = 0,
    CAPTURE = 1,
    PAWN_DOUBLE = 2,
    EP_CAPTURE = 4,
    PROMOTION = 8,
    KSIDE_CASTLE = 16,
    QSIDE_CASTLE = 32,
    CASTLE = KSIDE_CASTLE | QSIDE_CASTLE
  };

  enum MoveMasks : uint16_t
  {
    FROM = 0x3F,
    TO = 0xFC0,
    FROM_TO = FROM | TO,
    PROMOTION_PIECE = 0x7000
  };

  enum MovePromotions : uint16_t
  {
    KNIGHT_PROMOTION = KNIGHT << 12,
    BISHOP_PROMOTION = BISHOP << 12,
    ROOK_PROMOTION = ROOK << 12,
    QUEEN_PROMOTION = QUEEN << 12
  };

  namespace Moves
  {
    /**
     * @brief Returns a UCI string representation of the move
     * @param move The move to convert
     */
    std::string getUCI(Move move);

    /**
     * @brief Creates a move
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param promotionPieceType The piece type to promote to (if any)
     * @return The created move
     */
    Move createMove(Square from, Square to, PieceType promotionPieceType = NO_TYPE);

    /**
     * Returns the move flags for a move
     * @param from The square the piece is moving from
     * @param to The square the piece is moving to
     * @param pieceType The type of the piece moving
     * @param capturedPiece The piece being captured (if any)
     */
    uint8_t getMoveFlags(Square from, Square to, PieceType pieceType, Piece capturedPiece);

    /**
     * @brief Checks if a move is a promotion
     * @param to The square the piece is moving to
     * @param pieceType The type of the piece moving (only matters if it's a pawn)
     */
    bool isPromotion(Square to, PieceType pieceType);
  };
}