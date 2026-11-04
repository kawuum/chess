#pragma once

#include <cstdint>
#include <random>
#include <string>

#include "../template/ai_template.hpp"

/**
 * Plays a random legal move. Every instance has its own random generator
 * (std::mt19937), so two AIs never influence each other.
 */
class random_ai : public ai_template {
 public:
  /**
   * Seeds the generator from std::random_device: a different game every time.
   */
  explicit random_ai(engine* e, std::string name = "Random AI");
  /**
   * Fixed seed: in the same position the AI always picks the same move, so a
   * whole game can be repeated exactly (useful for tests).
   */
  random_ai(engine* e, std::string name, uint32_t seed);

  move next_move() override;
  piece_type get_promotion_piece_type() override;

 private:
  std::mt19937 rng_;
};
