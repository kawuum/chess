#include "wxImagePanel.hpp"

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel
)
// some useful events

EVT_LEFT_DOWN(wxImagePanel::mouseDown)
/*
EVT_MOTION(wxImagePanel::mouseMoved)
EVT_LEFT_UP(wxImagePanel::mouseReleased)
EVT_RIGHT_DOWN(wxImagePanel::rightClick)
EVT_LEAVE_WINDOW(wxImagePanel::mouseLeftWindow)
EVT_KEY_DOWN(wxImagePanel::keyPressed)
EVT_KEY_UP(wxImagePanel::keyReleased)
EVT_MOUSEWHEEL(wxImagePanel::mouseWheelMoved)
*/

// catch paint events
EVT_PAINT(wxImagePanel::paintEvent)

END_EVENT_TABLE()


// some useful events
/*
 void wxImagePanel::mouseMoved(wxMouseEvent& event) {}
 void wxImagePanel::mouseDown(wxMouseEvent& event) {}
 void wxImagePanel::mouseWheelMoved(wxMouseEvent& event) {}
 void wxImagePanel::mouseReleased(wxMouseEvent& event) {}
 void wxImagePanel::rightClick(wxMouseEvent& event) {}
 void wxImagePanel::mouseLeftWindow(wxMouseEvent& event) {}
 void wxImagePanel::keyPressed(wxKeyEvent& event) {}
 void wxImagePanel::keyReleased(wxKeyEvent& event) {}
 */
wxImagePanel::wxImagePanel(wxFrame *parent,
                           int size_x,
                           int size_y,
                           int grid_x,
                           int grid_y,
                           std::function<void(uint8_t, uint8_t)> notify_click)
    : wxPanel(parent), grid_x(grid_x), grid_y(grid_y), notify_click(notify_click) {
  wxImage image;
  original_image = image;
}

wxImagePanel::wxImagePanel(wxFrame *parent,
                           wxString file,
                           int size_x,
                           int size_y,
                           int grid_x,
                           int grid_y,
                           std::function<void(uint8_t, uint8_t)> notify_click)
    : wxPanel(parent), pos_x(0), pos_y(0), grid_x(grid_x), grid_y(grid_y), notify_click(notify_click) {
  // load the file... ideally add a check to see if loading was successful
  wxImage image;
  image.LoadFile(file);
  original_image = image;
  image.Rescale(size_x, size_y, wxIMAGE_QUALITY_NORMAL);
}

wxImagePanel::wxImagePanel::wxImagePanel(wxFrame *parent,
                                         wxString file,
                                         int size_x,
                                         int size_y,
                                         int pos_x,
                                         int pos_y,
                                         int grid_x,
                                         int grid_y,
                                         std::function<void(uint8_t, uint8_t)> notify_click)
    : wxPanel(parent), pos_x(pos_x), pos_y(pos_y), grid_x(grid_x), grid_y(grid_y), notify_click(notify_click) {
  wxImage image;
  image.LoadFile(file);
  original_image = image;
  image.Rescale(size_x, size_y, wxIMAGE_QUALITY_NORMAL);
}


/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void wxImagePanel::paintEvent(wxPaintEvent &evt) {
  // depending on your system you may need to look at double-buffered dcs
  wxPaintDC dc(this);
  render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void wxImagePanel::paintNow() {
  // depending on your system you may need to look at double-buffered dcs
  wxClientDC dc(this);
  render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void wxImagePanel::render(wxDC &dc) {
  if (!original_image.IsOk()) return;

  wxImage image;
  image = original_image;
  if (this->GetSize().GetWidth() > 0 && this->GetSize().GetHeight() > 0) {
    image.Rescale(this->GetSize().GetWidth(), this->GetSize().GetHeight(), wxIMAGE_QUALITY_NORMAL);
    dc.DrawBitmap(image, pos_x, pos_y, false);
  }
}

void wxImagePanel::setRenderPosition(int pos_x, int pos_y) {
  this->pos_x = pos_x;
  this->pos_y = pos_y;
}

int wxImagePanel::getRenderPositionX() {
  return pos_x;
}

int wxImagePanel::getRenderPositionY() {
  return pos_y;
}

void wxImagePanel::mouseDown(wxMouseEvent &event) {
  notify_click(this->grid_x, this->grid_y);
}
