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

//#include "../../../operations/gmic/%op_name%.hh"

#include "%op_name%_config.hh"


PF::Gmic%op_name2%ConfigDialog::Gmic%op_name2%ConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, %title% ),
  %widgets_imp%
{
  controlsBox.pack_start( iterations_slider );
  %widgets_imp2%
  
  add_widget( controlsBox );
}



void PF::Gmic%op_name2%ConfigDialog::open()
{
  OperationConfigDialog::open();
}
