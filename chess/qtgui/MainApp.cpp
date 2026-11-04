#include <QApplication>

#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  MainWindow window;
  window.resize(640, 700);
  window.show();

  return app.exec();
}
