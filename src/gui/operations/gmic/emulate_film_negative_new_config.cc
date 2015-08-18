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

//#include "../../../operations/gmic/emulate_film_negative_new.hh"

#include "emulate_film_negative_new_config.hh"


PF::GmicEmulateFilmNegativeNewConfigGUI::GmicEmulateFilmNegativeNewConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Film Emulation [negative new] (G'MIC)"  ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_preset_selector( this, "preset", "preset", 0),
  prop_effect_selector( this, "effect", "effect", 1),
  prop_opacity_slider( this, "opacity", "opacity", 1, 0, 1, .01, .1, 1),
  prop_gamma_slider( this, "gamma", "gamma", 0, -1.2, 1.2, .02, .2, 1),
  prop_contrast_slider( this, "contrast", "contrast", 1, 0, 4, .04, .4, 1),
  prop_brightness_slider( this, "brightness", "brightness", 0, -255, 255, 5.10, 51.0, 1),
  prop_hue_slider( this, "hue", "hue", 0, -180, 180, 3.60, 36.0, 1),
  prop_saturation_slider( this, "saturation", "saturation", 0, -1, 1, .02, .2, 1),
  prop_post_normalize_slider( this, "post_normalize", "post_normalize", 0, 0, 1, 1, 5, 1)
{
  controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( prop_preset_selector );
  controlsBox.pack_start( prop_effect_selector );
  controlsBox.pack_start( prop_opacity_slider );
  controlsBox.pack_start( prop_gamma_slider );
  controlsBox.pack_start( prop_contrast_slider );
  controlsBox.pack_start( prop_brightness_slider );
  controlsBox.pack_start( prop_hue_slider );
  controlsBox.pack_start( prop_saturation_slider );
  controlsBox.pack_start( prop_post_normalize_slider );
  
  add_widget( controlsBox );
}



void PF::GmicEmulateFilmNegativeNewConfigGUI::open()
{
  OperationConfigGUI::open();
}
