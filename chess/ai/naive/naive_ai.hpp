#pragma once

#include <string>

#include "../template/ai_template.hpp"

/**
 * A naive AI that looks only one half move ahead: it tries every legal move,
 * rates the position after it with evaluate() and plays the best one. A
 * second baseline next to random_ai.
 *
 * Idea of the evaluation (always from the view of the AI's own color):
 * - own pieces give points, enemy pieces take points away;
 * - squares covered by own pieces give points, squares covered by enemy
 *   pieces take points away;
 * - enemy pieces threatened by own pieces give points, own pieces threatened
 *   by enemy pieces take points away.
 *
 * The building blocks are in engine/evaluation.hpp: material(),
 * attacked_squares(), attackers(), threatened_pieces(), threatened_value(),
 * defended_pieces(), piece_value().
 */
class naive_ai : public ai_template {
 public:
  explicit naive_ai(engine* e, std::string name = "Naive AI");

  /**
   * The best move after one half move, or a move with type NO_MOVE if there
   * is none.
   */
  move next_move() override;

  /**
   * Rates the position b from the view of color me: the higher, the better
   * for me. Public, so tests can check it directly.
   */
  int evaluate(const board& b, piece_color me) const;
};
