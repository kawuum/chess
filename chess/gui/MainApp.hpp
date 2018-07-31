#ifndef MAINAPP_HPP
#define MAINAPP_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "wxImagePanel.hpp"

class MainApp: public wxApp
{
public:
    wxImagePanel *drawPane;
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MainApp);

#endif
