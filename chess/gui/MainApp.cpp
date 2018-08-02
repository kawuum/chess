#include "MainApp.hpp"
#include "MainFrame.hpp"
#include "../engine/engine.hpp"

bool MainApp::OnInit()
{
    wxInitAllImageHandlers();
    /*
    board b;
    
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
                        
    draw_board(eng->get_current_board());
    
    eng->get_current_board().get_bit_matrix_for_color(BLACK).print();
    
    eng->perform_move(1,1,1,3);
    
    eng->get_current_board().get_bit_matrix_for_color(BLACK).print();
    
    //sleep(3000);
    
    draw_board(eng->get_current_board());
    
    return true;
}

    void MainApp::draw_board(board& b) {
            wxGridSizer* sizer = new wxGridSizer(8, 0, 0);
    MainFrame *frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640));
        
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            wxPanel *drawPane;
            
            piece p = b.get_piece(i, j);
            
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
                drawPane = new wxImagePanel(frame, url, 80, 80);
                
            } else {
               
              drawPane = new wxPanel(frame);   
            }
            
            if((i+j)%2) {
                drawPane->SetBackgroundColour(wxColor(80,80,80));
            } else {
                drawPane->SetBackgroundColour(wxColor(255,255,255));
            }
            
            sizer->Add(drawPane, 1, wxEXPAND);
        }
    }
    
    frame->SetSizer(sizer);
    
    frame->Show(true);
    }
