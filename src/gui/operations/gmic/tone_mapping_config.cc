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

#include "../../../operations/gmic/tone_mapping.hh"

#include "tone_mapping_config.hh"


PF::GmicToneMappingConfigGUI::GmicToneMappingConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Tone mapping (G'MIC)"  ),
  updateButton( "Update" ),
  prop_threshold_slider( this, "threshold", "threshold", 0.5, 0, 1, .01, .1, 1),
  prop_gamma_slider( this, "gamma", "gamma", 0.7, 0, 1, .01, .1, 1),
  prop_smoothness_slider( this, "smoothness", "smoothness", 0.1, 0, 10, .10, 1.0, 1),
  prop_iterations_slider( this, "iterations", "iterations", 30, 0, 500, 1, 5, 1),
  prop_channels_selector( this, "channels", "channels", 3),
  prop_padding_slider( this, "padding", "padding", 20, 0, 500, 1, 5, 1)
{
  controlsBox.pack_start( updateButton );
  controlsBox.pack_start( prop_threshold_slider );
  controlsBox.pack_start( prop_gamma_slider );
  controlsBox.pack_start( prop_smoothness_slider );
  controlsBox.pack_start( prop_iterations_slider );
  controlsBox.pack_start( prop_channels_selector );
  
  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicToneMappingConfigGUI::on_update) );
  
  add_widget( controlsBox );
}



void PF::GmicToneMappingConfigGUI::on_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicToneMappingPar* par = dynamic_cast<GmicToneMappingPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}


void PF::GmicToneMappingConfigGUI::open()
{
  OperationConfigGUI::open();
}
