#include "minimax_ai.hpp"

#include <limits>
#include <utility>

#include "../../engine/scoring.hpp"

minimax_ai::minimax_ai(engine* e, std::string name, int depth)
    : minimax_ai(e, std::move(name), std::random_device{}(), depth) {}

minimax_ai::minimax_ai(engine* e, std::string name, uint32_t seed, int depth)
    : ai_template(e, std::move(name)), depth_(depth), rng_(seed) {}

move minimax_ai::next_move() {
  positions_ = 0;
  game_history gh = eng->get_current_gamestate();
  std::vector<move> best;
  int best_score = std::numeric_limits<int>::min();

  for (const move& m : eng->get_all_legal_moves(&gh)) {
    eng->perform_move(m);
    // The opponent is to move now: its score, negated, is ours.
    const int score = -negamax(depth_ - 1, 1);
    eng->undo_move();  // the engine is the one the GUI shows

    if (score > best_score) {
      best_score = score;
      best = {m};
    } else if (score == best_score) {
      best.push_back(m);
    }
  }
  best_score_ = best_score;
  best_moves_ = best;
  if (best.empty()) return move{};  // no legal move: type is NO_MOVE

  std::uniform_int_distribution<size_t> pick(0, best.size() - 1);
  return best[pick(rng_)];
}

int minimax_ai::negamax(int depth, int ply) {
  ++positions_;
  game_history gh = eng->get_current_gamestate();

  // The game is over in this position.
  if (gh.result == DRAW) return 0;
  if (gh.result != RUNNING) {
    // A win for one side means the side to move has been checkmated: the
    // worst outcome, but the later it happens, the less bad.
    return -kMateScore + ply;
  }

  // Deep enough: rate the position as it stands.
  if (depth == 0) return scoring::score(gh.curr_board, gh.to_move);

  // Otherwise try every move; the opponent's best answer decides.
  int best = std::numeric_limits<int>::min();
  for (const move& m : eng->get_all_legal_moves(&gh)) {
    eng->perform_move(m);
    const int score = -negamax(depth - 1, ply + 1);
    eng->undo_move();
    if (score > best) best = score;
  }
  return best;
}
