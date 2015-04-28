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


#ifndef TAB_LABEL_WIDGET__HH
#define TAB_LABEL_WIDGET__HH

#include <gtkmm.h>


class VTabLabelWidget: public Gtk::VBox
{
  Gtk::Image image;
  Gtk::Button button;
  Gtk::Label label;
  Gtk::Widget* widget;

public:
  VTabLabelWidget( Glib::ustring l, Gtk::Widget* w ):
    VBox(),
    image(),
    button(),
    label( l ),
    widget( w )
  {
    image.set_from_icon_name( "window-close", Gtk::ICON_SIZE_MENU );
    button.add( image );
    button.set_relief( Gtk::RELIEF_NONE );
    label.set_angle(90);
    pack_start( button, Gtk::PACK_SHRINK );
    pack_start( label, Gtk::PACK_SHRINK );

    button.signal_clicked().connect( sigc::mem_fun(*this,
                                                   &VTabLabelWidget::on_close_button_clicked) );

    show_all();
  }

  void on_close_button_clicked() 
  {
    signal_close.emit( widget );
  }

  sigc::signal< void, Gtk::Widget* > signal_close;
};


class HTabLabelWidget: public Gtk::HBox
{
  Gtk::Image image;
  Gtk::Button button;
  Gtk::Label label;
  Gtk::Widget* widget;

public:
  HTabLabelWidget( Glib::ustring l, Gtk::Widget* w ):
    HBox(),
    image(),
    button(),
    label( l ),
    widget( w )
  {
    image.set_from_icon_name( "window-close", Gtk::ICON_SIZE_MENU );
    button.add( image );
    button.set_relief( Gtk::RELIEF_NONE );
    pack_start( label, Gtk::PACK_SHRINK );
    pack_start( button, Gtk::PACK_SHRINK );

    button.signal_clicked().connect( sigc::mem_fun(*this,
                                                   &HTabLabelWidget::on_close_button_clicked) );

    show_all();
  }

  void on_close_button_clicked() 
  {
    signal_close.emit( widget );
  }

  void set_label( Glib::ustring l ) { label.set_text( l ); }
  Glib::ustring get_label() { return label.get_text(); }

  sigc::signal< void, Gtk::Widget* > signal_close;
};


#endif
