#include "MainApp.hpp"
#include "MainFrame.hpp"
#include "../engine/engine.hpp"
#include "../datastructures/consts.hpp"
#include <algorithm>

MainApp::~MainApp() {
}

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
       
    frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));
    
    return true;
}
