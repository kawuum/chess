#pragma once

#include <stdint.h>

#include "../datastructures/enums.hpp"

/**
 * Precomputed attack bitboards, built by the compiler (constexpr).
 *
 * For every square s (index 8 * y + x, bit order of bit_matrix: a1 is the
 * highest bit) the tables hold the squares that a piece standing on s
 * attacks. Knights, kings and pawns cannot be blocked, so their attacks only
 * depend on the square. Rooks, bishops and queens can be blocked; for them the
 * tables only hold all squares on the same lines, which is used as a quick
 * pre-check.
 *
 * Each table has 64 entries of 8 bytes, 3 KB in total: they stay in the CPU's
 * fastest cache.
 */
struct attack_tables {
  uint64_t knight[64];
  uint64_t king[64];
  uint64_t pawn[2][64];       // [color][square]: diagonally forwards
  uint64_t rook_lines[64];    // same rank and file, without the square itself
  uint64_t bishop_lines[64];  // both diagonals, without the square itself
};

/** The bit of square (x, y) in the bit order of bit_matrix. */
constexpr uint64_t square_bit(int x, int y) {
  return (uint64_t{1} << 63) >> (8 * y + x);
}

namespace attack_tables_detail {

constexpr bool on_board(int x, int y) {
  return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}

// All squares reachable from (x, y) with the given single steps.
template <int N>
constexpr uint64_t steps(int x, int y, const int (&d)[N][2]) {
  uint64_t bits = 0;
  for (int i = 0; i < N; ++i) {
    int nx = x + d[i][0], ny = y + d[i][1];
    if (on_board(nx, ny)) bits |= square_bit(nx, ny);
  }
  return bits;
}

// All squares from (x, y) to the edge in the given directions.
template <int N>
constexpr uint64_t lines(int x, int y, const int (&d)[N][2]) {
  uint64_t bits = 0;
  for (int i = 0; i < N; ++i) {
    for (int nx = x + d[i][0], ny = y + d[i][1]; on_board(nx, ny);
         nx += d[i][0], ny += d[i][1]) {
      bits |= square_bit(nx, ny);
    }
  }
  return bits;
}

constexpr int kKnight[8][2] = {{1, 2},   {2, 1},   {2, -1}, {1, -2},
                               {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
constexpr int kKing[8][2] = {{1, 0},  {1, 1},   {0, 1},  {-1, 1},
                             {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
constexpr int kWhitePawn[2][2] = {{-1, 1}, {1, 1}};    // captures upwards
constexpr int kBlackPawn[2][2] = {{-1, -1}, {1, -1}};  // captures downwards
constexpr int kStraight[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
constexpr int kDiagonal[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

constexpr attack_tables make_attack_tables() {
  attack_tables t{};
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      int s = 8 * y + x;
      t.knight[s] = steps(x, y, kKnight);
      t.king[s] = steps(x, y, kKing);
      t.pawn[static_cast<int>(piece_color::WHITE)][s] = steps(x, y, kWhitePawn);
      t.pawn[static_cast<int>(piece_color::BLACK)][s] = steps(x, y, kBlackPawn);
      t.rook_lines[s] = lines(x, y, kStraight);
      t.bishop_lines[s] = lines(x, y, kDiagonal);
    }
  }
  return t;
}

}  // namespace attack_tables_detail

/** The tables, computed at compile time. */
inline constexpr attack_tables kAttacks =
    attack_tables_detail::make_attack_tables();
