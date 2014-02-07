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

#include "imageread_config.hh"


PF::ImageReadConfigDialog::ImageReadConfigDialog():
  OperationConfigDialog("Open an image" )
{
  label.set_text( "file name:" );

  controlsBox.pack_start( label );
  controlsBox.pack_start( fileEntry );
  
  add_widget( controlsBox );

  fileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &ImageReadConfigDialog::on_filename_changed));
}


void PF::ImageReadConfigDialog::on_filename_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::cout<<"New image file name: "<<fileEntry.get_text()<<std::endl;
    PF::ImageReaderPar* par = 
      dynamic_cast<PF::ImageReaderPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      get_layer()->set_dirty( true );
      std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
  }
}


void PF::ImageReadConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::ImageReaderPar* par = 
      dynamic_cast<PF::ImageReaderPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      //brightnessAdj.set_value( par->get_brightness() );
      //contrastAdj.set_value( par->get_contrast() );
    }
  }
  OperationConfigDialog::open();
}
