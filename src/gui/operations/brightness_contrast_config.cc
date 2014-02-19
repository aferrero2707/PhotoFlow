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

#include "brightness_contrast_config.hh"


PF::BrightnessContrastConfigDialog::BrightnessContrastConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Brightness/Contrast Adjustment" ),
  //brightnessAdj( 0, -1, 1, 0.05, 0.2, 0),
  //contrastAdj( 0, -1, 1, 0.05, 0.2, 0),
  //brightnessScale(brightnessAdj),
  //contrastScale(contrastAdj)
  brightnessSlider( this, "brightness", "Brightness", 0, -1, 1, 0.05, 0.2, 1),
  contrastSlider( this, "contrast", "Contrast", 0, -1, 1, 0.05, 0.2, 1)
{
  /*
  lbrightness.set_text( "brightness" );
  brightnessScale.set_size_request(200, 30);
  brightnessScale.set_digits(2);
  brightnessScale.set_value_pos(Gtk::POS_RIGHT);
  lcontrast.set_text( "contrast" );
  contrastScale.set_size_request(200, 30);
  contrastScale.set_digits(2);
  contrastScale.set_value_pos(Gtk::POS_RIGHT);

  lbrightnessAl.set(0,0.5,0,1);
  lbrightnessAl.add( lbrightness );
  lcontrastAl.set(0,0.5,0,1);
  lcontrastAl.add( lcontrast );

  controlsBox.pack_start( lbrightnessAl );
  controlsBox.pack_start( brightnessScale );
  controlsBox.pack_start( lcontrastAl );
  controlsBox.pack_start( contrastScale );
  */
  controlsBox.pack_start( brightnessSlider );
  controlsBox.pack_start( contrastSlider );
  
  
  add_widget( controlsBox );

  /*
  brightnessAdj.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &BrightnessContrastConfigDialog::on_brightness_value_changed));

  contrastAdj.signal_value_changed().
    connect(sigc::mem_fun(*this,
			  &BrightnessContrastConfigDialog::on_contrast_value_changed));
  */
}


/*
void PF::BrightnessContrastConfigDialog::on_brightness_value_changed()
{
  double val = brightnessAdj.get_value();
  std::cout<<"New brightness value: "<<val<<std::endl;
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::BrightnessContrastPar* par = 
      dynamic_cast<PF::BrightnessContrastPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      par->set_brightness( val );
      get_layer()->set_dirty( true );
      std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
  }
}


void PF::BrightnessContrastConfigDialog::on_contrast_value_changed()
{
  double val = contrastAdj.get_value();
  std::cout<<"New contrast value: "<<val<<std::endl;
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::BrightnessContrastPar* par = 
      dynamic_cast<PF::BrightnessContrastPar*>(get_layer()->get_processor()->get_par());
    if( par ) {
      par->set_contrast( val );
      get_layer()->set_dirty( true );
      std::cout<<"  updating image"<<std::endl;
      get_layer()->get_image()->update();
    }
  }
}
*/


void PF::BrightnessContrastConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    brightnessSlider.init();
    contrastSlider.init();
  }
  OperationConfigDialog::open();
}
