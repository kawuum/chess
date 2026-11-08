#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "../template/ai_template.hpp"

/**
 * The same search as minimax_ai, with the same result, but much faster:
 * alpha-beta pruning skips moves that cannot change the result.
 *
 * The idea: while searching, two bounds are carried along.
 * - alpha: the score the side to move is already sure of (from a move
 *   searched earlier). A move that scores less is not interesting.
 * - beta: the score the opponent will not allow. If a move reaches beta, the
 *   opponent would never have played into this position, so the remaining
 *   moves here need not be searched at all (a "cutoff").
 * The score of the best move stays exactly the minimax score; only the
 * number of positions shrinks.
 *
 * Move ordering: alpha-beta prunes most when good moves come first, so
 * promotions and captures (most valuable victim first) are searched before
 * quiet moves.
 *
 * Equally good moves: at the root every move is searched with the window
 * (best - 1, infinity), so a move as good as the best one gets its exact
 * score. The set of best moves is therefore the same as with minimax_ai, and
 * one of them is picked at random.
 */
class alphabeta_ai : public ai_template {
 public:
  static constexpr int kMateScore = 100000;  // same scale as minimax_ai
  static constexpr int kDefaultDepth = 4;
  // Larger than any score, also a mate: the "no bound yet" value, and
  // -kInfinity is best_score() when there is no legal move.
  static constexpr int kInfinity = 1000000;

  /**
   * Seeds the random generator from std::random_device and searches
   * kDefaultDepth half moves. A depth is only accepted together with a
   * seed (constructor below): an overload (name, int depth) would win
   * over (name, uint32_t seed) for a plain number like 1.
   */
  explicit alphabeta_ai(engine* e, std::string name = "Alpha-Beta AI");
  /**
   * Fixed seed: the same position always gives the same move (tests).
   * sort_moves = false searches the moves in the generator's order (only
   * for tests: the result is the same, it just takes longer).
   */
  alphabeta_ai(engine* e, std::string name, uint32_t seed,
               int depth = kDefaultDepth, bool sort_moves = true);

  /**
   * The best move after searching depth half moves, or a move with type
   * NO_MOVE if there is none. Among equally good moves one is picked at
   * random.
   */
  move next_move() override;

  /** How many half moves the search looks ahead. */
  int depth() const { return depth_; }
  /** How many positions the last next_move() looked at. */
  long long positions_searched() const { return positions_; }
  /** Score of the best move of the last next_move(), for the AI's side. */
  int best_score() const { return best_score_; }
  /** All equally best moves of the last next_move() (one was played). */
  const std::vector<move>& best_moves() const { return best_moves_; }

 private:
  // The score of the engine's current position for the side to move,
  // looking depth half moves ahead, if it lies between alpha and beta.
  // Otherwise the result is only a bound: alpha if the true score is at
  // most alpha, beta if it is at least beta.
  int alphabeta(int depth, int ply, int alpha, int beta);

  // The legal moves, the most promising first (see move_priority() in the
  // .cpp): good moves early make the cutoffs come early.
  std::vector<move> moves_in_search_order(const game_history& gh);

  int depth_;
  bool sort_moves_;
  std::mt19937 rng_;
  long long positions_ = 0;
  int best_score_ = 0;
  std::vector<move> best_moves_;
};
