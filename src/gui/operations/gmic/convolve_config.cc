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

//#include "../../../operations/gmic/convolve.hh"

#include "convolve_config.hh"


PF::GmicConvolveConfigGUI::GmicConvolveConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Convolve (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_kernel_selector( this, "kernel", "kernel", 0),
  prop_boundary_selector( this, "boundary", "boundary", 1),
  prop_kernel_mul( this, "kernel_mul", "Kernel mult.", 1, 1, 10, 1, 1, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_kernel_selector );
  controlsBox.pack_start( prop_boundary_selector );
  controlsBox.pack_start( prop_kernel_mul );
  
  add_widget( controlsBox );
}



void PF::GmicConvolveConfigGUI::open()
{
  OperationConfigGUI::open();
}
