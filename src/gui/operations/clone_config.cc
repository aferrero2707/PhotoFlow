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

#include "clone_config.hh"


PF::CloneConfigGUI::CloneConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Clone layer" ),
  layer_list( this, "Layer name:"),
  sourceSelector( this, "source_channel", _("Source channel: "), 1 )
{
  add_widget( layer_list );
  add_widget( sourceSelector );

  //fileEntry.signal_activate().
  //  connect(sigc::mem_fun(*this,
  //			  &CloneConfigGUI::on_filename_changed));
}


void PF::CloneConfigGUI::on_layer_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
  }
}


void PF::CloneConfigGUI::do_update()
{
  layer_list.update_model();
  OperationConfigGUI::do_update();
}


void PF::CloneConfigGUI::init()
{
  layer_list.update_model();
  OperationConfigGUI::init();
}
