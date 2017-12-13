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

#include "../../operations/brightness_contrast.hh"

#include "brightness_contrast_config.hh"


PF::BrightnessContrastConfigGUI::BrightnessContrastConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Brightness/Contrast Adjustment" ),
  //brightnessAdj( 0, -1, 1, 0.05, 0.2, 0),
  //contrastAdj( 0, -1, 1, 0.05, 0.2, 0),
  //brightnessScale(brightnessAdj),
  //contrastScale(contrastAdj)
  brightnessSlider( this, "brightness", _("Brightness"), 0, -1, 1, 0.05, 0.2, 1),
  contrastSlider( this, "contrast", _("Contrast"), 0, -1, 1, 0.05, 0.2, 1),
  outputModeSlider( this, "color_blend", _("Output mode"), 0, -1, 1, 0.05, 0.2, 1)
{
  //frame.add( controlsBox );

  padding1.set_size_request( 2, 20 );
  padding2.set_size_request( 2, 10 );
  padding3.set_size_request( 2, 10 );

  add_widget( padding1 );

  controlsBox.pack_start( brightnessSlider );
  controlsBox.pack_start( contrastSlider );
  add_widget( controlsBox );

  //add_widget( padding2 );
  //add_widget( hline );
  //add_widget( padding3 );

  padding4.set_size_request( 10, 2 );
  outputModeBox.pack_start( outputModeSlider, Gtk::PACK_SHRINK );
  outputModeBox.pack_start( padding4, Gtk::PACK_EXPAND_WIDGET  );
  add_widget( outputModeBox );
}



void PF::BrightnessContrastConfigGUI::open()
{
//  if( get_layer() && get_layer()->get_image() &&
//      get_layer()->get_processor() &&
//      get_layer()->get_processor()->get_par() ) {
//    brightnessSlider.init();
//    contrastSlider.init();
//  }
  OperationConfigGUI::open();
}
