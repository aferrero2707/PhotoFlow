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

#include "watermark_fourier_config.hh"
#include "../../operations/gmic/watermark_fourier.hh"


PF::GmicWatermarkFourierConfigGUI::GmicWatermarkFourierConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Digital Watermark" ),
  updateButton( "Update" ),
  textBox( this, "text", "Text: ", "Watermark" ),
  textsizeSlider( this, "text_size", "Text size: ", 53, 13, 128, 1, 5, 1 )
{
  //add_widget( updateButton );
  add_widget( textBox );
  add_widget( textsizeSlider );

  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicWatermarkFourierConfigGUI::on_update) );

  //fileEntry.signal_activate().
  //  connect(sigc::mem_fun(*this,
  //			  &GmicWatermarkFourierConfigGUI::on_filename_changed));
}


void PF::GmicWatermarkFourierConfigGUI::on_update()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicWatermarkFourierPar* par = dynamic_cast<GmicWatermarkFourierPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}



