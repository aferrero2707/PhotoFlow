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

#include "../../operations/threshold.hh"

#include "threshold_config.hh"


PF::ThresholdConfigGUI::ThresholdConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Threshold" ),
  threshold_slider( this, "threshold", _("threshold"), 100, 0, 100, 2, 10, 100 ),
  invert_checkbox( this, "invert", _("invert"), false )
{
  controlsBox.pack_start( invert_checkbox );
  controlsBox.pack_start( threshold_slider );
  
  add_widget( controlsBox );
}



