#include "evaluation.hpp"

#include "../datastructures/bits.hpp"
#include "attack_tables.hpp"

namespace evaluation {

namespace {

constexpr piece_type kTypes[] = {KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN};
constexpr int kStraight[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
constexpr int kDiagonal[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

constexpr bool on_board(int x, int y) {
  return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}

uint64_t pieces(const board& b, piece_type t, piece_color c) {
  return b.get_bit_matrix_for_piece(piece(t, c)).get_raw();
}

piece_color opponent(piece_color c) {
  return c == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
}

// Calls f(x, y) for every set bit. bits::leading_zeros counts the zero bits
// above the highest set bit; in the bit order of bit_matrix (a1 = highest
// bit) that is directly the square index 8 * y + x.
template <typename F>
void for_each_square(uint64_t bits, F f) {
  while (bits != 0) {
    int i = bits::leading_zeros(bits);
    f(i % 8, i / 8);
    bits &= ~((uint64_t{1} << 63) >> i);  // clear this bit
  }
}

// Squares covered along one direction: every square up to and including the
// first piece.
uint64_t ray(const board& b, int x, int y, int dx, int dy) {
  uint64_t bits = 0;
  for (x += dx, y += dy; on_board(x, y); x += dx, y += dy) {
    bits |= square_bit(x, y);
    if (b.get_piece(x, y).is_valid()) break;
  }
  return bits;
}

template <int N>
uint64_t rays(const board& b, int x, int y, const int (&d)[N][2]) {
  uint64_t bits = 0;
  for (const auto& dir : d) bits |= ray(b, x, y, dir[0], dir[1]);
  return bits;
}

// True if the first piece from (x, y) in direction (dx, dy) belongs to c and
// has type t1 or t2.
bool first_piece_is(const board& b, int x, int y, int dx, int dy, piece_color c,
                    piece_type t1, piece_type t2) {
  for (x += dx, y += dy; on_board(x, y); x += dx, y += dy) {
    piece p = b.get_piece(x, y);
    if (!p.is_valid()) continue;
    return p.get_piece_color() == c &&
           (p.get_piece_type() == t1 || p.get_piece_type() == t2);
  }
  return false;
}

}  // namespace

int material(const board& b, piece_color c) {
  int sum = 0;
  for (piece_type t : kTypes) {
    sum += piece_value(t) *
           b.get_bit_matrix_for_piece(piece(t, c)).get_num_bits_set();
  }
  return sum;
}

bit_matrix attacked_squares(const board& b, piece_color c) {
  const int color = static_cast<int>(c);
  uint64_t covered = 0;
  // Knights, kings and pawns: one table lookup per piece.
  for_each_square(pieces(b, KNIGHT, c),
                  [&](int x, int y) { covered |= kAttacks.knight[8 * y + x]; });
  for_each_square(pieces(b, KING, c),
                  [&](int x, int y) { covered |= kAttacks.king[8 * y + x]; });
  for_each_square(pieces(b, PAWN, c), [&](int x, int y) {
    covered |= kAttacks.pawn[color][8 * y + x];
  });
  // Sliding pieces: walk the rays up to the first piece.
  const uint64_t queens = pieces(b, QUEEN, c);
  for_each_square(pieces(b, ROOK, c) | queens,
                  [&](int x, int y) { covered |= rays(b, x, y, kStraight); });
  for_each_square(pieces(b, BISHOP, c) | queens,
                  [&](int x, int y) { covered |= rays(b, x, y, kDiagonal); });
  return bit_matrix(covered);
}

int attackers(const board& b, int x, int y, piece_color c) {
  // Same idea as move_generation::is_square_attacked(), but counting: look
  // outwards from the square. A knight on t covers s exactly if a knight on
  // s would cover t. Pawns capture in one direction only, so the table of
  // the other color is used.
  const int s = 8 * y + x;
  const int other = static_cast<int>(opponent(c));
  int count = bits::popcount(kAttacks.knight[s] & pieces(b, KNIGHT, c)) +
              bits::popcount(kAttacks.king[s] & pieces(b, KING, c)) +
              bits::popcount(kAttacks.pawn[other][s] & pieces(b, PAWN, c));
  for (const auto& d : kStraight) {
    if (first_piece_is(b, x, y, d[0], d[1], c, ROOK, QUEEN)) ++count;
  }
  for (const auto& d : kDiagonal) {
    if (first_piece_is(b, x, y, d[0], d[1], c, BISHOP, QUEEN)) ++count;
  }
  return count;
}

bit_matrix threatened_pieces(const board& b, piece_color by) {
  return attacked_squares(b, by) & b.get_bit_matrix_for_color(opponent(by));
}

int threatened_value(const board& b, piece_color by) {
  int sum = 0;
  for_each_square(threatened_pieces(b, by).get_raw(), [&](int x, int y) {
    sum += piece_value(b.get_piece(x, y).get_piece_type());
  });
  return sum;
}

bit_matrix defended_pieces(const board& b, piece_color c) {
  // A piece never covers its own square, so every hit is covered by another
  // piece.
  return attacked_squares(b, c) & b.get_bit_matrix_for_color(c);
}

}  // namespace evaluation
