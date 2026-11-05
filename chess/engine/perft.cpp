#include "perft.hpp"

namespace {

std::vector<move> legal_moves(engine& e) {
  game_history gs = e.get_current_gamestate();
  return e.get_all_legal_moves(&gs);
}

// Plays m, counts below it, takes it back.
uint64_t count_after(engine& e, const move& m, int depth) {
  e.perform_move(m);
  uint64_t nodes = perft(e, depth - 1);
  e.undo_move();
  return nodes;
}

}  // namespace

uint64_t perft(engine& e, int depth) {
  if (depth == 0) return 1;
  std::vector<move> moves = legal_moves(e);

  // Shortcut: at depth 1 every legal move is one leaf, no need to play it.
  // A promotion is already four moves in the list (one per piece).
  if (depth == 1) return moves.size();

  uint64_t nodes = 0;
  for (const move& m : moves) nodes += count_after(e, m, depth);
  return nodes;
}

std::vector<std::pair<std::string, uint64_t>> perft_divide(engine& e,
                                                           int depth) {
  std::vector<std::pair<std::string, uint64_t>> result;
  for (const move& m : legal_moves(e)) {
    result.emplace_back(to_uci(m), count_after(e, m, depth));
  }
  return result;
}
