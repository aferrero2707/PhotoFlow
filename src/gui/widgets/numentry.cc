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

#include "numentry.hh"


PF::NumEntry::NumEntry():
Gtk::HBox(),
//step_up(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-down-small.png"),
//step_down(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-down-small.png"),
button_step_up(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-up-small.png", PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-up-small-pressed.png"),
button_step_down(PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-down-small.png", PF::PhotoFlow::Instance().get_data_dir()+"/icons/libre-chevron-down-small-pressed.png"),
digits(1), inhibited(false)
{
  pack_start( entry, Gtk::PACK_SHRINK );
  pack_start( button_box, Gtk::PACK_SHRINK );

  //button_step_up.add( step_up );
  //button_step_up.set_relief (Gtk::RELIEF_NONE);
  //button_step_up.set_border_width (0);
  //button_step_up.set_can_focus(false);
  //button_step_down.add( step_down );
  //button_step_down.set_relief (Gtk::RELIEF_NONE);
  //button_step_down.set_border_width (0);
  //button_step_down.set_can_focus(false);

  button_step_up.set_size_request( 20, -1 );
  button_step_down.set_size_request( 20, -1 );
  button_box.pack_start( button_step_up, Gtk::PACK_EXPAND_WIDGET );
  button_box.pack_start( button_step_down, Gtk::PACK_EXPAND_WIDGET );

  entry.signal_activate().connect( sigc::mem_fun(*this,&PF::NumEntry::text_changed) );

  entry.signal_focus_out_event().connect( sigc::mem_fun(*this,&PF::NumEntry::my_on_focus_out) );

  entry.signal_key_press_event().connect( sigc::mem_fun(*this,&PF::NumEntry::on_key_press_or_release_event), false );
  entry.signal_key_release_event().connect( sigc::mem_fun(*this,&PF::NumEntry::on_key_press_or_release_event), false );
  entry.add_events( Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::FOCUS_CHANGE_MASK );
  entry.set_width_chars(5);
  entry.set_has_frame( false );

  button_step_up.signal_clicked.connect(sigc::mem_fun(*this,
        &PF::NumEntry::step_up) );
  button_step_down.signal_clicked.connect(sigc::mem_fun(*this,
        &PF::NumEntry::step_down) );

}


bool PF::NumEntry::on_key_press_or_release_event(GdkEventKey* event)
{
  if (event->type == GDK_KEY_PRESS &&
      (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) == 0) {
    if( (event->keyval == GDK_KEY_Up) ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      float new_val = adjustment->get_value();
      new_val += adjustment->get_step_increment();
      adjustment->set_value( new_val );
      return true;
    }
    if( event->keyval == GDK_KEY_Down ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      float new_val = adjustment->get_value();
      new_val -= adjustment->get_step_increment();
      adjustment->set_value( new_val );
      return true;
    }
    if( event->keyval == GDK_KEY_Page_Up ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      float new_val = adjustment->get_value();
      new_val += adjustment->get_page_increment();
      adjustment->set_value( new_val );
      return true;
    }
    if( event->keyval == GDK_KEY_Page_Down ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      float new_val = adjustment->get_value();
      new_val -= adjustment->get_page_increment();
      adjustment->set_value( new_val );
      return true;
    }
  }
  return false;
}


void PF::NumEntry::step_up()
{
  double new_val = adjustment->get_value();
  new_val += adjustment->get_step_increment();
  adjustment->set_value( new_val );
}


void PF::NumEntry::step_down()
{
  double new_val = adjustment->get_value();
  new_val -= adjustment->get_step_increment();
  adjustment->set_value( new_val );
}


#ifdef GTKMM_2
void PF::NumEntry::set_adjustment( Gtk::Adjustment* a )
#endif
#ifdef GTKMM_3
void PF::NumEntry::set_adjustment( Glib::RefPtr<Gtk::Adjustment> a )
#endif
{
  adjustment = a;
  adjustment->signal_value_changed().
      connect(sigc::mem_fun(*this,
          &NumEntry::changed));
  changed();
}


void PF::NumEntry::changed()
{
  inhibited = true;
  std::ostringstream str;
  str << adjustment->get_value();
  entry.set_text( str.str() );
  inhibited = false;
}


void PF::NumEntry::text_changed()
{
  if( inhibited ) return;
  std::istringstream str( entry.get_text() );
  float val;
  str >> val;
  adjustment->set_value( val );
}


