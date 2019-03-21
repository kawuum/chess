//
// Created by root on 21.03.19.
//

#ifndef CHESS_CHESSSQUARE_HPP
#define CHESS_CHESSSQUARE_HPP

#include <QLabel>
#include <QImage>

class ChessSquare : QLabel {
public:
    ChessSquare();
    ChessSquare(int x, int y, int color);
    ~ChessSquare();
    void setImage(QPixmap* img = nullptr);
private:
    int xCoord;
    int yCoord;
    int backgroundColor;
    QPixmap* image;
};


#endif //CHESS_CHESSSQUARE_HPP
