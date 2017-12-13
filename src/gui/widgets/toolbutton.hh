/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#ifndef TOOL_BUTTON_HH
#define TOOL_BUTTON_HH

#include <sigc++/sigc++.h>

#include <gtkmm.h>

#include "../../base/image.hh"

namespace PF {

class LayerWidget;

  class ToolButton: public Gtk::VBox
  {
    Gtk::EventBox event_box;
    Gtk::VBox button_box;
    Gtk::Image img;

    Image* image;
    LayerWidget* layer_widget;

    Glib::ustring toolname;

  public:
    sigc::signal<void> signal_clicked;

    ToolButton(Glib::ustring imgname, Glib::ustring toolname, Image* image, LayerWidget* layer_widget);

    void set_image(Glib::ustring i)
    {
      img.set( i );
    }

    // Handlers for the mouse events inside the image area
    bool on_button_press_event( GdkEventButton* button );
    bool on_button_release_event( GdkEventButton* button );

    void add_layer();
  };


}

#endif
