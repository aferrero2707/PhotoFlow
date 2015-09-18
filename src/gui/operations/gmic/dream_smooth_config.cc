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

#include "dream_smooth_config.hh"

#include "../../../operations/gmic/dream_smooth.hh"


PF::GmicDreamSmoothConfigGUI::GmicDreamSmoothConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Dream Smoothing (G'MIC)"  ),
  updateButton( "Update" ),
  iterations_slider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  prop_interations_slider( this, "smooth_interations", "Smooth interations", 1, 1, 10, 1, 5, 1),
  prop_equalize_slider( this, "equalize", "equalize", 0, 0, 1, 1, 5, 1),
  prop_merging_option_selector( this, "merging_option", "merging_option", 1),
  prop_opacity_slider( this, "opacity", "opacity", 0.8, 0, 1, .01, .1, 1),
  prop_reverse_slider( this, "reverse", "reverse", 0, 0, 1, 1, 5, 1),
  prop_smoothness_slider( this, "smoothness", "smoothness", 0.8, 0, 5, .05, .5, 1),
  paddingSlider( this, "padding", "Tiles overlap", 0, 0, 1000, 1, 5, 1)
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( updateButton );
  prop_interations_slider.set_passive( true );
  controlsBox.pack_start( prop_interations_slider );
  prop_equalize_slider.set_passive( true );
  controlsBox.pack_start( prop_equalize_slider );
  prop_merging_option_selector.set_passive( true );
  controlsBox.pack_start( prop_merging_option_selector );
  prop_opacity_slider.set_passive( true );
  controlsBox.pack_start( prop_opacity_slider );
  prop_reverse_slider.set_passive( true );
  controlsBox.pack_start( prop_reverse_slider );
  prop_smoothness_slider.set_passive( true );
  controlsBox.pack_start( prop_smoothness_slider );
  //controlsBox.pack_start( paddingSlider );

  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicDreamSmoothConfigGUI::on_update) );
  
  add_widget( controlsBox );
}



void PF::GmicDreamSmoothConfigGUI::on_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicDreamSmoothPar* par = dynamic_cast<GmicDreamSmoothPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}


void PF::GmicDreamSmoothConfigGUI::open()
{
  OperationConfigGUI::open();
}
