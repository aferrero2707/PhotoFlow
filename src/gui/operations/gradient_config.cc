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

#include "gradient_config.hh"


PF::GradientConfigDialog::GradientConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Gradient tool" ),
  typeSelector( this, "gradient_type", "Gradient type: ", 1 ),
  invert_box( this, "invert", "Invert", true ),
  center_x( this, "gradient_center_x", "Center X (%)", 100, 0, 100, 1, 10, 100),
  center_y( this, "gradient_center_y", "Center Y (%)", 100, 0, 100, 1, 10, 100)
{
  hbox.pack_start( typeSelector );
  hbox.pack_start( invert_box, Gtk::PACK_SHRINK );
  add_widget( hbox );
  add_widget( center_x );
  add_widget( center_y );
}
