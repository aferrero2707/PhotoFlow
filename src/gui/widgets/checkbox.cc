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

#include "checkbox.hh"


PF::CheckBox::CheckBox( OperationConfigDialog* dialog, std::string pname, std::string l, int val ):
  Gtk::HBox(),
  PF::PFWidget( dialog, pname )
{
  label.set_text( l.c_str() );

  pack_start( label );
  pack_start( check );

  check.set_active( val!=0 );

  check.signal_toggled().
    connect(sigc::mem_fun(*this,
			  &PFWidget::changed));

  show_all_children();
}


void PF::CheckBox::get_value()
{
  if( !get_prop() ) return;
  std::string str = get_prop()->get_str().c_str();
  check.set_active( str=="true" );
}


void PF::CheckBox::set_value()
{
  if( !get_prop() ) return;
  //std::string str = entry.get_text().c_str();
  //get_prop()->update(str);
}
