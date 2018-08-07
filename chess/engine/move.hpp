#include "../datastructures/piece.hpp"
#include <stdint.h>

#ifndef MOVE_HPP
#define MOVE_HPP

struct move {
    piece mover;
    uint8_t from_x;
    uint8_t from_y;
    uint8_t to_x;
    uint8_t to_y;
};

#endif
