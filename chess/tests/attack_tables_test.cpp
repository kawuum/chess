// The precomputed attack tables against an independent, slow computation for
// all 64 squares, plus a few hand-checked squares. Catches the classic
// bitboard mistakes: wrong bit order and pieces "wrapping" from the h-file to
// the a-file.
#include "../engine/attack_tables.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

namespace {

// Bitboard from square names, e.g. bits({"b3", "c2"}).
uint64_t bits(std::initializer_list<const char*> squares) {
  uint64_t b = 0;
  for (const char* sq : squares) b |= square_bit(sq[0] - 'a', sq[1] - '1');
  return b;
}

int sq(const char* name) { return 8 * (name[1] - '1') + (name[0] - 'a'); }

// The slow way: test every target square with the rules of each piece.
uint64_t slow(int x, int y, bool (*attacks)(int dx, int dy)) {
  uint64_t b = 0;
  for (int ty = 0; ty < 8; ++ty) {
    for (int tx = 0; tx < 8; ++tx) {
      if ((tx != x || ty != y) && attacks(tx - x, ty - y)) {
        b |= square_bit(tx, ty);
      }
    }
  }
  return b;
}

bool knight(int dx, int dy) {
  return std::abs(dx) * std::abs(dy) == 2;  // 1x2 or 2x1
}
bool king(int dx, int dy) { return std::abs(dx) <= 1 && std::abs(dy) <= 1; }
bool white_pawn(int dx, int dy) { return std::abs(dx) == 1 && dy == 1; }
bool black_pawn(int dx, int dy) { return std::abs(dx) == 1 && dy == -1; }
bool rook_line(int dx, int dy) { return dx == 0 || dy == 0; }
bool bishop_line(int dx, int dy) { return std::abs(dx) == std::abs(dy); }

TEST(AttackTables, MatchSlowComputationOnAllSquares) {
  const int white = static_cast<int>(piece_color::WHITE);
  const int black = static_cast<int>(piece_color::BLACK);
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      int s = 8 * y + x;
      std::string where = std::string(1, char('a' + x)) + char('1' + y);
      EXPECT_EQ(kAttacks.knight[s], slow(x, y, knight)) << "knight " << where;
      EXPECT_EQ(kAttacks.king[s], slow(x, y, king)) << "king " << where;
      EXPECT_EQ(kAttacks.pawn[white][s], slow(x, y, white_pawn))
          << "white pawn " << where;
      EXPECT_EQ(kAttacks.pawn[black][s], slow(x, y, black_pawn))
          << "black pawn " << where;
      EXPECT_EQ(kAttacks.rook_lines[s], slow(x, y, rook_line))
          << "rook lines " << where;
      EXPECT_EQ(kAttacks.bishop_lines[s], slow(x, y, bishop_line))
          << "bishop lines " << where;
    }
  }
}

TEST(AttackTables, HandCheckedSquares) {
  EXPECT_EQ(kAttacks.knight[sq("a1")], bits({"b3", "c2"}));
  EXPECT_EQ(kAttacks.knight[sq("e4")],
            bits({"d6", "f6", "c5", "g5", "c3", "g3", "d2", "f2"}));
  EXPECT_EQ(kAttacks.king[sq("h8")], bits({"g8", "g7", "h7"}));
  EXPECT_EQ(kAttacks.pawn[static_cast<int>(piece_color::WHITE)][sq("e4")],
            bits({"d5", "f5"}));
  EXPECT_EQ(kAttacks.pawn[static_cast<int>(piece_color::BLACK)][sq("e4")],
            bits({"d3", "f3"}));
}

TEST(AttackTables, NoWrapAroundTheEdges) {
  // A knight or king on the h-file must never reach the a- or b-file, and
  // vice versa: with shift-based bitboards that is the classic bug.
  const uint64_t a_b_files =
      bits({"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "b1", "b2", "b3",
            "b4", "b5", "b6", "b7", "b8"});
  for (int y = 0; y < 8; ++y) {
    int h = 8 * y + 7;
    EXPECT_EQ(kAttacks.knight[h] & a_b_files, 0u);
    EXPECT_EQ(kAttacks.king[h] & a_b_files, 0u);
  }
}

TEST(AttackTables, ComputedAtCompileTime) {
  // static_assert only compiles if the compiler itself could evaluate the
  // tables: proof that nothing is computed when the program starts.
  static_assert(kAttacks.knight[0] != 0, "tables are constexpr");
  SUCCEED();
}

}  // namespace
