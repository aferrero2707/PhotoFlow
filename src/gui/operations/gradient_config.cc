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


#define CURVE_SIZE 192

static std::ostream& operator <<( std::ostream& str, const VipsRect& r )
{
  str<<r.width<<","<<r.height<<"+"<<r.left<<"+"<<r.top;
  return str;
}





PF::GradientConfigGUI::GradientConfigGUI( PF::Layer* layer ):
      OperationConfigGUI( layer, _("Gradient tool") ),
      typeSelector( this, "gradient_type", _("Type: "), 1 ),
      invert_box( this, "invert", _("Invert"), true ),
      center_x( this, "gradient_center_x", _("Center X (%)"), 100, 0, 100, 1, 10, 100),
      center_y( this, "gradient_center_y", _("Center Y (%)"), 100, 0, 100, 1, 10, 100),
      greyCurveEditor( this, "grey_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      rgbCurveEditor( this, "RGB_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      RCurveEditor( this, "R_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      GCurveEditor( this, "G_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      BCurveEditor( this, "B_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      LCurveEditor( this, "L_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      aCurveEditor( this, "a_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      bCurveEditor( this, "b_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
      active_point_id( -1 )
{
  hbox.pack_start( typeSelector, Gtk::PACK_SHRINK );
  hbox.pack_start( invert_box, Gtk::PACK_SHRINK, 5 );
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
#ifndef NDEBUG
      std::cout<<"GradientConfigGUI::switch_curve() cs="<<cs<<std::endl;
#endif
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



bool PF::GradientConfigGUI::pointer_press_event( int button, double sx, double sy, int mod_key )
{
  std::cout<<"GradientConfigGUI::pointer_press_event(): button="<<button<<std::endl;

  if( !get_editing_flag() ) return false;

  if( button != 1 && button != 3 ) return false;

  // Retrieve the layer associated to the filter
  PF::Layer* layer = get_layer();
  if( !layer ) return false;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return false;

  // Retrieve the pipeline #0 (full resolution preview)
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) return false;
  int level = pipeline->get_level();
  float scale = 1;
  for( int i = 1; i <= level; i++ ) scale *= 2;

  // Find the pipeline node associated to the current layer
  PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
  if( !node ) return false;
  if( !node->image ) return false;

  PF::GradientPar* par = dynamic_cast<PF::GradientPar*>(get_par());
  if( !par ) return false;
  if( par->get_gradient_type() != GRADIENT_VERTICAL &&
      par->get_gradient_type() != GRADIENT_HORIZONTAL ) return false;

  const std::vector< std::pair<float,float> >* ppoints = NULL;
  if( par->get_gradient_type() == GRADIENT_VERTICAL ) ppoints = &(par->get_vmod().get_points());
  if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) ppoints = &(par->get_hmod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;

  double x = sx, y = sy, w = 10, h = 10;
  screen2layer( x, y, w, h );
  int D = w;

  // Find handle point
  active_point_id = -1;
  for(unsigned int i = 0; i < points.size(); i++ ) {
    double px = 0, py = 0;
    if( par->get_gradient_type() == GRADIENT_VERTICAL ) {
      px = points[i].first*node->image->Xsize;
      py = points[i].second*node->image->Ysize;
    }
    if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) {
      px = points[i].second*node->image->Xsize;
      py = points[i].first*node->image->Ysize;
    }
    double dx = x - px;
    double dy = y - py;
    if( (fabs(dx) > D) || (fabs(dy) > D) ) continue;
    active_point_id = i;
    break;
  }

  if( active_point_id < 0 && button == 1 ) {

    if( par->get_gradient_type() == GRADIENT_VERTICAL )
      active_point_id = par->get_vmod().add_point( x/node->image->Xsize, y/node->image->Ysize );

    if( par->get_gradient_type() == GRADIENT_HORIZONTAL )
      active_point_id = par->get_hmod().add_point( y/node->image->Ysize, x/node->image->Xsize );
  }
  if( active_point_id >= 0 && button == 3 ) {
    if( par->get_gradient_type() == GRADIENT_VERTICAL )
      par->get_vmod().remove_point( active_point_id );
    if( par->get_gradient_type() == GRADIENT_HORIZONTAL )
      par->get_hmod().remove_point( active_point_id );
    active_point_id = -1;
  }

  return true;
}


bool PF::GradientConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  std::cout<<"GradientConfigGUI::pointer_release_event(): button="<<button<<std::endl;

  if( !get_editing_flag() ) return false;

  if( button != 1 && button != 3 ) return false;

  PF::GradientPar* par = dynamic_cast<PF::GradientPar*>(get_par());
  if( !par ) return false;
  if( par->get_gradient_type() != GRADIENT_VERTICAL &&
      par->get_gradient_type() != GRADIENT_HORIZONTAL ) return false;

  // Retrieve the layer associated to the filter
  PF::Layer* layer = get_layer();
  if( !layer ) return false;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return false;

  image->update();

  return false;
}


bool PF::GradientConfigGUI::pointer_motion_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  if( button != 1 ) return false;
  if( active_point_id < 0 ) return false;

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
  if( !node->image ) return false;

  PF::GradientPar* par = dynamic_cast<PF::GradientPar*>(get_par());
  if( !par ) return false;
  if( par->get_gradient_type() != GRADIENT_VERTICAL &&
      par->get_gradient_type() != GRADIENT_HORIZONTAL ) return false;

  const std::vector< std::pair<float,float> >* ppoints = NULL;
  if( par->get_gradient_type() == GRADIENT_VERTICAL ) ppoints = &(par->get_vmod().get_points());
  if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) ppoints = &(par->get_hmod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;

  if( (int)(points.size()) <= active_point_id ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );
  float fx = x/node->image->Xsize, fy = y/node->image->Ysize;

  if( par->get_gradient_type() == GRADIENT_VERTICAL )
    par->get_vmod().set_point( active_point_id, fx, fy );
  if( par->get_gradient_type() == GRADIENT_HORIZONTAL )
    par->get_hmod().set_point( active_point_id, fy, fx );

  return true;
}



bool PF::GradientConfigGUI::modify_preview( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out,
    float scale, int xoffset, int yoffset )
{
  PF::GradientPar* par = dynamic_cast<PF::GradientPar*>(get_par());
  if( !par ) return false;
  if( par->get_gradient_type() != GRADIENT_VERTICAL &&
      par->get_gradient_type() != GRADIENT_HORIZONTAL ) return false;

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
  if( !node->image ) return false;

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to outout
  buf_out.copy( buf_in );

  const std::vector< std::pair<float,float> >* ppoints = NULL;
  if( par->get_gradient_type() == GRADIENT_VERTICAL ) ppoints = &(par->get_vmod().get_points());
  if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) ppoints = &(par->get_hmod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;

  int point_size = 2;

  int ps = points.size();
  //std::cout<<"GradientConfigGUI::modify_preview(): ps="<<ps<<std::endl;
  for( int i = 0; i < ps; i++ ) {
    double px = 0, py = 0, pw = 1, ph = 1;
    if( par->get_gradient_type() == GRADIENT_VERTICAL ) {
      px = points[i].first*node->image->Xsize;
      py = points[i].second*node->image->Ysize;
    }
    if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) {
      px = points[i].second*node->image->Xsize;
      py = points[i].first*node->image->Ysize;
    }
    layer2screen( px, py, pw, ph );
    //std::cout<<"GradientConfigGUI::modify_preview(): point #"<<i<<"="<<px<<","<<py<<std::endl;
    VipsRect point = { (int)px-point_size-1,
        (int)py-point_size-1,
        point_size*2+3, point_size*2+3};
    VipsRect point2 = { (int)px-point_size,
        (int)py-point_size,
        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    if( i == active_point_id )
      buf_out.fill( point2, 255, 0, 0 );
    else
      buf_out.fill( point2, 255, 255, 255 );

    int i2 = i - 1;
    if( i2 < 0 ) continue;

    double px0 = 0, py0 = 0, pw0 = 1, ph0 = 1;
    if( par->get_gradient_type() == GRADIENT_VERTICAL ) {
      px0 = points[i2].first*node->image->Xsize;
      py0 = points[i2].second*node->image->Ysize;
    }
    if( par->get_gradient_type() == GRADIENT_HORIZONTAL ) {
      px0 = points[i2].second*node->image->Xsize;
      py0 = points[i2].first*node->image->Ysize;
    }
    layer2screen( px0, py0, pw0, ph0 );

    //printf("buf_out.draw_line( %f, %f, %f, %f)\n", px0, py0, px, py );
    buf_out.draw_line( px0, py0, px, py, buf_in );
  }
  //std::cout<<std::endl;


  return true;
}
