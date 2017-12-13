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

#include "textbox.hh"


PF::TextBox::TextBox( OperationConfigGUI* dialog, std::string pname, std::string l, double val ):
  Gtk::HBox(),
  PF::PFWidget( dialog, pname )
{
  label.set_text( l.c_str() );

  pack_start( label );
  pack_start( entry );

  std::ostringstream ostr;
  ostr<<val;
  Glib::ustring strval = ostr.str().c_str();
  entry.set_text( strval );

  entry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));

  show_all_children();
}


PF::TextBox::TextBox( OperationConfigGUI* dialog, std::string pname, std::string l, std::string val ):
  Gtk::HBox(),
  PF::PFWidget( dialog, pname )
{
  label.set_text( l.c_str() );

  pack_start( label );
  pack_start( entry );

  Glib::ustring strval = val.c_str();
  entry.set_text( strval );

  entry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));

  show_all_children();
}


void PF::TextBox::get_value()
{
  if( !get_prop() ) return;
  Glib::ustring val = get_prop()->get_str().c_str();
  entry.set_text( val );
}


void PF::TextBox::set_value()
{
  if( !get_prop() ) return;
  std::string str = entry.get_text().c_str();
  get_prop()->update(str);
}
