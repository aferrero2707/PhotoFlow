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

//#include "../../../operations/gmic/smooth_diffusion.hh"

#include "smooth_diffusion_config.hh"


PF::GmicSmoothDiffusionConfigDialog::GmicSmoothDiffusionConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Smooth [diffusion] (G'MIC)"  ),
  //iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_iterations_slider( this, "iterations", "iterations", 8, 1, 100, .99, 9.9, 1),
  prop_sharpness_slider( this, "sharpness", "sharpness", 0.7, 0, 2, .02, .2, 1),
  prop_anisotropy_slider( this, "anisotropy", "anisotropy", 0.3, 0, 1, .01, .1, 1),
  prop_gradient_smoothness_slider( this, "gradient_smoothness", "gradient_smoothness", 0.6, 0, 10, .10, 1.0, 1),
  prop_tensor_smoothness_slider( this, "tensor_smoothness", "tensor_smoothness", 1.1, 0, 10, .10, 1.0, 1),
  prop_time_step_slider( this, "time_step", "time_step", 15, 5, 50, .45, 4.5, 1)
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_iterations_slider );
  controlsBox.pack_start( prop_sharpness_slider );
  controlsBox.pack_start( prop_anisotropy_slider );
  controlsBox.pack_start( prop_gradient_smoothness_slider );
  controlsBox.pack_start( prop_tensor_smoothness_slider );
  controlsBox.pack_start( prop_time_step_slider );
  
  add_widget( controlsBox );
}



void PF::GmicSmoothDiffusionConfigDialog::open()
{
  OperationConfigDialog::open();
}
