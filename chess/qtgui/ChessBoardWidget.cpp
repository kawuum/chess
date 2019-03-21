//
// Created by root on 21.03.19.
//

#include "ChessBoardWidget.hpp"

ChessBoardWidget::ChessBoardWidget() : QWidget(), squares() {
    eng = new engine();
    eng->new_game();
    printf("hi\n");
    for (int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            printf("x:\t%d\ty:\t%d\n", x, y);
            squares[x + (8*y)] = (new ChessSquare(x, y, (x+y)%2));
            piece p = eng->get_current_board().get_piece(x, y);
            if(p.is_valid()) {
                //load image
                /*QString qstr = QString::fromStdString(getFilenameForPiece(p));
                QPicture* pic = new QPicture();
                pic->load(qstr);*/
                // FIXME: memory leak?!
                QPixmap pic(QString::fromStdString(getFilenameForPiece(p)));
                squares[x + (8*y)]->setImage(&pic);
            } else {
                //set to no image
                squares[x + (8*y)]->setImage();
            }
        }
    }

    printf("bye\n");

    //drawBoard();
}

ChessBoardWidget::~ChessBoardWidget() {}

void ChessBoardWidget::newGame() {
    eng->new_game();
}

void ChessBoardWidget::undoMove() {
    eng->undo_move();
}

void ChessBoardWidget::addAi() {
    // dostuff
}

void ChessBoardWidget::removeAi() {
    // dostuff
}

void ChessBoardWidget::drawBoard() {
    for (int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            piece p  = eng->get_current_board().get_piece(x, y);
            if(p.is_valid()) {
                //load image
                // FIXME: memory leak?!
                QPixmap pic(QString::fromStdString(getFilenameForPiece(p)));
                squares[x + (8*y)]->setImage(&pic);
            } else {
                //set to no image
                squares[x + (8*y)]->setImage();
            }
        }
    }
}

std::string ChessBoardWidget::getFilenameForPiece(piece p) {
    if(!p.is_valid()) {
        return "";
    }

    std::string url = "../gui/images";
    if (p.get_piece_color() == piece_color::WHITE) {
        url = url + "/white_pieces";
    } else {
        url = url + "/black_pieces";
    }
    url = url + "/120px-Chess_tile_";

    if (p.get_piece_type() == piece_type::KING) {
        url = url + "k";
    } else if (p.get_piece_type() == piece_type::QUEEN) {
        url = url + "q";
    } else if (p.get_piece_type() == piece_type::BISHOP) {
        url = url + "b";
    } else if (p.get_piece_type() == piece_type::KNIGHT) {
        url = url + "n";
    } else if (p.get_piece_type() == piece_type::ROOK) {
        url = url + "r";
    } else if (p.get_piece_type() == piece_type::PAWN) {
        url = url + "p";
    }
    url = url + ".png";
    return url;
}