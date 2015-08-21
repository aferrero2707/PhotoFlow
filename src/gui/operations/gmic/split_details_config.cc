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

#include "../../../operations/gmic/split_details.hh"

#include "split_details_config.hh"


PF::GmicSplitDetailsConfigGUI::GmicSplitDetailsConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Multi-Scale Decomposition"  ),
  updateButton( "Update" ),
  prop_nscales_slider( this, "nscales", "nscales", 4, 1, 10, 1, 5, 1),
  prop_base_scale_slider( this, "base_scale", "base scale %", 1, 0, 1000000, .01, 1, 1),
  prop_detail_scale_slider( this, "detail_scale", "detail scale %", 0.01, 0, 1000000, .01, 1, 1)
{
  controlsBox.pack_start( updateButton );
  controlsBox.pack_start( prop_base_scale_slider );
  controlsBox.pack_start( prop_detail_scale_slider );
  
  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicSplitDetailsConfigGUI::on_update) );
  
  add_widget( controlsBox );
}



void PF::GmicSplitDetailsConfigGUI::on_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicSplitDetailsPar* par = dynamic_cast<GmicSplitDetailsPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}


void PF::GmicSplitDetailsConfigGUI::open()
{
  OperationConfigGUI::open();
}
