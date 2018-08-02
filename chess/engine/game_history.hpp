#include "../datastructures/board.hpp"
#include "../datastructures/enums.hpp"
#include <memory>

#ifndef GAMEHISTORY_HPP
#define GAMEHISTORY_HPP

struct game_history {
    int num_halfmoves;
    board curr_board;
    std::shared_ptr<game_history> prev;
    piece_color to_move;
};

#endif
