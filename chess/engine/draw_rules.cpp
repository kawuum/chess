#include "draw_rules.hpp"

#include "attack_tables.hpp"

namespace draw_rules {

namespace {

// True if either color has a piece of this type: one test of the raw bits
// instead of counting them (insufficient_material() runs after every move).
bool any(const board& b, piece_type t) {
  return (b.get_bit_matrix_for_piece(piece(t, piece_color::WHITE)).get_raw() |
          b.get_bit_matrix_for_piece(piece(t, piece_color::BLACK)).get_raw()) !=
         0;
}

int count(const board& b, piece_type t) {
  return b.get_bit_matrix_for_piece(piece(t, piece_color::WHITE))
             .get_num_bits_set() +
         b.get_bit_matrix_for_piece(piece(t, piece_color::BLACK))
             .get_num_bits_set();
}

// All light squares (h1, a2, ...): x + y is odd. a1 is dark.
constexpr uint64_t light_squares() {
  uint64_t bits = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if ((x + y) % 2 == 1) bits |= square_bit(x, y);
    }
  }
  return bits;
}
constexpr uint64_t kLightSquares = light_squares();

uint64_t all_bishops(const board& b) {
  return b.get_bit_matrix_for_piece(piece(BISHOP, piece_color::WHITE))
             .get_raw() |
         b.get_bit_matrix_for_piece(piece(BISHOP, piece_color::BLACK))
             .get_raw();
}

bool same_position(const game_history& a, const game_history& b) {
  return a.to_move == b.to_move && a.curr_board == b.curr_board &&
         a.castlingrights[0] == b.castlingrights[0] &&
         a.castlingrights[1] == b.castlingrights[1] &&
         a.ep_square == b.ep_square;
}

}  // namespace

bool insufficient_material(const board& b) {
  // With a pawn, a rook or a queen a mate is always possible.
  if (any(b, PAWN) || any(b, ROOK) || any(b, QUEEN)) {
    return false;
  }
  const int knights = count(b, KNIGHT);
  const int bishops = count(b, BISHOP);

  // King against king, or one bishop or one knight on the whole board.
  if (knights + bishops <= 1) return true;

  // Only bishops, all on light squares or all on dark squares: they can
  // never attack a square of the other color, so the king always escapes.
  if (knights == 0) {
    const uint64_t on_light = all_bishops(b) & kLightSquares;
    const uint64_t on_dark = all_bishops(b) & ~kLightSquares;
    return on_light == 0 || on_dark == 0;
  }
  return false;
}

int repetitions(const game_history& gh) {
  // Only positions since the last capture or pawn move can be the same,
  // because these moves cannot be taken back. That is what halfmove_clock
  // counts. With the same side to move, an earlier position is an even
  // number of half moves back: 2, 4, 6, ...
  int count = 1;  // gh itself
  const game_history* earlier = gh.prev.get();
  for (int back = 1; earlier != nullptr && back <= gh.halfmove_clock; ++back) {
    if (back % 2 == 0 && same_position(gh, *earlier)) ++count;
    earlier = earlier->prev.get();
  }
  return count;
}

draw_reason applying_rule(const game_history& gh) {
  if (insufficient_material(gh.curr_board)) return INSUFFICIENT_MATERIAL;
  if (gh.halfmove_clock >= 100) return FIFTY_MOVE_RULE;
  if (repetitions(gh) >= 3) return THREEFOLD_REPETITION;
  return NO_DRAW;
}

}  // namespace draw_rules
