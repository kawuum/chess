#include "../datastructures/piece.hpp"
#include <stdint.h>

#ifndef MOVE_HPP
#define MOVE_HPP

// TODO: Add type of move (like capturing / promotion) + captured piece? We could probably compute this in move_generation. This would give performance benefits, since we would not need to search for the captured piece in the board when performing a capturing move.
struct move {
    piece mover;
    uint8_t from_x;
    uint8_t from_y;
    uint8_t to_x;
    uint8_t to_y;
    
    bool operator==(const move& other) {
     return this->from_x == other.from_x && this->from_y == other.from_y && this->to_x == other.to_x && this->to_y == other.to_y;   
    }
};

#endif
