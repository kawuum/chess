//
// Created by root on 21.03.19.
//

#ifndef CHESS_CHESSBOARDWIDGET_HPP
#define CHESS_CHESSBOARDWIDGET_HPP

#include <QWidget>
#include <QPicture>
#include "ChessSquare.hpp"
#include "../engine/engine.hpp"
#include "../datastructures/piece.hpp"

class ChessBoardWidget : public QWidget {

public:
    ChessBoardWidget();
    ~ChessBoardWidget();
    void newGame();
    void undoMove();
    void addAi();
    void removeAi();
private:
    ChessSquare* squares[64];
    engine* eng;
    void drawBoard();
    std::string getFilenameForPiece(piece p);
};


#endif //CHESS_CHESSBOARDWIDGET_HPP
