#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "../template/ai_template.hpp"

/**
 * An AI that looks several half moves ahead with the minimax idea: it
 * assumes that the opponent always answers with the move that is best for
 * the opponent, and picks the move after which its worst case is best.
 *
 * Written as "negamax": the score of a position for one side is minus its
 * score for the other side, so one function serves both sides. At the end
 * of the search (depth 0) the position is rated with scoring::score().
 *
 * Checkmate counts as kMateScore minus the half moves until the mate: a
 * quick mate is better than a slow one, and being mated later is less bad
 * than being mated soon. Stalemate and other draws count 0.
 *
 * Every position is visited (no alpha-beta yet, see roadmap 5.3), so the
 * depth must stay small: 3 half moves look at roughly 30^3 positions.
 */
class minimax_ai : public ai_template {
 public:
  static constexpr int kMateScore = 100000;
  static constexpr int kDefaultDepth = 3;

  /** Seeds the random generator from std::random_device. */
  explicit minimax_ai(engine* e, std::string name = "Minimax AI",
                      int depth = kDefaultDepth);
  /** Fixed seed: the same position always gives the same move (tests). */
  minimax_ai(engine* e, std::string name, uint32_t seed,
             int depth = kDefaultDepth);

  /**
   * The best move after searching depth half moves, or a move with type
   * NO_MOVE if there is none. Among equally good moves one is picked at
   * random.
   */
  move next_move() override;

  /** How many positions the last next_move() looked at. */
  long long positions_searched() const { return positions_; }

 private:
  // The score of the engine's current position for the side to move,
  // looking depth half moves ahead. ply = half moves from the root (for
  // the mate distance).
  int negamax(int depth, int ply);

  int depth_;
  std::mt19937 rng_;
  long long positions_ = 0;
};
