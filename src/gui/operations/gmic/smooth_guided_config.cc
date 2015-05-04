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

//#include "../../../operations/gmic/smooth_guided.hh"

#include "smooth_guided_config.hh"


PF::GmicSmoothGuidedConfigDialog::GmicSmoothGuidedConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Smooth [guided] (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_radius_slider( this, "radius", "radius", 5, 1, 100, 1, 5, 1),
  prop_smoothness_slider( this, "smoothness", "smoothness", 30, 0, 512, 5.12, 51.2, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_radius_slider );
  controlsBox.pack_start( prop_smoothness_slider );
  
  add_widget( controlsBox );
}



void PF::GmicSmoothGuidedConfigDialog::open()
{
  OperationConfigDialog::open();
}
