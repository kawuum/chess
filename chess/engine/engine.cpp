#include "engine.hpp"

void engine::new_game()
{
    this->gh = std::make_shared<game_history>();
    this->gh->num_halfmoves = 0;
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

void engine::perform_move(int from_x, int from_y, int to_x, int to_y)
{
    // TODO: CHECK IF MOVE IS LEGAL
    // TODO: Possible memory leaks...
    
    std::shared_ptr<game_history> new_gm = std::make_shared<game_history>();
    
    new_gm->curr_board = this->gh->curr_board;
    new_gm->curr_board.move_piece(from_x, from_y, to_x, to_y);
    new_gm->num_halfmoves = this->gh->num_halfmoves + 1;
    new_gm->to_move = this->gh->to_move == WHITE ? BLACK : WHITE;
    new_gm->prev = this->gh;
    this->gh = new_gm;
}

board& engine::get_current_board() {
    return this->gh->curr_board;
}


