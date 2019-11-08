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

#include "sharpen_config.hh"


PF::SharpenConfigGUI::SharpenConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Sharpen" ),
modeSelector( this, "method", "Sharpen method: ", 0 ),
usmRadiusSlider( this, "usm_radius", "Radius", 1, 0, 100, 0.05, 0.1, 1),
eusmAmountSlider( this, "eusm_amount", "amount", 1, 0, 500, 0.5, 5, 100),
eusmIterationsSlider( this, "eusm_iterations", "iterations", 1, 0, 10, 1, 5, 1),
eusmRadiusSlider( this, "eusm_radius", "radius", 1, 0, 10, 1, 5, 1),
eusmThresholdLSlider( this, "eusm_threshold_l", "noise threshold", 1, 0, 100, 0.5, 5, 100000),
eusmThresholdHSlider( this, "eusm_threshold_h", "edge threshold", 1, 0, 100, 0.5, 5, 5000),
eusmShowMaskCbox(this, "eusm_show_mask", "show mask", true),
eusmLinearCbox(this, "eusm_linear", "linear", true),
rlSigmaSlider( this, "rl_sigma", "Sigma", 1, 0, 100, 0.05, 0.1, 1),
rlIterationsSlider( this, "rl_iterations", "Iterations", 10, 1, 100, 1, 5, 1),
textureStrengthSlider( this, "texture_strength", _("strength"), 1, 0, 4, 0.05, 0.5, 1),
textureRadiusSlider( this, "texture_radius", _("radius"), 4, 0, 32, 0.1, 1, 1)
{
  controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK );

  usmControlsBox.pack_start( usmRadiusSlider, Gtk::PACK_SHRINK );

  eusmControlsBox.pack_start( eusmAmountSlider, Gtk::PACK_SHRINK );
  //eusmControlsBox.pack_start( eusmIterationsSlider, Gtk::PACK_SHRINK );
  eusmControlsBox.pack_start( eusmRadiusSlider, Gtk::PACK_SHRINK );
  eusmControlsBox.pack_start( eusmThresholdLSlider, Gtk::PACK_SHRINK );
  eusmControlsBox.pack_start( eusmThresholdHSlider, Gtk::PACK_SHRINK );
  eusmControlsBox.pack_start( eusmShowMaskCbox, Gtk::PACK_SHRINK );
  //eusmControlsBox.pack_start( eusmLinearCbox, Gtk::PACK_SHRINK );

  rlControlsBox.pack_start( rlSigmaSlider, Gtk::PACK_SHRINK );
  rlControlsBox.pack_start( rlIterationsSlider, Gtk::PACK_SHRINK );

  textureControlsBox.pack_start( textureStrengthSlider, Gtk::PACK_SHRINK );
  textureControlsBox.pack_start( textureRadiusSlider, Gtk::PACK_SHRINK );

  add_widget( controlsBox );

  get_main_box().show_all_children();
}




void PF::SharpenConfigGUI::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    OpParBase* par = get_layer()->get_processor()->get_par();
    PropertyBase* prop = par->get_property( "method" );
    if( !prop )  return;

    if( usmControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( usmControlsBox );
    if( eusmControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( eusmControlsBox );
    if( rlControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( rlControlsBox );
    if( textureControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( textureControlsBox );

    switch( prop->get_enum_value().first ) {
    case PF::SHARPEN_USM:
      controlsBox.pack_start( usmControlsBox, Gtk::PACK_SHRINK );
      usmControlsBox.show();
      break;
    case PF::SHARPEN_EUSM:
      controlsBox.pack_start( eusmControlsBox, Gtk::PACK_SHRINK );
      eusmControlsBox.show();
      break;
    case PF::SHARPEN_DECONV:
      controlsBox.pack_start( rlControlsBox, Gtk::PACK_SHRINK );
      rlControlsBox.show();
      break;
    case PF::SHARPEN_TEXTURE:
      controlsBox.pack_start( textureControlsBox, Gtk::PACK_SHRINK );
      textureControlsBox.show();
      break;
		}
  }
  controlsBox.show_all_children();

  OperationConfigGUI::do_update();
}

