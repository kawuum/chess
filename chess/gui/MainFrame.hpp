#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/sizer.h>
#include "wxImagePanel.hpp"
#include <memory>
#include <../engine/engine.hpp>

class MainFrame: public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MainFrame();
    wxImagePanel** panels;
    void recolor_board();
    void draw_board(board& b);
    void notify_click(uint8_t x_coord, uint8_t y_coord);
private:
    void OnNewGame(wxCommandEvent& event);
    void OnUndoMove(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void new_game();
    
    bool clicked;
    std::tuple<uint8_t, uint8_t> clicked_coords;
    wxGridSizer *sizer;
    std::shared_ptr<engine> eng;
    
    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_NewGame = 1,
    ID_UndoMove = 2
};

#endif
