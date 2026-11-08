#include "alphabeta_ai.hpp"

#include <algorithm>
#include <utility>

#include "../../engine/scoring.hpp"

namespace {

// How promising a move looks before it is searched; higher = earlier.
// Promotions first, then captures by "most valuable victim, least valuable
// attacker" (MVV-LVA): taking a queen comes before taking a pawn, and with
// the same victim a pawn as attacker before a queen (the queen could be
// taken back). Quiet moves get 0 and keep the generator's order.
int move_priority(const move& m, const board& b) {
  int priority = 0;
  if (is_promotion(m)) priority += 10 * scoring::centipawns(m.promotion);
  const piece victim = b.get_piece(m.to_x, m.to_y);
  const bool capture = victim.is_valid() || m.type == ENPASSANT;
  if (capture) {
    const int victim_value = victim.is_valid()
                                 ? scoring::centipawns(victim.get_piece_type())
                                 : scoring::centipawns(PAWN);  // en passant
    // The king has no material value; as an attacker it counts as the most
    // valuable piece (it should take last).
    const int attacker_value =
        m.mover.get_piece_type() == KING
            ? 2000
            : scoring::centipawns(m.mover.get_piece_type());
    priority += 10 * victim_value - attacker_value + 10000;
  }
  return priority;
}

}  // namespace

alphabeta_ai::alphabeta_ai(engine* e, std::string name)
    : alphabeta_ai(e, std::move(name), std::random_device{}()) {}

alphabeta_ai::alphabeta_ai(engine* e, std::string name, uint32_t seed,
                           int depth, bool sort_moves)
    : ai_template(e, std::move(name)),
      depth_(std::max(depth, 1)),  // the search stops at depth 0
      sort_moves_(sort_moves),
      rng_(seed) {}

std::vector<move> alphabeta_ai::moves_in_search_order(game_history& gh) {
  std::vector<move> moves = eng->get_all_legal_moves(&gh);
  if (sort_moves_) {
    // stable_sort: moves with the same priority keep their order.
    std::stable_sort(moves.begin(), moves.end(),
                     [&gh](const move& a, const move& b) {
                       return move_priority(a, gh.curr_board) >
                              move_priority(b, gh.curr_board);
                     });
  }
  return moves;
}

move alphabeta_ai::next_move() {
  positions_ = 0;
  game_history gh = eng->get_current_gamestate();
  std::vector<move> best;
  int best_score = -kInfinity;

  for (const move& m : moves_in_search_order(gh)) {
    eng->perform_move(m);
    // Window (best_score - 1, infinity) for this move, seen from the
    // opponent: (-infinity, -(best_score - 1)). A move as good as the best
    // one so far gets its exact score; a worse one only "best_score - 1".
    const int score = -alphabeta(depth_ - 1, 1, -kInfinity, -(best_score - 1));
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

int alphabeta_ai::alphabeta(int depth, int ply, int alpha, int beta) {
  ++positions_;
  game_history gh = eng->get_current_gamestate();

  // The game is over in this position (see minimax_ai).
  if (gh.result == DRAW) return 0;
  if (gh.result != RUNNING) return -kMateScore + ply;

  if (depth == 0) return scoring::score(gh.curr_board, gh.to_move);

  for (const move& m : moves_in_search_order(gh)) {
    eng->perform_move(m);
    // The opponent's window is ours, turned around: what is "already sure"
    // for us is the most the opponent can get, and the other way round.
    const int score = -alphabeta(depth - 1, ply + 1, -beta, -alpha);
    eng->undo_move();

    if (score >= beta) {
      // Too good: the opponent will avoid this position. The remaining
      // moves cannot change that, so stop here (cutoff).
      return beta;
    }
    if (score > alpha) alpha = score;  // a new best move in this position
  }
  return alpha;
}
