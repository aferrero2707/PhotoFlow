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

//#include "../../../operations/gmic/gradient_norm.hh"

#include "gradient_norm_config.hh"


PF::GmicGradientNormConfigGUI::GmicGradientNormConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Gradient Norm (G'MIC)"  ),
  //iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_smoothness_slider( this, "smoothness", "Smoothness", 0, 0, 10, .10, 1.0, 1),
  prop_linearity_slider( this, "linearity", "linearity", 0.5, 0, 1.5, .01, .1, 1),
  prop_min_threshold_slider( this, "min_threshold", "Threshold", 0, 0, 100, 1.00, 10.0, 1),
  prop_max_threshold_slider( this, "max_threshold", "Multiplier", 100, 0, 100, 1.00, 10.0, 1)
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_smoothness_slider );
  controlsBox.pack_start( prop_linearity_slider );
  controlsBox.pack_start( prop_min_threshold_slider );
  controlsBox.pack_start( prop_max_threshold_slider );
  
  add_widget( controlsBox );
}



void PF::GmicGradientNormConfigGUI::open()
{
  OperationConfigGUI::open();
}
