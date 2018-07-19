//
// Created by Siegfried Depner on 7/17/18.
//

#include "board.hpp"

void board::addPiece(piece p, uint8_t x, uint8_t y)
{
    switch(p.get_piece_type())
    {
        case BISHOP:
            if(p.is_white()) this->white_bishops.set_bit_at(x, y);
            else this->black_bishops.set_bit_at(x, y);
            break;
        case KING:
            if(p.is_white()) this->white_king.set_bit_at(x, y);
            else this->black_king.set_bit_at(x, y);
            break;
        case KNIGHT:
            if(p.is_white()) this->white_knights.set_bit_at(x, y);
            else this->black_knights.set_bit_at(x, y);
            break;
        case PAWN:
            if(p.is_white()) this->white_pawns.set_bit_at(x, y);
            else this->black_pawns.set_bit_at(x, y);
            break;
        case QUEEN:
            if(p.is_white()) this->white_queens.set_bit_at(x, y);
            else this->black_queens.set_bit_at(x, y);
            break;
        case ROOK:
            if(p.is_white()) this->white_rooks.set_bit_at(x, y);
            else this->black_rooks.set_bit_at(x, y);
            break;
    }
}

void board::clear()
{
    this->black_bishops.clear();
    this->black_king.clear();
    this->black_knights.clear();
    this->black_pawns.clear();
    this->black_queens.clear();
    this->black_rooks.clear();
    this->white_bishops.clear();
    this->white_king.clear();
    this->white_knights.clear();
    this->white_pawns.clear();
    this->white_queens.clear();
    this->white_rooks.clear();
}

bool board::check_sanity()
{
    // TODO: implement
}

bool board::is_empty()
{
    return this->get_num_pieces() == 0;
}


uint8_t board::get_num_pieces()
{
    return this->get_num_pieces(BLACK) + this->get_num_pieces(WHITE);
}

uint8_t board::get_num_pieces(piece_color c)
{
    return this->get_num_pieces(piece(BISHOP, c)) +
           this->get_num_pieces(piece(KING, c)) +
           this->get_num_pieces(piece(KNIGHT, c)) +
           this->get_num_pieces(piece(PAWN, c)) +
           this->get_num_pieces(piece(QUEEN, c)) +
           this->get_num_pieces(piece(ROOK, c));
}

uint8_t board::get_num_pieces(piece p)
{
    return this->get_bit_matrix_for_piece(p).get_num_bits_set();
    /*switch(p.get_piece_color())
    {
        case WHITE:
        {
            switch(p.get_piece_type())
            {
                case BISHOP:
                    return this->white_bishops.get_num_bits_set();
                    break;
                case KING:
                    return this->white_king.get_num_bits_set();
                    break;
                case KNIGHT:
                    return this->white_knights.get_num_bits_set();
                    break;
                case PAWN:
                    return this->white_pawns.get_num_bits_set();
                    break;
                case QUEEN:
                    return this->white_queens.get_num_bits_set();
                    break;
                case ROOK:
                    return this->white_rooks.get_num_bits_set();
                    break;
            }
        }
        break;
        case BLACK:
        {
            switch(p.get_piece_type())
            {
                case BISHOP:
                    return this->black_bishops.get_num_bits_set();
                    break;
                case KING:
                    return this->black_king.get_num_bits_set();
                    break;
                case KNIGHT:
                    return this->black_knights.get_num_bits_set();
                    break;
                case PAWN:
                    return this->black_pawns.get_num_bits_set();
                    break;
                case QUEEN:
                    return this->black_queens.get_num_bits_set();
                    break;
                case ROOK:
                    return this->black_rooks.get_num_bits_set();
                    break;
            }
        }
        break;
    }*/
    return this->get_bit_matrix_for_piece(p).get_num_bits_set();
}

bit_matrix & board::get_bit_matrix_for_piece(piece p)
{
    switch(p.get_piece_color())
    {
        case WHITE:
        {
            switch(p.get_piece_type())
            {
                case BISHOP:
                    return this->white_bishops;
                    break;
                case KING:
                    return this->white_king;
                    break;
                case KNIGHT:
                    return this->white_knights;
                    break;
                case PAWN:
                    return this->white_pawns;
                    break;
                case QUEEN:
                    return this->white_queens;
                    break;
                case ROOK:
                    return this->white_rooks;
                    break;
            }
        }
        break;
        case BLACK:
        {
            switch(p.get_piece_type())
            {
                case BISHOP:
                    return this->black_bishops;
                    break;
                case KING:
                    return this->black_king;
                    break;
                case KNIGHT:
                    return this->black_knights;
                    break;
                case PAWN:
                    return this->black_pawns;
                    break;
                case QUEEN:
                    return this->black_queens;
                    break;
                case ROOK:
                    return this->black_rooks;
                    break;
            }
        }
        break;
    }
}








