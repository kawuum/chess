#ifndef WXIMAGEPANEL_HPP
#define WXIMAGEPANEL_HPP

#include <wx/wx.h>
#include <wx/sizer.h>
 
class wxImagePanel : public wxPanel
    {
        wxImage image;
        wxImage original_image;
        int pos_x, pos_y;
 
    public:
        wxImagePanel(wxFrame* parent, wxString file, int size_x, int size_y);
        wxImagePanel(wxFrame* parent, wxString file, int size_x, int size_y, int pos_x, int pos_y);
         
        void paintEvent(wxPaintEvent & evt);
        void paintNow();
 
        void render(wxDC& dc);
        void setRenderPosition(int pos_x, int pos_y);
        int getRenderPositionX();
        int getRenderPositionY();
 
        // some useful events
        /*
         void mouseMoved(wxMouseEvent& event);
         void mouseDown(wxMouseEvent& event);
         void mouseWheelMoved(wxMouseEvent& event);
         void mouseReleased(wxMouseEvent& event);
         void rightClick(wxMouseEvent& event);
         void mouseLeftWindow(wxMouseEvent& event);
         void keyPressed(wxKeyEvent& event);
         void keyReleased(wxKeyEvent& event);
         */
 
        DECLARE_EVENT_TABLE()
    };
 
 
#endif
