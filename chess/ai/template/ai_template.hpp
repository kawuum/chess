#pragma once
#include "../../engine/engine.hpp"
#include "../../engine/move.hpp"

class ai_template {
 public:
  ai_template(engine* e, std::string n) : eng(e), name(n) {}
  virtual ~ai_template() {}
  /**
   * The move the AI wants to play in the engine's current position, or a move
   * with type NO_MOVE if there is none. A promotion move already contains the
   * piece the AI chose (move::promotion).
   */
  virtual move next_move() = 0;
  std::string get_name() { return name; }

 protected:
  engine* eng;
  std::string name;
};
