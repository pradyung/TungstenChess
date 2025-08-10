#pragma once

namespace TungstenChess
{
  constexpr std::array<int, 7> PIECE_VALUES = { 0, 100, 300, 300, 500, 900, 0 };

  template <typename T, std::size_t N, std::size_t... I>
  constexpr std::array<T, N> reverse_impl(const std::array<T, N>& a, std::index_sequence<I...>)
  {
    return { a[N - 1 - I]... };
  }

  template <typename T, std::size_t N>
  constexpr std::array<T, N> reverse(const std::array<T, N>& a)
  {
    return reverse_impl(a, std::make_index_sequence<N>{});
  }

  constexpr std::array<int, 64> WHITE_PAWN_EVAL_TABLE = { 0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30, 20, 10, 10, 5, 5, 10, 30, 30, 10, 5, 5, 0, 0, 0, 25, 25, 0, 0, 0, 5, -5, -10, 0, 0, -10, -5, 5, 5, 10, 10, -20, -20, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0 };
  constexpr std::array<int, 64> WHITE_KNIGHT_EVAL_TABLE = { -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 0, 0, 0, -20, -40, -30, 0, 10, 15, 15, 10, 0, -30, -30, 5, 15, 20, 20, 15, 5, -30, -30, 0, 15, 20, 20, 15, 0, -30, -30, 5, 10, 15, 15, 10, 5, -30, -40, -20, 0, 5, 5, 0, -20, -40, -50, -40, -30, -30, -30, -30, -40, -50 };
  constexpr std::array<int, 64> WHITE_BISHOP_EVAL_TABLE = { -20, -10, -10, -10, -10, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 5, 5, 10, 10, 5, 5, -10, -10, 0, 10, 10, 10, 10, 0, -10, -10, 10, 10, 10, 10, 10, 10, -10, -10, 5, 0, 0, 0, 0, 5, -10, -20, -10, -10, -10, -10, -10, -10, -20 };
  constexpr std::array<int, 64> WHITE_ROOK_EVAL_TABLE = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0 };
  constexpr std::array<int, 64> WHITE_QUEEN_EVAL_TABLE = { -20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -5, 0, 5, 5, 5, 5, 0, -5, 0, 0, 5, 5, 5, 5, 0, -5, -10, 5, 5, 5, 5, 5, 0, -10, -10, 0, 5, 0, 0, 0, 0, -10, -20, -10, -10, -5, -5, -10, -10, -20 };

  constexpr std::array<int, 64> KING_EVAL_TABLE = { -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20, -20, -20, -20, -10, 20, 20, 0, 0, 0, 0, 20, 20, 20, 30, 10, 0, 0, 10, 30, 20 };
  constexpr std::array<int, 64> KING_ENDGAME_EVAL_TABLE = { -50, -30, -30, -30, -30, -30, -30, -50, -30, -30, 0, 0, 0, 0, -30, -30, -30, -10, 20, 30, 30, 20, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 20, 30, 30, 20, -10, -30, -30, -20, -10, 0, 0, -10, -20, -30, -50, -40, -30, -20, -20, -30, -40, -50 };
  constexpr std::array<int, 16> KINGS_DISTANCE_EVAL_TABLE = { 0, 0, 70, 70, 50, 30, 20, 0, -10, -20, -30, -40, -50, -60, -70, -70 };

  constexpr std::array<int, 64> PIECE_EVAL_TABLES[PIECE_NUMBER] = {
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    { { 0 } },
    WHITE_PAWN_EVAL_TABLE,
    WHITE_KNIGHT_EVAL_TABLE,
    WHITE_BISHOP_EVAL_TABLE,
    WHITE_ROOK_EVAL_TABLE,
    WHITE_QUEEN_EVAL_TABLE,
    { { 0 } },
    { { 0 } },
    { { 0 } },
    reverse(WHITE_PAWN_EVAL_TABLE),
    reverse(WHITE_KNIGHT_EVAL_TABLE),
    reverse(WHITE_BISHOP_EVAL_TABLE),
    reverse(WHITE_ROOK_EVAL_TABLE),
    reverse(WHITE_QUEEN_EVAL_TABLE),
    { { 0 } }
  };
}