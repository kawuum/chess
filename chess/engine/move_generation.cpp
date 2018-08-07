#include "move_generation.hpp"
#include "assert.h"

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, game_history gh)
{
    assert(mover.get_piece_color() == gh.to_move);
    
}

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    // TODO: Generates only pseudo-legal moves!
    switch(mover.get_piece_type()) {
        case KNIGHT:
            return knightmoves(mover, from_x, from_y, b);
            break;
        case KING:
            return kingmoves(mover, from_x, from_y, b);
            break;
        case BISHOP:
            return bishopmoves(mover, from_x, from_y, b);
            break;
        case ROOK:
            return rookmoves(mover, from_x, from_y, b);
            break;
        case PAWN:
            return pawnmoves(mover, from_x, from_y, b);
            break;
        case QUEEN:
            return queenmoves(mover, from_x, from_y, b);
            break;
    }
}

std::vector<move> move_generation::knightmoves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    std::vector<move> moves;
    assert(mover.get_piece_type() == KNIGHT);
    for(int x = -2; x <= 2; ++x) {
        if(from_x + x < 0 || from_x + x > 7 || x == 0)
            continue;
        for(int y = -2; y <= 2; ++y) {
            if(from_y + y <0 || from_y + y > 7 || y == 0)
                continue;
            // TODO: NOT YET ACCURATE!!! x = -2 && y= -2 !?!?
            //check occupancy, if not occupied by friendly piece, add move to vector
            if(b.get_piece(from_x + x, from_y + y).get_piece_color() != mover.get_piece_color()) {
                //moves.insert(moves.end(), );
            }
        }
    }
    return moves;
}
std::vector<move> move_generation::kingmoves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    std::vector<move> moves;
    assert(mover.get_piece_type() == KING);
    for(int x = -1; x <= 1; ++x) {
        if(from_x + x < 0 || from_x + x > 7)
            continue;
        for(int y = -2; y <= 2; ++y) {
            if(from_y + y <0 || from_y + y > 7 || (y == 0 && x == 0))
                continue;
            //check occupancy, if not occupied by friendly piece, add move to vector
            if(b.get_piece(from_x + x, from_y + y).get_piece_color() != mover.get_piece_color()) {
                //moves.insert(moves.end(), );
            }
        }
    }
    return moves;
}

std::vector<move> move_generation::bishopmoves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    std::vector<move> moves;
    assert(mover.get_piece_type() == BISHOP || mover.get_piece_type() == QUEEN);
    
    // TODO: FANCY FAIRY MAGIC STUFF HAPPENING HERE
    
    return moves;
}

std::vector<move> move_generation::rookmoves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    std::vector<move> moves;
    assert(mover.get_piece_type() == ROOK || mover.get_piece_type() == QUEEN);
    
    // TODO: FANCY FAIRY MAGIC STUFF HAPPENING HERE
    
    return moves;
}

std::vector<move> move_generation::queenmoves(piece mover, uint8_t from_x, uint8_t from_y, board b)
{
    std::vector<move> moves;
    assert(mover.get_piece_type() == QUEEN);
    return concat_vectors(rookmoves(mover, from_x, from_y, b), bishopmoves(mover, from_x, from_y, b));
}


std::vector<move> move_generation::concat_vectors(std::vector<move> a, std::vector<move> b)
{
    std::vector<move> ab;
    ab.reserve(a.size() + b.size());
    ab.insert(ab.end(), a.begin(), a.end());
    ab.insert(ab.end(), b.begin(), b.end());
    return ab;
}


