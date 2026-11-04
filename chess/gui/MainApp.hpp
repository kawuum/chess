#ifndef MAINAPP_HPP
#define MAINAPP_HPP

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <memory>
#include <tuple>

#include "../engine/engine.hpp"
#include "MainFrame.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "wxImagePanel.hpp"

class MainApp : public wxApp {
 public:
  ~MainApp();
  virtual bool OnInit();

 private:
  MainFrame* frame;
};

wxIMPLEMENT_APP(MainApp);

#endif
