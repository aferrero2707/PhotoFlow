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

//#include "../../../operations/gmic/smooth_median.hh"

#include "smooth_median_config.hh"


PF::GmicSmoothMedianConfigDialog::GmicSmoothMedianConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Smooth [median] (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_radius_slider( this, "radius", "radius", 3, 1, 20, .19, 1.9, 1),
  prop_threshold_slider( this, "threshold", "threshold", 255, 0, 255, 2.55, 25.5, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_radius_slider );
  controlsBox.pack_start( prop_threshold_slider );
  
  add_widget( controlsBox );
}



void PF::GmicSmoothMedianConfigDialog::open()
{
  OperationConfigDialog::open();
}
