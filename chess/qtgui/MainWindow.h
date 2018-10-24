#pragma once

#include <QMainWindow>
#include <QObject>
#include <QApplication>

class MainWindow : public QMainWindow {
  Q_OBJECT
  public:
    MainWindow(QWidget *parent = 0);
  public slots:
    void new_game();
    void undo_move();
    void add_ai();
    void remove_ai();
  private:
    
};
