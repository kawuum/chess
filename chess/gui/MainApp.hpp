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
#include "../engine/engine.hpp"
#include <memory>

class MainApp: public wxApp
{
public:
    ~MainApp();
    virtual bool OnInit();
    void draw_board(board& b);
    void notify_click(uint8_t x_coord, uint8_t y_coord);
    
private:
    wxGridSizer *sizer;
    MainFrame *frame;
    wxImagePanel** panels;
    std::shared_ptr<engine> eng;
    
    bool clicked;
};

wxIMPLEMENT_APP(MainApp);

#endif
