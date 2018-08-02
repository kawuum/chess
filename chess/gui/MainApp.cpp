#include "MainApp.hpp"
#include "MainFrame.hpp"
#include "../engine/engine.hpp"

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
    frame = NULL;

    board b;
    
    /*
    b.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 0, 0);
    b.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 1, 0);
    b.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 2, 0);
    b.add_piece(piece(piece_type::QUEEN, piece_color::WHITE), 3, 0);
    b.add_piece(piece(piece_type::KING, piece_color::WHITE), 4, 0);
    b.add_piece(piece(piece_type::BISHOP, piece_color::WHITE), 5, 0);
    b.add_piece(piece(piece_type::KNIGHT, piece_color::WHITE), 6, 0);
    b.add_piece(piece(piece_type::ROOK, piece_color::WHITE), 7, 0);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 0, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 1, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 2, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 3, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 4, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 5, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 6, 1);
    b.add_piece(piece(piece_type::PAWN, piece_color::WHITE), 7, 1);                   
        
    b.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 0, 7);
    b.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 1, 7);
    b.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 2, 7);   
    b.add_piece(piece(piece_type::QUEEN, piece_color::BLACK), 3, 7);
    b.add_piece(piece(piece_type::KING, piece_color::BLACK), 4, 7);
    b.add_piece(piece(piece_type::BISHOP, piece_color::BLACK), 5, 7);
    b.add_piece(piece(piece_type::KNIGHT, piece_color::BLACK), 6, 7);
    b.add_piece(piece(piece_type::ROOK, piece_color::BLACK), 7, 7);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 0, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 1, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 2, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 3, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 4, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 5, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 6, 6);
    b.add_piece(piece(piece_type::PAWN, piece_color::BLACK), 7, 6);
    */
    
    engine* eng = new engine();
    eng->new_game();
                        
    eng->perform_move(1,1,1,3); 
    eng->perform_move(1,3,1,4);
    eng->perform_move(5,6,5,5);
    
    draw_board(eng->get_current_board());
    
    eng->get_current_board().get_bit_matrix_for_color(WHITE).print();
    
    return true;
}

    void MainApp::draw_board(board& b) {
        
    if (frame != NULL) {
        frame->Destroy();
    }
    frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));
    sizer = new wxGridSizer(8, 0, 0);
        
    for(int y = 8; y > 0; --y) {
        for(int x = 0; x < 8; ++x) {
            
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
                frame->drawPane = new wxImagePanel(frame, url, 80, 80, 0, 0);
                
            } else {               
              frame->drawPane = new wxPanel(frame);   
            }
            
            if((x+y)%2) {
                frame->drawPane->SetBackgroundColour(wxColor(80,80,80));
            } else {
                frame->drawPane->SetBackgroundColour(wxColor(255,255,255));
            }
            
            sizer->Add(frame->drawPane, 1, wxEXPAND);
        }
    }
    
    frame->SetSizer(sizer);
    
    frame->Show(true);
    }
