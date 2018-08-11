#include "move_generation.hpp"
#include "assert.h"
#include "../datastructures/enums.hpp"

std::vector<move> move_generation::generate_all_moves(game_history gh)
{
    return generate_all_moves(gh.curr_board, gh.to_move, false);
}


std::vector<move> move_generation::generate_all_moves(board& b, piece_color to_move, bool check_for_check)
{
    std::vector<move> moves;
    moves.reserve(80); // too many, but who cares... ;-)
    
    bit_matrix bm = b.get_bit_matrix_for_color(to_move);
    for(uint8_t y = 0; y < 8; ++y) {
        for(uint8_t x = 0; x < 8; ++x) {
            if(bm.get_bit_at(x, y)) {
                piece p = b.get_piece(x, y);
                if(!p.is_valid())
                    continue;
                std::vector<move> piece_moves = generate_moves(p, x, y, b, check_for_check);
                    
                if(check_for_check && !piece_moves.empty()) {
                    std::cout << "We found a check position: From (" << (int)piece_moves[0].from_x << "," << (int)piece_moves[0].from_y << ") to (" << (int)piece_moves[0].to_x << "," << (int)piece_moves[0].to_y << ")" << std::endl;
                    return piece_moves;
                }
                else if(!piece_moves.empty())
                    moves.insert(moves.end(), piece_moves.begin(), piece_moves.end());
            }
        }
    }

    return moves;
}


std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, std::shared_ptr<game_history> gh)
{    
    assert(mover.is_valid());
    assert(mover.get_piece_color() == gh->to_move);
    this->gamestate = *gh;
    return generate_moves(mover, from_x, from_y, gh->curr_board, false);
}

std::vector<move> move_generation::generate_moves(piece mover, uint8_t from_x, uint8_t from_y, board& b, bool check_for_check)
{
    // TODO: Generates only pseudo-legal moves!
    assert(mover.is_valid());

    switch(mover.get_piece_type()) {
        case KNIGHT: {
            std::vector<move> moves;
            moves.reserve(8);
            knightmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        case KING: {
            std::vector<move> moves;
            moves.reserve(8);
            kingmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        case BISHOP: {
            std::vector<move> moves;
            moves.reserve(14);
            bishopmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        case ROOK: {
            std::vector<move> moves;
            moves.reserve(14);
            rookmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        case PAWN: {
            std::vector<move> moves;
            moves.reserve(4);
            pawnmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        case QUEEN: {
            std::vector<move> moves;
            moves.reserve(28);
            queenmoves(mover, from_x, from_y, b, moves, check_for_check);
            return moves;
            break;
        }
        default:
            assert(false);
            return std::vector<move>();
            break;
    }
}

void move_generation::knightmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
{
    assert(mover.get_piece_type() == KNIGHT);
    for(int8_t x = -2; x <= 2; ++x) {
        if(from_x + x < 0 || from_x + x > 7 || x == 0)
            continue;
        for(int8_t y = -2; y <= 2; ++y) {
            if((from_y + y < 0) || (from_y + y > 7) || (y == 0) || (std::abs(x%2) == std::abs(y%2)))
                continue;
            //check occupancy, if not occupied by friendly piece, add move to vector
            auto p = b.get_piece(from_x + x, from_y + y);
            move m = (move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y), MOVE};
            if(p.is_valid() && (p.get_piece_color() != mover.get_piece_color())) {
                m.type = CAPTURE;
                if(check_for_check && p.get_piece_type() == KING) {
                    // doesn't matter that this is a capturing move an we return a standard move, we just want to find the first king capture
                    moves.push_back(m);
                    return;
                }
                else if(!check_for_check && !is_check(b, m))
                    moves.push_back(m);
            } else if(!p.is_valid()) {
                if(!check_for_check && !is_check(b, m))
                    moves.push_back(m);
            }
        }
    }
}

void move_generation::kingmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
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
            if((p.is_valid() && p.get_piece_color() != mover.get_piece_color())) {
                move m = (move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y), CAPTURE};
                if(check_for_check && p.get_piece_type() == KING) {
                    moves.push_back(m);
                    return;
                }
                else if(!check_for_check && !is_check(b, m))                    
                    moves.push_back(m);
            } else if (!p.is_valid()) {
                move m = (move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y), MOVE};
                if(!check_for_check && !is_check(b, m))
                    moves.push_back(m);
            }
        }
    }
}

void move_generation::bishopmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
{
    assert(mover.get_piece_type() == BISHOP || mover.get_piece_type() == QUEEN);
    
    bool stopped[4] = {false}; /* upper-left, lower-left, upper-right, lower-right */
    int8_t x_diff = 1;
    int8_t y_diff = 1;
    while(((((int8_t) from_x) - x_diff >= 0) && (((int8_t) from_x) + x_diff <= 7)) 
        || ((((int8_t) from_x) - x_diff >= 0) && (((int8_t) from_x) - x_diff >= 0)) 
        || ((((int8_t) from_x) + x_diff <=7) && (((int8_t) from_x) + x_diff <= 7))  
        || ((((int8_t) from_x) + x_diff <= 7) && (((int8_t) from_x) - x_diff >= 0)) ) {
     
        // upper-left
        bishopmoves_helper(mover, b, &stopped[0], from_x, from_y, (int8_t) from_x - x_diff, (int8_t) from_y + y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;

        // lower-left
        bishopmoves_helper(mover, b, &stopped[1], from_x, from_y, (int8_t) from_x - x_diff, (int8_t) from_y - y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        // upper-right
        bishopmoves_helper(mover, b, &stopped[2], from_x, from_y, (int8_t) from_x + x_diff, (int8_t) from_y + y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        // lower-right
        bishopmoves_helper(mover, b, &stopped[3], from_x, from_y, (int8_t) from_x + x_diff, (int8_t) from_y - y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        ++x_diff;
        ++y_diff;
    }
}

void move_generation::bishopmoves_helper(piece& mover, board& b, bool* stopped, uint8_t from_x, uint8_t from_y, int8_t x_diff, int8_t y_diff, std::vector<move>& moves, bool check_for_check) {
    if(!*stopped && x_diff >= 0 && x_diff <=7 && y_diff >=0 &&y_diff <=7) {
        piece p = b.get_piece(x_diff, y_diff);
        if(p.is_valid()) {         
        piece_color p_col = p.get_piece_color();
        if(p_col != mover.get_piece_color()) {
            move m = (move) {mover, from_x, from_y, (uint8_t)x_diff, (uint8_t)y_diff, CAPTURE};
            if(check_for_check && p.get_piece_type() == KING) {
                moves.push_back(m);
                return;
            }
            if(!check_for_check && !is_check(b, m))
                moves.push_back(m);
        }
        *stopped = true;                
        } else {
            move m = (move) {mover, from_x, from_y, (uint8_t)x_diff, (uint8_t)y_diff, MOVE};
            if(!check_for_check && !is_check(b, m)) {
                moves.push_back(m);
            }
        }
    }
}

void move_generation::rookmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
{
    assert(mover.get_piece_type() == ROOK || mover.get_piece_type() == QUEEN);
    
    bool stopped[4] = {false}; /* x-left, x-right, y-down, y-up */
    int8_t x_diff = 1;
    int8_t y_diff = 1;
    
    while((((int8_t) from_x) - x_diff >= 0) || (((int8_t) from_x) + x_diff <= 7) || (((int8_t) from_y) - y_diff >= 0) || (((int8_t) from_y) + y_diff <= 7)) {
        
        // x-left
        rookmoves_helper(mover, b, &stopped[0], from_x, from_y, (int8_t) from_x - x_diff, from_y, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        // x-right
        rookmoves_helper(mover, b, &stopped[1], from_x, from_y, from_x + x_diff, from_y, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        // y-up
        rookmoves_helper(mover, b, &stopped[2], from_x, from_y, from_x, from_y + y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        
        // y-down
        rookmoves_helper(mover, b, &stopped[3], from_x, from_y, from_x, (int8_t) from_y - y_diff, moves, check_for_check);
        if(check_for_check && !moves.empty())
            return;
        ++x_diff;
        ++y_diff;
    }
}

void move_generation::rookmoves_helper(piece& mover, board& b, bool* stopped, uint8_t from_x, uint8_t from_y, int8_t x_diff, int8_t y_diff, std::vector<move>& moves, bool check_for_check) 
{
    if(!*stopped && x_diff >= 0 && x_diff <=7 && y_diff >=0 && y_diff <=7) {
        piece p = b.get_piece(x_diff, y_diff);
        if(p.is_valid()) {         
        piece_color p_col = p.get_piece_color();
        if(p_col != mover.get_piece_color()) {
            move m = (move) {mover, from_x, from_y, (uint8_t) x_diff, (uint8_t) y_diff, CAPTURE};
            if(check_for_check && p.get_piece_type() == KING) {
                moves.push_back(m);
                return;
            }
            if(!check_for_check && !is_check(b, m))
                moves.push_back(m);
        } 
        *stopped = true;   
        } else {
            move m = (move) {mover, from_x, from_y, (uint8_t) x_diff, (uint8_t) y_diff, MOVE};
            if(!check_for_check && !is_check(b, m))
                moves.push_back(m);
        }
    }
}    

void move_generation::queenmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
{
    assert(mover.get_piece_type() == QUEEN);
    rookmoves(mover, from_x, from_y, b, moves, check_for_check);
    if(check_for_check && !moves.empty())
        return;
    bishopmoves(mover, from_x, from_y, b, moves, check_for_check);
}

void move_generation::pawnmoves(piece mover, uint8_t from_x, uint8_t from_y, board& b, std::vector<move>& moves, bool check_for_check)
{
    assert(mover.get_piece_type() == PAWN);
    
    int8_t y, starting_y, promotion_y;
    y = mover.get_piece_color() == piece_color::WHITE ? 1 : -1;
    starting_y = mover.get_piece_color() == piece_color::WHITE ? 1 : 6;
    promotion_y = mover.get_piece_color() == piece_color::WHITE ? 7 : 0;
    
    //check possible capture moves
    //we don't have to boundary check y here, since a pawn on the last file gets promoted (and can't move backwards)
    for(int8_t x = -1; x < 2; x += 2) {
        if((from_x + x >= 0) && (from_x + x <= 7)) {
            piece p = b.get_piece(from_x + x, from_y + y);
            if(p.is_valid() && p.get_piece_color() != mover.get_piece_color())
            {
                move_type mtype = from_y + y == promotion_y ? CAPTURING_PROMOTION : CAPTURE;
                move m = (move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y), mtype};
                if(check_for_check && p.get_piece_type() == KING) 
                    moves.push_back(m);
                else if(!check_for_check && !is_check(b, m))
                    moves.push_back(m);
            } else if ((std::get<0>(this->gamestate.ep_square) == (from_x + x)) && (std::get<1>(this->gamestate.ep_square) == (from_y + y))) {
                move m = (move) {mover, from_x, from_y, (uint8_t)(from_x + x), (uint8_t)(from_y + y), ENPASSANT};
                if(check_for_check && p.get_piece_type() == KING) 
                    moves.push_back(m);
                else if(!check_for_check && !is_check(b, m))
                    moves.push_back(m);
            }
        }
    }
    
    //we checked capturing moves, so nothing to do left if checking for king captures
    if(check_for_check) return;
    
    //check occupancy for tile ahead of piece
    piece p = b.get_piece(from_x, from_y + y);
    if(!p.is_valid()) {
        move_type mtype = from_y + y == promotion_y ? PROMOTION : MOVE;
        move m = (move) {mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y), mtype};
        if(is_check(b, m))
            return;
        moves.push_back(m);
        //check if we are still on the starting file
        if(from_y == starting_y) {
            p = b.get_piece(from_x, from_y + 2*y);
            if(!p.is_valid())
            {
                m = (move) {mover, from_x, from_y, (uint8_t)(from_x), (uint8_t)(from_y + y*2), MOVE};
                if(!is_check(b, m))
                    moves.push_back(m);
            }
        }
    }
}
bool move_generation::is_check(board& b, move m)
{
    // TODO: finish implementation
    
    board new_board = b;
    switch(m.type) {
        case MOVE:
            new_board.move_piece(m.mover, m.from_x, m.from_y, m.to_x, m.to_y);
            break;
        case CAPTURE:
            new_board.move_piece(m.mover, m.from_x, m.from_y, new_board.get_piece(m.to_x, m.to_y), m.to_x, m.to_y);
            break;
        case CASTLING:
            break;
        case ENPASSANT:
            break;
        case PROMOTION:
            break;
        case CAPTURING_PROMOTION:
            break;
        default:
            return false;
            break;
    }
    
    piece_color to_move = m.mover.get_piece_color() == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
    std::vector<move> moves = generate_all_moves(new_board, to_move, true);
    return !moves.empty();
    return false;
}



