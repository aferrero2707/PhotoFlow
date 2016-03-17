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

#ifndef NUMENTRY_HH
#define NUMENTRY_HH

#include <gtkmm.h>

#include "pfwidget.hh"
#include "imagebutton.hh"

namespace PF {

class NumEntry: public Gtk::HBox
{
  int digits;
  bool inhibited;
  Gtk::Entry entry;
  //Gtk::Image step_up, step_down;
  ImageButton button_step_up, button_step_down;
  Gtk::VBox button_box;
#ifdef GTKMM_2
    Gtk::Adjustment* adjustment;
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::Adjustment> adjustment;
#endif
public:
    NumEntry();

#ifdef GTKMM_2
    void set_adjustment( Gtk::Adjustment* a );
#endif
#ifdef GTKMM_3
    void set_adjustment( Glib::RefPtr<Gtk::Adjustment> a );
#endif

    bool on_key_press_or_release_event(GdkEventKey* event);
    bool my_on_focus_out(GdkEventFocus *focus)
    {
      text_changed();
      return false;
    }

    void step_up();
    void step_down();

    float get_value() { return adjustment->get_value(); }
    float set_value(float val) { adjustment->set_value(val); changed(); }

    Glib::SignalProxy0<void> signal_changed() { return adjustment->signal_value_changed(); }

    void changed();
    void text_changed();
};

}

#endif
