#include <QApplication>

#include "MainWindow.hpp"
#include "Theme.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  theme::apply(app);

  MainWindow window;
  window.resize(960, 700);  // board and side panel
  window.show();

  return app.exec();
}
