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

//#include "../../../operations/gmic/smooth_total_variation.hh"

#include "smooth_nlmeans_config.hh"


PF::GmicSmoothNonLocalMeansConfigGUI::GmicSmoothNonLocalMeansConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Smooth [non-local means] (G'MIC)"  ),
  //iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_radius_slider( this, "radius", _("radius"), 4, 0, 10, 0.5, 2, 1),
  prop_size_slider( this, "size", _("size"), 1, 0, 10, 0.5, 2, 1),
  prop_padding_slider( this, "padding", "padding", 1, 0, 100, 1, 5, 1)
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_radius_slider );
  controlsBox.pack_start( prop_size_slider );
  controlsBox.pack_start( prop_padding_slider );
  
  add_widget( controlsBox );
}



void PF::GmicSmoothNonLocalMeansConfigGUI::open()
{
  OperationConfigGUI::open();
}
