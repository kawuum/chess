#ifndef WXIMAGEPANEL_HPP
#define WXIMAGEPANEL_HPP

#include <wx/wx.h>
#include <wx/sizer.h>
#include <functional>

class wxImagePanel : public wxPanel
    {
        wxImage original_image;
        int pos_x, pos_y;
        int grid_x, grid_y;
        std::function<void(uint8_t, uint8_t)> notify_click;
        
 
    public:
        wxImagePanel(wxFrame* parent, int size_x, int size_y, int grid_x, int grid_y, std::function<void(uint8_t, uint8_t)> notify_click);
        wxImagePanel(wxFrame* parent, wxString file, int size_x, int size_y, int grid_x, int grid_y, std::function<void(uint8_t, uint8_t)> notify_click);
        wxImagePanel(wxFrame* parent, wxString file, int size_x, int size_y, int pos_x, int pos_y, int grid_x, int grid_y, std::function<void(uint8_t, uint8_t)> notify_click);
         
        void paintEvent(wxPaintEvent & evt);
        void paintNow();
 
        void render(wxDC& dc);
        void setRenderPosition(int pos_x, int pos_y);
        int getRenderPositionX();
        int getRenderPositionY();
        void mouseDown(wxMouseEvent& event);
        
        
 
        // some useful events
        /*
         void mouseMoved(wxMouseEvent& event);
         void mouseWheelMoved(wxMouseEvent& event);
         void mouseReleased(wxMouseEvent& event);
         void rightClick(wxMouseEvent& event);
         void mouseLeftWindow(wxMouseEvent& event);
         void keyPressed(wxKeyEvent& event);
         void keyReleased(wxKeyEvent& event);
         */
 
        DECLARE_EVENT_TABLE();
    };
 
 
#endif
