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

#include "tone_mapping_config.hh"


PF::ToneMappingConfigGUI::ToneMappingConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Tone Mapping" ),
  exposureSlider( this, "exposure", _("exposure"), 0, -10, 10, 0.1, 1 ),
  modeSelector( this, "method", "method: ", 0 ),
  gamma_slider( this, "gamma", _("gamma adjustment"), 1, 1, 5, 0.2, 1, 1 ),
  filmic_A_slider( this, "filmic_A", _("shoulder strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
  filmic_B_slider( this, "filmic_B", _("linear strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
  filmic_C_slider( this, "filmic_C", _("linear angle"), 0.5, 0, 1, 0.02, 0.1, 1 ),
  filmic_D_slider( this, "filmic_D", _("toe strength"), 0.5, 0, 1, 0.02, 0.1, 1 ),
  filmic_E_slider( this, "filmic_E", _("toe num."), 0.5, 0, 0.1, 0.002, 0.01, 1 ),
  filmic_F_slider( this, "filmic_F", _("toe den."), 0.5, 0, 1, 0.02, 0.1, 1 ),
  filmic_W_slider( this, "filmic_W", _("lin. white point"), 10, 1, 100, 2, 10, 1 ),
  lumi_blend_frac_slider( this, "lumi_blend_frac", _("preserve colors"), 1, 0, 1, 0.02, 0.1, 1 )
{

  gammaControlsBox.pack_start( gamma_slider, Gtk::PACK_SHRINK );

  filmicControlsBox.pack_start( filmic_A_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_B_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_C_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_D_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_E_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_F_slider, Gtk::PACK_SHRINK );
  filmicControlsBox.pack_start( filmic_W_slider, Gtk::PACK_SHRINK );

  controlsBox.pack_start( exposureSlider, Gtk::PACK_SHRINK, 10 );
  controlsBox.pack_start( modeSelector, Gtk::PACK_SHRINK, 10 );
  controlsBox.pack_start( gammaControlsBox, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( filmicControlsBox, Gtk::PACK_SHRINK, 0 );

  //controlsBox.pack_end( lumi_blend_frac_slider, Gtk::PACK_SHRINK, 10 );

  add_widget( controlsBox );
}




void PF::ToneMappingConfigGUI::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    OpParBase* par = get_layer()->get_processor()->get_par();
    PropertyBase* prop = par->get_property( "method" );
    if( !prop )  return;

    //std::cout<<"PF::ToneMappingConfigGUI::do_update() called."<<std::endl;

    if( prop->get_enum_value().first != PF::TONE_MAPPING_EXP_GAMMA &&
        gammaControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( gammaControlsBox );

    if( prop->get_enum_value().first != PF::TONE_MAPPING_FILMIC &&
        filmicControlsBox.get_parent() == &controlsBox )
      controlsBox.remove( filmicControlsBox );

    switch( prop->get_enum_value().first ) {
    case PF::TONE_MAPPING_EXP_GAMMA:
      controlsBox.pack_start( gammaControlsBox, Gtk::PACK_SHRINK );
      gammaControlsBox.show();
      break;
    case PF::TONE_MAPPING_FILMIC:
      controlsBox.pack_start( filmicControlsBox, Gtk::PACK_SHRINK );
      filmicControlsBox.show();
      break;
    }
  }
  controlsBox.show_all_children();

  OperationConfigGUI::do_update();
}

