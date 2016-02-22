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

#ifndef TOGGLE_IMAGE_BUTTON_HH
#define TOGGLE_IMAGE_BUTTON_HH

#include <sigc++/sigc++.h>

#include <gtkmm.h>

namespace PF {

class ImageButton: public Gtk::VBox
{
  Gtk::EventBox event_box;
  Gtk::VBox button_box;
  Gtk::Image img, pressed_img;

public:
  sigc::signal<void> signal_clicked;

  ImageButton(Glib::ustring img, Glib::ustring pressed_img);

  void on_realize();
  void on_map();

  // Handlers for the mouse events inside the image area
  bool on_button_press_event( GdkEventButton* button );
  bool on_button_release_event( GdkEventButton* button );
};



class ToggleImageButton: public Gtk::VBox
{
  Gtk::EventBox event_box;
  Gtk::VBox button_box;
  Gtk::Image active_img, inactive_img;

  bool active;
  bool do_toggle;

public:
  sigc::signal<void> signal_clicked;
  sigc::signal<void> signal_activated, signal_deactivated;

  ToggleImageButton(Glib::ustring active, Glib::ustring inactive,
      bool do_toggle=false, bool initial_state=true);

  bool is_active() { return active; }
  void set_active( bool a );

  void set_active_image(Glib::ustring img)
  {
    active_img.set( img );
  }
  void set_inactive_image(Glib::ustring img)
  {
    inactive_img.set( img );
  }
  void set_images(Glib::ustring active, Glib::ustring inactive)
  {
    active_img.set( active );
    inactive_img.set( inactive );
  }

  void toggle();

  // Handlers for the mouse events inside the image area
  bool on_button_press_event( GdkEventButton* button );
  bool on_button_release_event( GdkEventButton* button );
};


}

#endif
