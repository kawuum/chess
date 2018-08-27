#pragma once

#include "../../engine/engine.hpp"
#include "../../engine/move.hpp"
#include "../../datastructures/enums.hpp"
#include "../template/ai_template.hpp"

class random_ai : public ai_template {
  public:
    random_ai(engine* e) : ai_template(e, "Random AI") { srand(time(NULL)); }
    random_ai(engine* e, std::string n) : ai_template(e, n) { srand(time(NULL)); }
    ~random_ai();
    virtual move next_move();
};
