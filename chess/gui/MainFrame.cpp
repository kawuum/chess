#include "MainFrame.hpp"
#include "../datastructures/consts.hpp"

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, wxImagePanel** panels)
        : wxFrame(NULL, wxID_ANY, title, pos, size), panels(panels)
{
    
        
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_NewGame, "&New Game\tCtrl-N",
                     "Start a new game");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText( "Welcome to Chess!" );
}
void MainFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a simple, non-functional chess programm.",
                  "About Chess", wxOK | wxICON_INFORMATION );
}
void MainFrame::OnNewGame(wxCommandEvent& event)
{
    //wxLogMessage("Welcome to Chess!");
    // TODO: implement
}

void MainFrame::recolor_board()
{
    for(int y = 8; y > 0; --y) {
        for(int x = 0; x < 8; ++x) {
            if((x+y)%2) {
                panels[x + (8 * (y-1))]->SetBackgroundColour(BG_GREY);
            } else {
                panels[x + (8 * (y-1))]->SetBackgroundColour(BG_WHITE);
            }
            panels[x + (8 * (y-1))]->paintNow();
        }
    }
}


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_NewGame,   MainFrame::OnNewGame)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()
