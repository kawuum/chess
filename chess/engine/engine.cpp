#include "engine.hpp"
#include "move_generation.hpp"
#include "move_generation.hpp"

void engine::new_game()
{
    this->gh = std::make_shared<game_history>();
    this->gh->num_halfmoves = 0;
    this->gh->to_move = piece_color::WHITE;
    this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 0, 0);
    this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 1, 0);
    this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 2, 0);
    this->gh->curr_board.add_piece(piece(piece_type::QUEEN, piece_color::WHITE), 3, 0);
    this->gh->curr_board.add_piece(piece(piece_type::KING, piece_color::WHITE), 4, 0);
    this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 5, 0);
    this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 6, 0);
    this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 7, 0);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 0, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 1, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 2, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 3, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 4, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 5, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 6, 1);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 7, 1);                   
        
    this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 0, 7);
    this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 1, 7);
    this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 2, 7);   
    this->gh->curr_board.add_piece(piece(piece_type::QUEEN, piece_color::BLACK), 3, 7);
    this->gh->curr_board.add_piece(piece(piece_type::KING, piece_color::BLACK), 4, 7);
    this->gh->curr_board.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 5, 7);
    this->gh->curr_board.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 6, 7);
    this->gh->curr_board.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 7, 7);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 0, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 1, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 2, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 3, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 4, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 5, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 6, 6);
    this->gh->curr_board.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 7, 6); 
}

void engine::perform_move(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y)
{
    this->perform_move(this->gh->curr_board.get_piece(from_x, from_y), from_x, from_y, to_x, to_y);
}

std::vector<move> engine::get_legal_moves(piece p, uint8_t from_x, uint8_t from_y) {
    move_generation mg;
    return mg.generate_moves(p, from_x, from_y, this->gh);
}

void engine::perform_move(piece p, uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y)
{
    // TODO: CHECK IF MOVE IS LEGAL
    // TODO: Possible memory leaks...
    
    move_generation mg;
    std::vector<move> moves = mg.generate_moves(p, from_x, from_y, this->gh);
    
    //check if move to perform is in generated moves --> LEGALITY CHECK
    
    //if(!p.is_valid())
        piece pie = this->gh->curr_board.get_piece(from_x, from_y);
    if(!pie.is_valid())
        return;
    if(pie.get_piece_color() != this->gh->to_move)
        return;
    /*if(pie != p)
        return; //should we just return here? or maybe ignore if the move seems to be legal?
    */
    
    std::shared_ptr<game_history> new_gm = std::make_shared<game_history>();
    
    new_gm->curr_board = this->gh->curr_board;
    new_gm->curr_board.move_piece(pie, from_x, from_y, to_x, to_y);
    new_gm->num_halfmoves = this->gh->num_halfmoves + 1;
    new_gm->to_move = this->gh->to_move == piece_color::WHITE ? piece_color::BLACK : piece_color::WHITE;
    new_gm->prev = this->gh;
    this->gh = new_gm;
}

board& engine::get_current_board() {
    return this->gh->curr_board;
}

void engine::undo_move()
{
    if(this->gh->prev != NULL)
        this->gh = this->gh->prev;
}



