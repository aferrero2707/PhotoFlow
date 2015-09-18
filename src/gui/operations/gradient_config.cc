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

#include "gradient_config.hh"


PF::GradientConfigGUI::GradientConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Gradient tool" ),
  typeSelector( this, "gradient_type", "Gradient type: ", 1 ),
  invert_box( this, "invert", "Invert", true ),
  center_x( this, "gradient_center_x", "Center X (%)", 100, 0, 100, 1, 10, 100),
  center_y( this, "gradient_center_y", "Center Y (%)", 100, 0, 100, 1, 10, 100),
  greyCurveEditor( this, "grey_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  rgbCurveEditor( this, "RGB_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  RCurveEditor( this, "R_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  GCurveEditor( this, "G_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  BCurveEditor( this, "B_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  LCurveEditor( this, "L_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  aCurveEditor( this, "a_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 ),
  bCurveEditor( this, "b_curve", new PF::CurveArea(), 0, 100, 0, 100, 240, 240 )
{
  hbox.pack_start( typeSelector );
  hbox.pack_start( invert_box, Gtk::PACK_SHRINK );
  add_widget( hbox );
  add_widget( center_x );
  add_widget( center_y );

#ifdef GTKMM_2
  rgbCurveSelector.append_text(_("RGB"));
  rgbCurveSelector.append_text(_("Red"));
  rgbCurveSelector.append_text(_("Green"));
  rgbCurveSelector.append_text(_("Blue"));
  rgbCurveSelector.set_active( 0 );

  labCurveSelector.append_text("L");
  labCurveSelector.append_text("a");
  labCurveSelector.append_text("b");
  labCurveSelector.set_active( 0 );
#endif

#ifdef GTKMM_3
  rgbCurveSelector.append(_("RGB"));
  rgbCurveSelector.append(_("Red"));
  rgbCurveSelector.append(_("Green"));
  rgbCurveSelector.append(_("Blue"));
  rgbCurveSelector.set_active( 0 );

  labCurveSelector.append("L");
  labCurveSelector.append("a");
  labCurveSelector.append("b");
  labCurveSelector.set_active( 0 );
#endif

  add_widget( selectorsBox );

  add_widget( curvesBox );

  rgbCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
                          &GradientConfigGUI::do_update));
  labCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
                          &GradientConfigGUI::do_update));

  switch_curve();
}


void PF::GradientConfigGUI::switch_curve()
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


    PF::colorspace_t cs = PF_COLORSPACE_UNKNOWN;
    PF::Image* image = get_layer()->get_image();
    PF::Pipeline* pipeline = image->get_pipeline(0);
    PF::PipelineNode* node = NULL;
    if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
    if( node && node->processor && node->processor->get_par() ) {
      PF::OpParBase* par = node->processor->get_par();
      cs = PF::convert_colorspace( par->get_interpretation() );
      //std::cout<<"OperationConfigGUI::update() par: "<<par<<std::endl;
    }

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


void PF::GradientConfigGUI::do_update()
{
  switch_curve();
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

#ifndef NDEBUG
    std::cout<<"CurvesConfigGUI::do_update() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif
    if( rgbCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( rgbCurveSelector );
    if( labCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( labCurveSelector );
    if( cmykCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( cmykCurveSelector );

    PF::colorspace_t cs = PF_COLORSPACE_UNKNOWN;
    PF::Image* image = get_layer()->get_image();
    PF::Pipeline* pipeline = image->get_pipeline(0);
    PF::PipelineNode* node = NULL;
    if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
    if( node && node->processor && node->processor->get_par() ) {
      PF::OpParBase* par = node->processor->get_par();
      cs = PF::convert_colorspace( par->get_interpretation() );
      //std::cout<<"OperationConfigGUI::update() par: "<<par<<std::endl;
    }

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

  PF::OperationConfigGUI::do_update();
}


