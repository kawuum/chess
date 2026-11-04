#include "perft.hpp"

namespace {

const piece_type kPromotionTypes[] = {QUEEN, ROOK, BISHOP, KNIGHT};

bool is_promotion(const move& m) {
  return m.type == PROMOTION || m.type == CAPTURING_PROMOTION;
}

std::vector<move> legal_moves(engine& e) {
  game_history gs = e.get_current_gamestate();
  return e.get_all_legal_moves(&gs);
}

// Plays m (with the given promotion piece), counts below it, takes it back.
uint64_t count_after(engine& e, move m, piece_type promotion, int depth) {
  if (is_promotion(m)) {
    piece p(promotion, e.get_color_to_move());
    e.perform_move(m, p);
  } else {
    e.perform_move(m);
  }
  uint64_t nodes = perft(e, depth - 1);
  e.undo_move();
  return nodes;
}

}  // namespace

uint64_t perft(engine& e, int depth) {
  if (depth == 0) return 1;
  std::vector<move> moves = legal_moves(e);

  // Shortcut: at depth 1 every legal move is one leaf, no need to play it.
  if (depth == 1) {
    uint64_t nodes = 0;
    for (const move& m : moves) nodes += is_promotion(m) ? 4 : 1;
    return nodes;
  }

  uint64_t nodes = 0;
  for (const move& m : moves) {
    if (is_promotion(m)) {
      for (piece_type t : kPromotionTypes) nodes += count_after(e, m, t, depth);
    } else {
      nodes += count_after(e, m, QUEEN, depth);
    }
  }
  return nodes;
}

std::vector<std::pair<std::string, uint64_t>> perft_divide(engine& e,
                                                           int depth) {
  std::vector<std::pair<std::string, uint64_t>> result;
  for (const move& m : legal_moves(e)) {
    if (is_promotion(m)) {
      for (piece_type t : kPromotionTypes) {
        result.emplace_back(to_uci(m, t), count_after(e, m, t, depth));
      }
    } else {
      result.emplace_back(to_uci(m), count_after(e, m, QUEEN, depth));
    }
  }
  return result;
}

std::string to_uci(const move& m, piece_type promotion) {
  std::string s;
  s += static_cast<char>('a' + m.from_x);
  s += static_cast<char>('1' + m.from_y);
  s += static_cast<char>('a' + m.to_x);
  s += static_cast<char>('1' + m.to_y);
  if (is_promotion(m)) s += "kqbnrp"[promotion];  // same order as piece_type
  return s;
}
