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

#include "../../operations/clip.hh"

#include "clip_config.hh"


PF::ClipConfigGUI::ClipConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Clip" ),
  clip_negative_checkbox( this, "clip_negative", _("clip negative values"), true ),
  clip_overflow_checkbox( this, "clip_overflow", _("clip overflow values"), true )
{
  controlsBox.pack_start( clip_negative_checkbox );
  controlsBox.pack_start( clip_overflow_checkbox );
  
  add_widget( controlsBox );
}



