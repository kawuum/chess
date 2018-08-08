#include "move_generation.hpp"
#include "assert.h"

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, std::shared_ptr<game_history> gh)
{
    assert(mover.is_valid());
    assert(mover.get_piece_color() == gh->to_move);
    return generate_moves(mover, from_x, from_y, gh->curr_board);
}

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board& b)
{
    // TODO: Generates only pseudo-legal moves!
    assert(mover.is_valid());

    switch(mover.get_piece_type()) {
        case KNIGHT: {
            std::vector<move> moves(8);
            knightmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        case KING: {
            std::vector<move> moves(8);
            kingmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        case BISHOP: {
            std::vector<move> moves(14);
            bishopmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        case ROOK: {
            std::vector<move> moves(14);
            rookmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        case PAWN: {
            std::vector<move> moves(4);
            pawnmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        case QUEEN: {
            std::vector<move> moves(28);
            queenmoves(mover, from_x, from_y, b, moves);
            return moves;
            break;
        }
        default:
            assert(false);
            return std::vector<move>();
            break;
    }
}

void move_generation::knightmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == KNIGHT);
    for(int8_t x = -2; x <= 2; ++x) {
        if(from_x + x < 0 || from_x + x > 7 || x == 0)
            continue;
        for(int8_t y = -2; y <= 2; ++y) {
            if((from_y + y < 0) || (from_y + y > 7) || (y == 0) || (x%2 == y%2))
                continue;
            //check occupancy, if not occupied by friendly piece, add move to vector
            auto p = b.get_piece(from_x + x, from_y + y);
            if(p.is_valid() && (p.get_piece_color() != mover.get_piece_color())) {
                moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y)});
            } else if(!p.is_valid()) {
                moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y)});
            }
        }
    }
}

void move_generation::kingmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == KING);
    for(int8_t x = -1; x <= 1; ++x) {
        if(from_x + x < 0 || from_x + x > 7)
            continue;
        for(int8_t y = -1; y <= 1; ++y) {
            if(from_y + y < 0 || from_y + y > 7 || (y == 0 && x == 0))
                continue;
            //check occupancy, if not occupied by friendly piece, add move to vector
            auto p = b.get_piece(from_x + x, from_y + y);
            if(p.is_valid() && p.get_piece_color() != mover.get_piece_color()) {
                moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y)});
            }
        }
    }
}

void move_generation::bishopmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == BISHOP || mover.get_piece_type() == QUEEN);
    
    // TODO: FANCY FAIRY MAGIC STUFF HAPPENING HERE
}

void move_generation::rookmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == ROOK || mover.get_piece_type() == QUEEN);
    
    // TODO: FANCY FAIRY MAGIC STUFF HAPPENING HERE
}

void move_generation::queenmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == QUEEN);
    rookmoves(mover, from_x, from_y, b, moves);
    bishopmoves(mover, from_x, from_y, b, moves);
}

void move_generation::pawnmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves)
{
    assert(mover.get_piece_type() == PAWN);
    
    int8_t y, starting_y;
    y = mover.get_piece_color() == WHITE ? 1 : -1;
    starting_y = mover.get_piece_color() == WHITE ? 1 : 6;
    
    //check possible capture moves
    //we don't have to boundary check y here, since a pawn on the last file gets promoted (and can't move backwards)
    for(int8_t x = -1; x < 2; x += 2) {
        if((from_x + x > 0) && (from_x + x < 7)) {
            piece p = b.get_piece(from_x + x, from_y + y);
            if(p.is_valid() && p.get_piece_color() != mover.get_piece_color())
                moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y)});
        }
    }
    
    //check occupancy for tile ahead of piece
    piece p = b.get_piece(from_x, from_y + y);
    if(!p.is_valid()) {
        moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y)});
        //check if we are still on the starting file
        if(from_y == starting_y) {
            p = b.get_piece(from_x, from_y + 2*y);
            if(!p.is_valid())
                moves.push_back((move) {mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y)});
        }
    }
}


