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

#include "curves_config.hh"

PF::CurvesConfigDialog::CurvesConfigDialog(PF::Layer* layer):
  PF::OperationConfigDialog( layer, "Curves Configuration" ),
  //rgbCurveSelector( this, "RGB_active_curve", "Channel: ", 1 ),
  //labCurveSelector( this, "Lab_active_curve", "Channel: ", 5 ),
  //cmykCurveSelector( this, "CMYK_active_curve", "Channel: ", 8 ),
  greyCurveEditor( this, "grey_curve" ),
  rgbCurveEditor( this, "RGB_curve" ),
  RCurveEditor( this, "R_curve" ),
  GCurveEditor( this, "G_curve" ),
  BCurveEditor( this, "B_curve" ),
  LCurveEditor( this, "L_curve" ),
  aCurveEditor( this, "a_curve" ),
  bCurveEditor( this, "b_curve" )
{

  //rgbCurveSelector.init(); 
  rgbCurveSelector.append("RGB");
  rgbCurveSelector.append("Red");
  rgbCurveSelector.append("Green");
  rgbCurveSelector.append("Blue");
  rgbCurveSelector.set_active( 0 );

  labCurveSelector.append("L");
  labCurveSelector.append("a");
  labCurveSelector.append("b");
  labCurveSelector.set_active( 0 );

  add_widget( selectorsBox );

  /*
  rgbCurveEditor.init();
  RCurveEditor.init();
  GCurveEditor.init();
  BCurveEditor.init();

  LCurveEditor.init();
  aCurveEditor.init();
  bCurveEditor.init();
  */

  add_widget( curvesBox );
  switch_curve();

  rgbCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
			  &CurvesConfigDialog::update));
  labCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
			  &CurvesConfigDialog::update));
  //add_control( &rgbCurveSelector );
  //add_control( &labCurveSelector );
  //add_control( &cmykCurveSelector );
}


PF::CurvesConfigDialog::~CurvesConfigDialog()
{
}


void PF::CurvesConfigDialog::switch_curve()
{
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    if( greyCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( greyCurveEditor );

    if( rgbCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( rgbCurveEditor );
    if( RCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( RCurveEditor );
    if( GCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( GCurveEditor );
    if( BCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( BCurveEditor );
    
    if( LCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( LCurveEditor );
    if( aCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( aCurveEditor );
    if( bCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( bCurveEditor );
    

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::colorspace_t cs = PF::convert_colorspace( par->get_interpretation() );
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      curvesBox.pack_start( greyCurveEditor, Gtk::PACK_SHRINK );
      greyCurveEditor.show();
      break;
    case PF_COLORSPACE_RGB:
      switch( rgbCurveSelector.get_active_row_number() ) {
      case 0:
	curvesBox.pack_start( rgbCurveEditor, Gtk::PACK_SHRINK );
	rgbCurveEditor.show();
	break;
      case 1:
	curvesBox.pack_start( RCurveEditor, Gtk::PACK_SHRINK );
	RCurveEditor.show();
	break;
      case 2:
	curvesBox.pack_start( GCurveEditor, Gtk::PACK_SHRINK );
	GCurveEditor.show();
	break;
      case 3:
	curvesBox.pack_start( BCurveEditor, Gtk::PACK_SHRINK );
	BCurveEditor.show();
	break;
      }
      break;
    case PF_COLORSPACE_LAB:
      switch( labCurveSelector.get_active_row_number() ) {
      case 0:
	curvesBox.pack_start( LCurveEditor, Gtk::PACK_SHRINK );
	LCurveEditor.show();
	break;
      case 1:
	curvesBox.pack_start( aCurveEditor, Gtk::PACK_SHRINK );
	aCurveEditor.show();
	break;
      case 2:
	curvesBox.pack_start( bCurveEditor, Gtk::PACK_SHRINK );
	bCurveEditor.show();
	break;
      }
      break;
      //chselBox.pack_start( labchSelector, Gtk::PACK_SHRINK );
      //labchSelector.show();
      break;
    case PF_COLORSPACE_CMYK:
      //chselBox.pack_start( cmykchSelector, Gtk::PACK_SHRINK );
      //cmykchSelector.show();
      break;
    default:
      break;
    }
  }

  //update();
}


void PF::CurvesConfigDialog::update()
{
  switch_curve();
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

#ifndef NDEBUG
    std::cout<<"CurvesConfigDialog::update() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif
    if( rgbCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( rgbCurveSelector );
    if( labCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( labCurveSelector );
    if( cmykCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( cmykCurveSelector );
    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::colorspace_t cs = PF::convert_colorspace( par->get_interpretation() );
    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      //greychSelector.show();
      break;
    case PF_COLORSPACE_RGB:
      selectorsBox.pack_start( rgbCurveSelector, Gtk::PACK_SHRINK );
      rgbCurveSelector.show();
      break;
    case PF_COLORSPACE_LAB:
      selectorsBox.pack_start( labCurveSelector, Gtk::PACK_SHRINK );
      labCurveSelector.show();
      break;
    case PF_COLORSPACE_CMYK:
      selectorsBox.pack_start( cmykCurveSelector, Gtk::PACK_SHRINK );
      cmykCurveSelector.show();
      break;
    default:
      break;
    }
  }

  PF::OperationConfigDialog::update();
}
