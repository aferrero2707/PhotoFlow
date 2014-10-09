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

#include "../../../operations/gmic/gmic.hh"

#include "gmic_config.hh"


PF::GMicConfigDialog::GMicConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "G'MIC Command Interface" ),
  iterationsSlider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  paddingSlider( this, "padding", "Tiles overlap", 0, 0, 1000, 1, 5, 1),
  xscaleSlider( this, "x_scale", "X scale factor", 1, 0, 100, 0.1, 1, 1),
  yscaleSlider( this, "y_scale", "Y scale factor", 1, 0, 100, 0.1, 1, 1)
{
  commandLabel.set_text( "G'MIC command:" );
  commandVBox.pack_start( commandLabel );
  commandVBox.pack_start( commandFileEntry );
  controlsBox.pack_start( commandVBox );

  controlsBox.pack_start( iterationsSlider );
  controlsBox.pack_start( xscaleSlider );
  controlsBox.pack_start( yscaleSlider );
  controlsBox.pack_start( paddingSlider );
  
  add_widget( controlsBox );

  commandFileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &GMicConfigDialog::on_command_changed));
}



void PF::GMicConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    //radiusSlider.init();
    PF::GMicPar* par = 
      dynamic_cast<PF::GMicPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      PropertyBase* prop = par->get_property( "command" );
      if( prop ) {
        PropertyBase* prop2 = dynamic_cast< PF::Property<std::string>* >( prop );
        if( prop2 ) {
          std::string c = prop2->get_str();
          //prop2->get( c );
          commandFileEntry.set_text( c.c_str() );
        }
      }
    }
  }
  OperationConfigDialog::open();
}



void PF::GMicConfigDialog::on_command_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = commandFileEntry.get_text();
    if( filename.empty() )
      return;
    PF::GMicPar* par = 
      dynamic_cast<PF::GMicPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "command" );
    if( !prop ) 
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}
