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

//#include "../../../operations/gmic/denoise.hh"

#include "denoise_config.hh"


PF::GmicDenoiseConfigDialog::GmicDenoiseConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Patch-based Denoise (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_sigma_s_slider( this, "sigma_s", "sigma_s", 1, 0, 1000000, 10000.00, 100000.0, 1),
  prop_sigma_r_slider( this, "sigma_r", "sigma_r", 1, 0, 1000000, 10000.00, 100000.0, 1),
  prop_psize_slider( this, "psize", "psize", 1, 0, 1000000, 10000.00, 100000.0, 1),
  prop_rsize_slider( this, "rsize", "rsize", 1, 0, 1000000, 10000.00, 100000.0, 1),
  prop_smoothness_slider( this, "smoothness", "smoothness", 1, 0, 1000000, 10000.00, 100000.0, 1),
  prop_is_fast_slider( this, "is_fast", "is_fast", 0, 0, 1, .01, .1, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_sigma_s_slider );
  controlsBox.pack_start( prop_sigma_r_slider );
  controlsBox.pack_start( prop_psize_slider );
  controlsBox.pack_start( prop_rsize_slider );
  controlsBox.pack_start( prop_smoothness_slider );
  controlsBox.pack_start( prop_is_fast_slider );
  
  add_widget( controlsBox );
}



void PF::GmicDenoiseConfigDialog::open()
{
  OperationConfigDialog::open();
}
