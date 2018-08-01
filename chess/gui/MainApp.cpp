#include "MainApp.hpp"
#include "MainFrame.hpp"

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
    wxGridSizer* sizer = new wxGridSizer(8, 0, 0);
    MainFrame *frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));
        
    int color_counter = 0;
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            wxImagePanel *another_drawPane;
            if(color_counter%2) {
                another_drawPane = new wxImagePanel(frame, wxT("../gui/images/square_black.png"), 80, 80);
            } else {
                another_drawPane = new wxImagePanel(frame, wxT("../gui/images/square_white.png"), 80, 80);
            }
            sizer->Add(another_drawPane, 1, wxEXPAND);
            ++color_counter;
        }
        ++color_counter;
    }
    
    frame->SetSizer(sizer);
    
    frame->Show(true);
    return true;
}
