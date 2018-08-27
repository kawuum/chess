#pragma once
#include "../../engine/engine.hpp"
#include "../../engine/move.hpp"

class ai_template {
  public:
    ai_template(engine* e, std::string n) : eng(e), name(n) {}
    virtual ~ai_template() {}
    virtual move next_move() = 0; // TODO: how do we handle promotion moves?
    std::string get_name() { return name; }
  protected:
    engine* eng;
    std::string name;
};
