//
// Created by root on 21.03.19.
//

#include "ChessSquare.hpp"

ChessSquare::ChessSquare() {
    this->xCoord = -1;
    this->yCoord = -1;
    this->backgroundColor = -1;
}
ChessSquare::ChessSquare(int x, int y, int color) {
    this->xCoord = x;
    this->yCoord = y;
    this->backgroundColor = color;
}

ChessSquare::~ChessSquare() {}

void ChessSquare::setImage(QPixmap *img) {
    this->image = img;
    if(this->image != nullptr) {
        this->setPixmap(*image);
    } else {
        this->clear();
    }
    this->setStyleSheet(backgroundColor == 0 ? "background-color: black" : "background-color: white");
    this->show();
}
