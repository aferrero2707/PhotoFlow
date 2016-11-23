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

#define CURVE_SIZE 260

PF::CurvesConfigGUI::CurvesConfigGUI(PF::Layer* layer):
  PF::OperationConfigGUI( layer, "Curves Configuration", false ),
  //rgbCurveSelector( this, "RGB_active_curve", "Channel: ", 1 ),
  //labCurveSelector( this, "Lab_active_curve", "Channel: ", 5 ),
  //cmykCurveSelector( this, "CMYK_active_curve", "Channel: ", 8 ),
  RGB_is_linear_check( this, "RGB_is_linear", _("linear"), false ),
  greyCurveEditor( this, "grey_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  rgbCurveEditor( this, "RGB_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  RCurveEditor( this, "R_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  GCurveEditor( this, "G_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  BCurveEditor( this, "B_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  LCurveEditor( this, "L_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  aCurveEditor( this, "a_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  bCurveEditor( this, "b_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  CCurveEditor( this, "C_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  MCurveEditor( this, "M_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  YCurveEditor( this, "Y_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  KCurveEditor( this, "K_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
  outputModeSlider( this, "color_blend", "Output mode", 0, 0, 1, 0.05, 0.2, 1)
{

  //rgbCurveSelector.init(); 
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

  cmykCurveSelector.append_text("C");
  cmykCurveSelector.append_text("M");
  cmykCurveSelector.append_text("Y");
  cmykCurveSelector.append_text("K");
  cmykCurveSelector.set_active(0);
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

  cmykCurveSelector.append("C");
  cmykCurveSelector.append("M");
  cmykCurveSelector.append("Y");
  cmykCurveSelector.append("K");
  cmykCurveSelector.set_active(0);
#endif

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

  padding1.set_size_request( 2, 10 );
  padding2.set_size_request( 2, 10 );

  //add_widget( padding1 );
  //add_widget( hline );
  //add_widget( padding2 );

  add_widget( outputModeSlider );

  rgbCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
                          &CurvesConfigGUI::do_update));
  labCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
                          &CurvesConfigGUI::do_update));
  cmykCurveSelector.signal_changed().
    connect(sigc::mem_fun(*this,
                          &CurvesConfigGUI::do_update));
  //add_control( &rgbCurveSelector );
  //add_control( &labCurveSelector );
  //add_control( &cmykCurveSelector );
}


PF::CurvesConfigGUI::~CurvesConfigGUI()
{
}


void PF::CurvesConfigGUI::activate_curve( PF::CurveEditor& curve )
{
#ifndef NDEBUG
      std::cout<<"CurvesConfigGUI::activate_curve() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

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

    if( ((&curve) != (&greyCurveEditor)) && (greyCurveEditor.get_parent() == (&curvesBox)) )
      curvesBox.remove( greyCurveEditor );

    if( ((&curve) != (&rgbCurveEditor)) && rgbCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( rgbCurveEditor );
    if( ((&curve) != (&RCurveEditor)) && RCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( RCurveEditor );
    if( ((&curve) != (&GCurveEditor)) && GCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( GCurveEditor );
    if( ((&curve) != (&BCurveEditor)) && BCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( BCurveEditor );
    
    if( ((&curve) != (&LCurveEditor)) && LCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( LCurveEditor );
    if( ((&curve) != (&aCurveEditor)) && aCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( aCurveEditor );
    if( ((&curve) != (&bCurveEditor)) && bCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( bCurveEditor );
    
    if( ((&curve) != (&CCurveEditor)) && CCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( CCurveEditor );
    if( ((&curve) != (&MCurveEditor)) && MCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( MCurveEditor );
    if( ((&curve) != (&YCurveEditor)) && YCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( YCurveEditor );
    if( ((&curve) != (&KCurveEditor)) && KCurveEditor.get_parent() == (&curvesBox) )
      curvesBox.remove( KCurveEditor );

    if( curve.get_parent() != (&curvesBox) ) {
      curvesBox.pack_start( curve, Gtk::PACK_EXPAND_PADDING );
      curve.show();
    }
  }
}


void PF::CurvesConfigGUI::switch_curve()
{
#ifndef NDEBUG
      std::cout<<"CurvesConfigGUI::switch_curve() for "<<get_layer()->get_name()<<" called"<<std::endl;
#endif
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

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
      activate_curve( greyCurveEditor );
      break;
    case PF_COLORSPACE_RGB:
      switch( rgbCurveSelector.get_active_row_number() ) {
      case 0:
        activate_curve( rgbCurveEditor );
        break;
      case 1:
        activate_curve( RCurveEditor );
        break;
      case 2:
        activate_curve( GCurveEditor );
        break;
      case 3:
        activate_curve( BCurveEditor );
        break;
      }
      break;
    case PF_COLORSPACE_LAB:
      switch( labCurveSelector.get_active_row_number() ) {
      case 0:
        activate_curve( LCurveEditor );
        break;
      case 1:
        activate_curve( aCurveEditor );
        break;
      case 2:
        activate_curve( bCurveEditor );
        break;
      }
      break;
      //chselBox.pack_start( labchSelector, Gtk::PACK_SHRINK );
      //labchSelector.show();
      break;
    case PF_COLORSPACE_CMYK:
      switch( cmykCurveSelector.get_active_row_number() ) {
      case 0:
        activate_curve( CCurveEditor );
        break;
      case 1:
        activate_curve( MCurveEditor );
        break;
      case 2:
        activate_curve( YCurveEditor );
        break;
      case 3:
        activate_curve( KCurveEditor );
        break;
      }
      //chselBox.pack_start( cmykchSelector, Gtk::PACK_SHRINK );
      //cmykchSelector.show();
      break;
    default:
      break;
    }
  }

  //update();
}


void PF::CurvesConfigGUI::do_update()
{
  switch_curve();
  //std::vector<Widget*> wl = chselBox.get_children();
  //wl.clear();
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

//#ifndef NDEBUG
    std::cout<<"CurvesConfigGUI::do_update() for "<<get_layer()->get_name()<<" called"<<std::endl;
//#endif
    if( rgbCurveSelector.get_parent() == &selectorsBox ) {
      selectorsBox.remove( rgbCurveSelector );
      selectorsBox.remove( RGB_is_linear_check );
    }
    if( labCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( labCurveSelector );
    if( cmykCurveSelector.get_parent() == &selectorsBox )
      selectorsBox.remove( cmykCurveSelector );

    bool RGB_is_linear = false;
    PF::colorspace_t cs = PF_COLORSPACE_UNKNOWN;
    PF::Image* image = get_layer()->get_image();
    PF::Pipeline* pipeline = image->get_pipeline(0);
    PF::PipelineNode* node = NULL;
    if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
    if( node && node->processor && node->processor->get_par() ) {
      PF::OpParBase* par = node->processor->get_par();
      cs = PF::convert_colorspace( par->get_interpretation() );
      //std::cout<<"OperationConfigGUI::update() par: "<<par<<std::endl;
      PF::Property<bool>* prop_lin =
          dynamic_cast< PF::Property<bool>* >( par->get_property("RGB_is_linear") );
      RGB_is_linear = prop_lin->get();
    }

    if( node && node->image ) {
      PF::ICCProfile* data = PF::get_icc_profile( node->image );
      rgbCurveEditor.set_icc_data( data );
      RCurveEditor.set_icc_data( data );
      GCurveEditor.set_icc_data( data );
      BCurveEditor.set_icc_data( data );
      rgbCurveEditor.set_display_mode( RGB_is_linear );
      RCurveEditor.set_display_mode( RGB_is_linear );
      GCurveEditor.set_display_mode( RGB_is_linear );
      BCurveEditor.set_display_mode( RGB_is_linear );
    }

    switch( cs ) {
    case PF_COLORSPACE_GRAYSCALE:
      //greychSelector.show();
      break;
    case PF_COLORSPACE_RGB:
      selectorsBox.pack_start( rgbCurveSelector, Gtk::PACK_SHRINK );
      selectorsBox.pack_start( RGB_is_linear_check, Gtk::PACK_SHRINK );
      rgbCurveSelector.show();
      RGB_is_linear_check.show();
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


bool PF::CurvesConfigGUI::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  return false;
}


bool PF::CurvesConfigGUI::pointer_release_event( int button, double x, double y, int mod_key )
{
  std::cout<<"CurvesConfigGUI::pointer_release_event(): button="<<button<<"  x="<<x<<"  y="<<y<<"    mod_key="<<mod_key<<std::endl;
  if( button != 1 || mod_key != (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) ) return false;

  // Retrieve the layer associated to the filter 
  PF::Layer* layer = get_layer();
  if( !layer ) return false;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return false;

  // Retrieve the pipeline #0 (full resolution preview)
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) return false;

  // Find the pipeline node associated to the current layer
  PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
  if( !node ) return false;

  // Find the input layer of the current filter
  if( node->input_id < 0 ) return false;
  PF::Layer* lin = image->get_layer_manager().get_layer( node->input_id );
  if( !lin ) return false;

  // Sample a 5x5 pixels region of the input layer
  std::vector<float> values;
  double lx = x, ly = y, lw = 1, lh = 1;
  screen2layer( lx, ly, lw, lh );
  std::cout<<"image->sample( lin->get_id(), "<<lx<<", "<<ly<<", 5, NULL, values );"<<std::endl;
  image->sample( lin->get_id(), lx, ly, 5, NULL, values );

  std::cout<<"CurvesConfigGUI::pointer_release_event(): values="<<values[0]<<","<<values[1]<<","<<values[2]<<std::endl;

  PF::OpParBase* par = get_layer()->get_processor()->get_par();
  PF::colorspace_t cs = PF_COLORSPACE_UNKNOWN;
  if( node->processor && node->processor->get_par() ) {
    PF::OpParBase* par = node->processor->get_par();
    cs = PF::convert_colorspace( par->get_interpretation() );
    //std::cout<<"OperationConfigGUI::update() par: "<<par<<std::endl;
    std::cout<<"CurvesConfigGUI::pointer_release_event(): interpretation="<<par->get_interpretation()<<std::endl;
    std::cout<<"CurvesConfigGUI::pointer_release_event(): colorspace="<<cs<<std::endl;
  }
  switch( cs ) {
  case PF_COLORSPACE_GRAYSCALE:
    if( values.empty() ) return false;
    greyCurveEditor.add_point( values[0] );
    break;
  case PF_COLORSPACE_RGB:
    if( values.size() != 3 ) return false;
    std::cout<<"CurvesConfigGUI::pointer_release_event(): rgbCurveSelector.get_active_row_number()="<<rgbCurveSelector.get_active_row_number()<<std::endl;
    switch( rgbCurveSelector.get_active_row_number() ) {
    case 0:
      std::cout<<"CurvesConfigGUI::pointer_release_event(): rgbCurveEditor.add_point( "<<(values[0]+values[1]+values[2])/3.0f<<" );"<<std::endl;
      rgbCurveEditor.add_point( (values[0]+values[1]+values[2])/3.0f );
      break;
    case 1:
      RCurveEditor.add_point( values[0] );
      break;
    case 2:
      GCurveEditor.add_point( values[1] );
      break;
    case 3:
      BCurveEditor.add_point( values[2] );
      break;
    }
    break;
  case PF_COLORSPACE_LAB:
    if( values.size() != 3 ) return false;
    switch( labCurveSelector.get_active_row_number() ) {
    case 0:
      LCurveEditor.add_point( values[0] );
      break;
    case 1:
      aCurveEditor.add_point( values[1] );
      break;
    case 2:
      bCurveEditor.add_point( values[2] );
      break;
    }
    break;
    break;
  case PF_COLORSPACE_CMYK:
    break;
  default:
    break;
  }

  return false;
}


bool PF::CurvesConfigGUI::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  return false;
}
