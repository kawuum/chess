#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "../template/ai_template.hpp"

/**
 * A naive AI that looks only one half move ahead: it tries every legal move,
 * rates the position after it and plays the best one. A second baseline next
 * to random_ai. Written to be easy to read, not to be fast.
 *
 * Rating of a position, always from the view of the AI's own color:
 * - own pieces give points, enemy pieces take points away;
 * - squares covered by own pieces give points, squares covered by enemy
 *   pieces take points away;
 * - enemy pieces threatened by own pieces give points, own pieces threatened
 *   by enemy pieces take points away.
 * A move that checkmates beats everything, a move that ends the game in a
 * draw (stalemate) rates 0, like an even position.
 *
 * Several equally good moves: the AI picks one of them at random, so two
 * games are not always the same.
 */
class naive_ai : public ai_template {
 public:
  /** Seeds the random generator from std::random_device. */
  explicit naive_ai(engine* e, std::string name = "Naive AI");
  /** Fixed seed: the same position always gives the same move (tests). */
  naive_ai(engine* e, std::string name, uint32_t seed);

  /**
   * The best move after one half move, or a move with type NO_MOVE if there
   * is none.
   */
  move next_move() override;

  /**
   * Rates the position b from the view of color me: the higher, the better
   * for me; 0 means even. Good for White by n is bad for Black by n.
   */
  int evaluate(const board& b, piece_color me) const;

 private:
  /** Plays m on the engine, rates the position for me, takes m back. */
  int rate_move(const move& m, piece_color me);

  /** All moves that share the highest rating. */
  std::vector<move> best_moves(piece_color me);

  std::mt19937 rng_;
};
