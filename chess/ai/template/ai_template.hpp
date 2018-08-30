#pragma once
#include "../../engine/engine.hpp"
#include "../../engine/move.hpp"

class ai_template {
  public:
    ai_template(engine* e, std::string n) : eng(e), name(n) {}
    virtual ~ai_template() {}
    virtual move next_move() = 0; // TODO: how do we handle promotion moves?
    virtual piece_type get_promotion_piece_type() = 0; // TODO: maybe like this?
    std::string get_name() { return name; }
  protected:
    engine* eng;
    std::string name;
};
