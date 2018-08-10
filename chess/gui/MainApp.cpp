#include "MainApp.hpp"
#include "MainFrame.hpp"
#include "../engine/engine.hpp"
#include "../datastructures/consts.hpp"
#include <algorithm>

MainApp::~MainApp() {
    if(panels != NULL) {
     free(panels);   
    }
}

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
    
    clicked = false;
    
    panels = (wxImagePanel **) malloc(sizeof(wxImagePanel*) * 64);
    
    eng = std::make_shared<engine>();
    eng->new_game();
    
    frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640), panels);
    
    draw_board(eng->get_current_board());
    
    return true;
}

void MainApp::draw_board(board& b) {

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
                             
              imagePanel = new wxImagePanel(frame, url, 80, 80, 0, 0, x, y - 1, std::bind(&MainApp::notify_click, this, std::placeholders::_1, std::placeholders::_2));
              frame->panels[x + (8 * (y - 1))] = imagePanel;
                
            } else { 
              imagePanel = new wxImagePanel(frame, 80, 80, x, y - 1,std::bind(&MainApp::notify_click, this, std::placeholders::_1, std::placeholders::_2));  
              frame->panels[x + (8 * (y - 1))] = imagePanel;
            }
            sizer->Add(imagePanel, 1, wxEXPAND);
        }
    }
    frame->SetSizer(sizer);
    frame->Layout();
    frame->Show(true);
    frame->recolor_board(); 
}

void MainApp::notify_click(uint8_t x_coord, uint8_t y_coord) {
    
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
        frame->recolor_board();
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
            frame->recolor_board();
            clicked = false; 
            eng->perform_move(*clicked_move);
            draw_board(eng->get_current_board());
         }
     }
        
        
     
    }
    
}
