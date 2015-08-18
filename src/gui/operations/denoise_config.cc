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

#include "../../operations/denoise.hh"

#include "denoise_config.hh"


PF::DenoiseConfigGUI::DenoiseConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Noise Reduction" ),
	modeSelector( this, "nr_mode", "N.R. mode: ", PF_NR_ANIBLUR ),
  iterationsSlider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  amplitudeSlider( this, "amplitude", "Amplitude", 1, 0, 100, 1, 1, 1),
  sharpnessSlider( this, "sharpness", "Sharpness", 1, 0, 10, 0.1, 1, 1),
  anisotropySlider( this, "anisotropy", "Anisotropy", 1, 0, 10, 0.1, 1, 1),
  alphaSlider( this, "alpha", "Alpha", 1, 0, 10, 0.1, 1, 1),
  sigmaSlider( this, "sigma", "Sigma", 1, 0, 10, 0.1, 1, 1)
{
  controlsBox.pack_start( modeSelector );
  controlsBox.pack_start( iterationsSlider );
  controlsBox.pack_start( amplitudeSlider );
  controlsBox.pack_start( sharpnessSlider );
  controlsBox.pack_start( anisotropySlider );
  controlsBox.pack_start( alphaSlider );
  controlsBox.pack_start( sigmaSlider );
  
  add_widget( controlsBox );
}



void PF::DenoiseConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    //radiusSlider.init();
  }
  OperationConfigGUI::open();
}
