#include "random_ai.hpp"

#include <utility>
#include <vector>

random_ai::random_ai(engine* e, std::string name)
    : random_ai(e, std::move(name), std::random_device{}()) {}

random_ai::random_ai(engine* e, std::string name, uint32_t seed)
    : ai_template(e, std::move(name)), rng_(seed) {}

move random_ai::next_move() {
  game_history gh = eng->get_current_gamestate();
  std::vector<move> moves = eng->get_all_legal_moves(&gh);
  if (moves.empty()) return move{};  // all fields zero: type is NO_MOVE
  // Uniform over all moves; unlike rand() % n it has no bias towards the
  // first moves of the list.
  std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
  return moves[pick(rng_)];
}

piece_type random_ai::get_promotion_piece_type() { return QUEEN; }
