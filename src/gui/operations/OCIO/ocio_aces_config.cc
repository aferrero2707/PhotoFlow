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

#include "../../../operations/OCIO/ocio_aces.hh"

#include "ocio_aces_config.hh"


PF::OCIOACESConfigGUI::OCIOACESConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "ACES (OCIO)" ),
  view_selector( this, "view_name", _("view: "), 0 )
{
  controlsBox.pack_start( view_selector, Gtk::PACK_SHRINK, 5 );
  add_widget( controlsBox );
}
