#include "MainApp.hpp"
#include "MainFrame.hpp"
#include "../engine/engine.hpp"

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
    
    draw_board(eng->get_current_board());
                                
    std::vector<move> moves = eng->get_legal_moves(piece(KNIGHT, piece_color::WHITE), 1, 0);
    
    eng->perform_move(1,0,2,2);
    eng->perform_move(1,6,1,5);
    eng->perform_move(0,1,0,2);
    eng->perform_move(1,5,1,4);
    auto l_moves =eng-> get_legal_moves(piece{piece_type::ROOK, piece_color::WHITE}, 0, 0);
    std::cout << "lmoves size is: " << l_moves.size() << std::endl;
    draw_board(eng->get_current_board());
    
    return true;
}

void MainApp::draw_board(board& b) {
        
    if (frame != NULL) {
        frame->Destroy();
        delete frame;
    }
    
    frame = new MainFrame("Chess", wxPoint(50, 50), wxSize(640, 640), panels);
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
              frame->panels[x + (x * (y - 1))] = imagePanel;
                
            } else { 
              imagePanel = new wxImagePanel(frame, 80, 80, x, y - 1,std::bind(&MainApp::notify_click, this, std::placeholders::_1, std::placeholders::_2));  
              frame->panels[x + (x * (y - 1))] = imagePanel;
            }
            
            if((x+y)%2) {
                imagePanel->SetBackgroundColour(wxColor(80,80,80));
            } else {
                imagePanel->SetBackgroundColour(wxColor(255,255,255));
            }
            
            sizer->Add(imagePanel, 1, wxEXPAND);
        }
    }
    
    frame->SetSizer(sizer);
    
    frame->Show(true);
}

void MainApp::notify_click(uint8_t x_coord, uint8_t y_coord) {
    if(!clicked) {
     clicked = true;
     
     panels[x_coord + (x_coord * (y_coord))]->SetBackgroundColour(wxColor(255, 0, 0));
     panels[x_coord + (x_coord * (y_coord))]->paintNow();
    } else {
     
     clicked = false;
    }
    std::cout << "HOLY I AM CALLED with " << (int)x_coord << " " << (int)y_coord << std::endl;
}
