#include "MainApp.hpp"

#include <algorithm>

#include "../datastructures/consts.hpp"
#include "../engine/engine.hpp"
#include "MainFrame.hpp"

MainApp::~MainApp() {}

bool MainApp::OnInit() {
  wxInitAllImageHandlers();

  frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));

  return true;
}
