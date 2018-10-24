#include "MainWindow.h"

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);  
    
  MainWindow window;

  window.resize(640, 480);
  window.setWindowTitle("Chess");
  window.show();

  return app.exec();
}
