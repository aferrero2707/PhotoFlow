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

#include "desaturate_config.hh"


PF::DesaturateConfigDialog::DesaturateConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Desaturate" ),
  modeSelector( this, "method", "Desaturate method: ", 0 )
{
  controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK );
  add_widget( controlsBox );

  show_all_children();
}




void PF::DesaturateConfigDialog::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    OpParBase* par = get_layer()->get_processor()->get_par();
    PropertyBase* prop = par->get_property( "method" );
    if( !prop )  return;
  }
  controlsBox.show_all_children();

  OperationConfigDialog::do_update();
}

