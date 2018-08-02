#ifndef MAINAPP_HPP
#define MAINAPP_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "wxImagePanel.hpp"
#include "MainFrame.hpp"
#include "board.hpp"
#include "piece.hpp"

class MainApp: public wxApp
{
public:
    virtual bool OnInit();
    void draw_board(board& b);
    
private:
    wxGridSizer *sizer;
    MainFrame *frame;
};

wxIMPLEMENT_APP(MainApp);

#endif
