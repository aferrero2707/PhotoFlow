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
	//modeSelector( this, "nr_mode", "N.R. mode: ", PF_NR_ANIBLUR ),
  impulse_nr_enable( this, "impulse_nr_enable", _("impulse NR"), false ),
  impulse_nr_threshold( this, "impulse_nr_threshold", _("threshold"), 50, 0, 100, 5, 20, 1),
  nlmeans_enable( this, "nlmeans_enable", _("non-local means"), false ),
  nlmeans_radius( this, "nlmeans_radius", _("radius"), 2, 1, 10, 1, 2, 1),
  nlmeans_strength( this, "nlmeans_strength", _("strength"), 50, 0, 100, 5, 20, 1),
  nlmeans_luma_frac( this, "nlmeans_luma_frac", _("luma"), 50, 0, 100, 5, 20, 100),
  nlmeans_chroma_frac( this, "nlmeans_chroma_frac", _("chroma"), 100, 0, 100, 5, 20, 100),
  iterationsSlider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  amplitudeSlider( this, "amplitude", "Amplitude", 1, 0, 100, 1, 1, 1),
  sharpnessSlider( this, "sharpness", "Sharpness", 1, 0, 10, 0.1, 1, 1),
  anisotropySlider( this, "anisotropy", "Anisotropy", 1, 0, 10, 0.1, 1, 1),
  alphaSlider( this, "alpha", "Alpha", 1, 0, 10, 0.1, 1, 1),
  sigmaSlider( this, "sigma", "Sigma", 1, 0, 10, 0.1, 1, 1)
{
  //controlsBox.pack_start( modeSelector );
  controlsBox.pack_start( impulse_nr_enable );
  controlsBox.pack_start( impulse_nr_threshold );
  controlsBox.pack_start( hline1 );

  controlsBox.pack_start( nlmeans_enable );
  controlsBox.pack_start( nlmeans_radius );
  controlsBox.pack_start( nlmeans_strength );
  controlsBox.pack_start( nlmeans_luma_frac );
  controlsBox.pack_start( nlmeans_chroma_frac );
  //controlsBox.pack_start( amplitudeSlider );
  //controlsBox.pack_start( sharpnessSlider );
  //controlsBox.pack_start( anisotropySlider );
  //controlsBox.pack_start( alphaSlider );
  //controlsBox.pack_start( sigmaSlider );
  
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
