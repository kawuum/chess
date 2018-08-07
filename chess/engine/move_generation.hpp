#include "../datastructures/piece.hpp"
#include <stdint.h>
#include <vector>
#include "move.hpp"
#include "../datastructures/board.hpp"
#include "game_history.hpp"

#ifndef MOVEGENERATION_HPP
#define MOVEGENERATION_HPP

class move_generation {
  public:
    std::vector<move> generate_moves(piece mover, uint8_t from_x, uint8_t from_y, std::shared_ptr<game_history> gh);
    std::vector<move> generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> generate_all_moves(board b, piece_color to_move);
    std::vector<move> generate_all_moves(game_history gh);
  private:
    std::vector<move> bishopmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> rookmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> pawnmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> knightmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> kingmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> queenmoves(piece mover, uint8_t from_x, uint8_t from_y, board b);
    std::vector<move> concat_vectors(std::vector<move> a, std::vector<move> b);
};

#endif
