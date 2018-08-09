#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/sizer.h>
#include "wxImagePanel.hpp"
#include <memory>

class MainFrame: public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, wxImagePanel** panels);
    wxImagePanel** panels;
    
private:
    void OnNewGame(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_NewGame = 1
};

#endif
