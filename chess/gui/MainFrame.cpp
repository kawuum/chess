#include "MainFrame.hpp"
#include "../datastructures/consts.hpp"
#include <algorithm>

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    panels = (wxImagePanel **) malloc(sizeof(wxImagePanel*) * 64);
        
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_NewGame, "&New Game\tCtrl-N",
                     "Start a new game");
    menuFile->AppendSeparator();
    menuFile->Append(ID_UndoMove, "&Undo Move\tCtrl-U", "Undo the last move");
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
    
    clicked = false;
    
    eng = std::make_shared<engine>();
    this->new_game();
}
MainFrame::~MainFrame() {
    if(panels != NULL) {
     free(panels);   
    }
}
void MainFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MainFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "This is a simple, barely functional chess programm.",
                  "About Chess", wxOK | wxICON_INFORMATION );
}
void MainFrame::OnNewGame(wxCommandEvent& event)
{

    this->new_game();
}

void MainFrame::OnUndoMove(wxCommandEvent& event)
{
    eng->undo_move();
    draw_board(eng->get_current_board());
}

void MainFrame::new_game()
{
    eng->new_game();
    draw_board(eng->get_current_board());
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

void MainFrame::notify_click(uint8_t x_coord, uint8_t y_coord) {
        
    if(!clicked) { // First time clicked, we check if the click is on a piece, else return
        auto piece = eng->get_current_board().get_piece(x_coord, y_coord);
     if(!piece.is_valid() || piece.get_piece_color() != eng->get_color_to_move()){
      return;   
     }
     clicked = true;
     clicked_coords = std::make_tuple(x_coord, y_coord);
          
     panels[x_coord + (8 * y_coord)]->SetBackgroundColour(BG_RED);
     panels[x_coord + (8 * y_coord)]->paintNow();
     
     std::vector<move> possible_moves = eng->get_legal_moves(eng->get_current_board().get_piece(x_coord, y_coord), x_coord, y_coord);
     
     for(auto it = possible_moves.begin(); it < possible_moves.end(); ++it) {
         panels[it->to_x + (8 * it->to_y)]->SetBackgroundColour(BG_BLUE);
         panels[it->to_x + (8 * it->to_y)]->paintNow();
     }
     
    } else { // A piece was already clicked, now we need to check if this is a possible move click or a disable piece click
     
     if(std::get<0>(clicked_coords) == x_coord && std::get<1>(clicked_coords) == y_coord) { // Check if this is a diable piece click      
        this->recolor_board();
        clicked = false;
     } else {  // Check if this is a possible move click
         std::vector<move> possible_moves = eng->get_legal_moves(eng->get_current_board().get_piece(std::get<0>(clicked_coords), std::get<1>(clicked_coords)), std::get<0>(clicked_coords), std::get<1>(clicked_coords));
         if(possible_moves.empty())
         {
             return;
         }
         move p_moved{possible_moves[0].mover, possible_moves[0].from_x, possible_moves[0].from_y, x_coord, y_coord};
         auto clicked_move = std::find(possible_moves.begin(), possible_moves.end(), p_moved) ;
         if(clicked_move != possible_moves.end()) { // it is a possible move click
            this->recolor_board();
            clicked = false; 
            eng->perform_move(*clicked_move);
            draw_board(eng->get_current_board());
         }
     }
    }
}

void MainFrame::draw_board(board& b)
{
    sizer = new wxGridSizer(8, 0, 0);  
    for(int y = 8; y > 0; --y) {
        for(int x = 0; x < 8; ++x) {
            wxImagePanel* imagePanel;
            piece p = b.get_piece(x, y-1);
            
            if(p.is_valid()) {
                std::string url = "../gui/images";
                if (p.get_piece_color() == piece_color::WHITE) {
                    url = url + "/white_pieces";
                } else {
                    url = url + "/black_pieces";
                }
                url = url + "/120px-Chess_tile_";
                
                if (p.get_piece_type() == piece_type::KING) {
                    url = url + "k";
                } else if(p.get_piece_type() == piece_type::QUEEN) {
                    url = url + "q";                  
                } else if(p.get_piece_type() == piece_type::BISHOP) {
                       url = url + "b";                 
                } else if(p.get_piece_type() == piece_type::KNIGHT) {
                         url = url + "n";               
                } else if(p.get_piece_type() == piece_type::ROOK) {
                       url = url + "r";                 
                } else if(p.get_piece_type() == piece_type::PAWN) {
                    url = url + "p";
                } 
                url = url + ".png";
                             
              imagePanel = new wxImagePanel(this, url, 80, 80, 0, 0, x, y - 1, std::bind(&MainFrame::notify_click, this, std::placeholders::_1, std::placeholders::_2));
              this->panels[x + (8 * (y - 1))] = imagePanel;
                
            } else { 
              imagePanel = new wxImagePanel(this, 80, 80, x, y - 1,std::bind(&MainFrame::notify_click, this, std::placeholders::_1, std::placeholders::_2));  
              this->panels[x + (8 * (y - 1))] = imagePanel;
            }
            sizer->Add(imagePanel, 1, wxEXPAND);
        }
    }
    this->SetSizer(sizer);
    this->Layout();
    this->Show(true);
    this->recolor_board(); 
}


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_NewGame,   MainFrame::OnNewGame)
    EVT_MENU(ID_UndoMove, MainFrame::OnUndoMove)
    EVT_MENU(wxID_EXIT,  MainFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
wxEND_EVENT_TABLE()
