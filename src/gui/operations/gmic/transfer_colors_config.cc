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

#include "../../operations/gmic/transfer_colors.hh"
#include "transfer_colors_config.hh"


PF::GmicTransferColorsConfigDialog::GmicTransferColorsConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Transfer colors [G'MIC]" ),
  updateButton( "Update" ),
  layer_list( this, "Reference image:"),
  regularizationSlider( this, "regularization", "Regularization: ", 8, 0, 16, 1, 5, 1 ),
  lumiSlider( this, "preserve_lumi", "Preserve lumi: ", 0.2, 0, 1, 0.05, 0.2, 1 ),
  precisionSelector( this, "precision", "Precision: ", 1 )
{
  add_widget( updateButton );
  add_widget( layer_list );
  add_widget( regularizationSlider );
  add_widget( lumiSlider );
  add_widget( precisionSelector );

  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicTransferColorsConfigDialog::on_update) );

  //fileEntry.signal_activate().
  //  connect(sigc::mem_fun(*this,
  //			  &GmicTransferColorsConfigDialog::on_filename_changed));
}


void PF::GmicTransferColorsConfigDialog::on_layer_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
  }
}


void PF::GmicTransferColorsConfigDialog::on_update()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    GmicTransferColorsPar* par = dynamic_cast<GmicTransferColorsPar*>( get_layer()->get_processor()->get_par() );
    if( !par ) return;
    par->refresh();
    get_layer()->get_image()->lock();
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
    get_layer()->get_image()->unlock();
  }
}


void PF::GmicTransferColorsConfigDialog::do_update()
{
  layer_list.update_model();
  OperationConfigDialog::do_update();
}


void PF::GmicTransferColorsConfigDialog::init()
{
  layer_list.update_model();
  OperationConfigDialog::init();
}
