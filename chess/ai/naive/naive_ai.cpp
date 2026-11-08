#include "naive_ai.hpp"

#include <limits>
#include <utility>

#include "../../engine/evaluation.hpp"

namespace {

// Weights of the three parts of the rating. A pawn of material is worth as
// much as ten covered squares: otherwise the AI would give away a knight
// just to cover a few more squares. A threat counts less than material,
// because a threatened piece is not lost yet.
constexpr int kMaterialWeight = 10;  // per pawn of material
constexpr int kSquareWeight = 1;     // per covered square
constexpr int kThreatWeight = 3;     // per pawn of threatened material

// Rating of a move that checkmates: higher than any position can reach.
constexpr int kWinRating = 1000000;

piece_color opponent(piece_color c) {
  return c == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
}

game_result win_for(piece_color c) {
  return c == piece_color::WHITE ? WHITE_WIN : BLACK_WIN;
}

int covered_squares(const board& b, piece_color c) {
  return evaluation::attacked_squares(b, c).get_num_bits_set();
}

}  // namespace

naive_ai::naive_ai(engine* e, std::string name)
    : naive_ai(e, std::move(name), std::random_device{}()) {}

naive_ai::naive_ai(engine* e, std::string name, uint32_t seed)
    : ai_template(e, std::move(name)), rng_(seed) {}

move naive_ai::next_move() {
  std::vector<move> candidates = best_moves(eng->get_color_to_move());
  if (candidates.empty()) return move{};  // all fields zero: type is NO_MOVE

  std::uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
  return candidates[pick(rng_)];
}

std::vector<move> naive_ai::best_moves(piece_color me) {
  std::vector<move> best;
  int best_rating = std::numeric_limits<int>::min();

  for (const move& m : eng->get_all_legal_moves()) {
    int rating = rate_move(m, me);
    if (rating > best_rating) {
      // A new best rating: forget the moves found so far.
      best_rating = rating;
      best = {m};
    } else if (rating == best_rating) {
      best.push_back(m);
    }
  }
  return best;
}

int naive_ai::rate_move(const move& m, piece_color me) {
  eng->perform_move(m);

  int rating;
  game_result result = eng->current_state().result;
  if (result == win_for(me)) {
    rating = kWinRating;  // checkmate
  } else if (result == DRAW) {
    rating = 0;  // stalemate: neither side wins
  } else {
    rating = evaluate(eng->get_current_board(), me);
  }

  eng->undo_move();  // the engine is the one the GUI shows: leave it as it was
  return rating;
}

int naive_ai::evaluate(const board& b, piece_color me) const {
  const piece_color enemy = opponent(me);

  const int material =
      evaluation::material(b, me) - evaluation::material(b, enemy);
  const int squares = covered_squares(b, me) - covered_squares(b, enemy);
  const int threats = evaluation::threatened_value(b, me) -
                      evaluation::threatened_value(b, enemy);

  return kMaterialWeight * material + kSquareWeight * squares +
         kThreatWeight * threats;
}
