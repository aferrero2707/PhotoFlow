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

//#include "../../../operations/gmic/smooth_anisotropic.hh"

#include "smooth_anisotropic_config.hh"


PF::GmicSmoothAnisotropicConfigGUI::GmicSmoothAnisotropicConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Smooth [anisotropic] (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_amplitude_slider( this, "amplitude", "amplitude", 60, 0, 1000, 10.00, 100.0, 1),
  prop_sharpness_slider( this, "sharpness", "sharpness", 0.7, 0, 2, .02, .2, 1),
  prop_anisotropy_slider( this, "anisotropy", "anisotropy", 0.3, 0, 1, .01, .1, 1),
  prop_gradient_smoothness_slider( this, "gradient_smoothness", "gradient_smoothness", 0.6, 0, 10, .10, 1.0, 1),
  prop_tensor_smoothness_slider( this, "tensor_smoothness", "tensor_smoothness", 1.1, 0, 10, .10, 1.0, 1),
  prop_spatial_precision_slider( this, "spatial_precision", "spatial_precision", 0.8, 0.1, 2.0, .01, .1, 1),
  prop_angular_precision_slider( this, "angular_precision", "angular_precision", 30, 1, 180, 1.79, 17.9, 1),
  prop_value_precision_slider( this, "value_precision", "value_precision", 2, 0.1, 5, .04, .4, 1),
  prop_interpolation_selector( this, "interpolation", "interpolation", 0),
  prop_fast_approximation_slider( this, "fast_approximation", "fast_approximation", 0, 0, 1, 1, 1, 1),
  prop_padding_slider( this, "padding", "padding", 0, 0, 512, 1, 10, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_amplitude_slider );
  controlsBox.pack_start( prop_sharpness_slider );
  controlsBox.pack_start( prop_anisotropy_slider );
  controlsBox.pack_start( prop_gradient_smoothness_slider );
  controlsBox.pack_start( prop_tensor_smoothness_slider );
  controlsBox.pack_start( prop_spatial_precision_slider );
  controlsBox.pack_start( prop_angular_precision_slider );
  controlsBox.pack_start( prop_value_precision_slider );
  controlsBox.pack_start( prop_interpolation_selector );
  controlsBox.pack_start( prop_fast_approximation_slider );
  controlsBox.pack_start( prop_padding_slider );
  
  add_widget( controlsBox );
}



void PF::GmicSmoothAnisotropicConfigGUI::open()
{
  OperationConfigGUI::open();
}
