#include "MainApp.hpp"
#include "MainFrame.hpp"

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    MainFrame *frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));
    
    drawPane = new wxImagePanel(frame, wxT("../gui/images/square_white.png"), 60, 60);
    
    drawPane->setRenderPosition(50, 50);
    sizer->Add(drawPane, 1, wxEXPAND);
    frame->SetSizer(sizer);
    
    frame->Show(true);
    return true;
}
