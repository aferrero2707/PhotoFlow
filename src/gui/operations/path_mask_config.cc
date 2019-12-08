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

#include "path_mask_config.hh"


#define CURVE_SIZE 300

static std::ostream& operator <<( std::ostream& str, const VipsRect& r )
{
  str<<r.width<<","<<r.height<<"+"<<r.left<<"+"<<r.top;
  return str;
}





PF::PathMaskConfigGUI::PathMaskConfigGUI( PF::Layer* layer ):
              OperationConfigGUI( layer, "Path mask tool" ),
              invert_box( this, "invert", _("invert"), true ),
              enable_falloff_box( this, "enable_falloff", _("enable falloff"), true ),
              falloffCurveEditor( this, "falloff_curve", new PF::CurveArea(), 0, 100, 0, 100, CURVE_SIZE, CURVE_SIZE ),
              active_point_id( -1 ), center_selected( false ),
              initializing( false )
{
  hbox.pack_start( invert_box, Gtk::PACK_SHRINK );
  hbox.pack_start( enable_falloff_box, Gtk::PACK_SHRINK );
  hbox.set_spacing(10);
  add_widget( hbox );

  add_widget( curvesBox );

  curvesBox.pack_start( falloffCurveEditor, Gtk::PACK_SHRINK );
  falloffCurveEditor.show();
}


void PF::PathMaskConfigGUI::parameters_reset()
{
  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( par ) par->path_reset();
  OperationConfigGUI::parameters_reset();
}



void PF::PathMaskConfigGUI::do_update()
{
  PF::OperationConfigGUI::do_update();
}



bool PF::PathMaskConfigGUI::pointer_press_event( int button, double sx, double sy, int mod_key )
{
  std::cout<<"PathMaskConfigGUI::pointer_press_event(): button="<<button<<std::endl;

  if( !get_editing_flag() ) return false;

  border_resizing = false;
  path_resizing = false;
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

  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( !par ) return false;
  const std::vector< std::pair<float,float> >* ppoints = NULL;
  ppoints = &(par->get_smod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;

  double x = sx, y = sy, w = 10, h = 10;
  screen2layer( x, y, w, h );
  int D = w;

  if( initializing && button == 3 ) {
    initializing = false;
    return true;
  }

  if( initializing && button == 1 ) {
    active_point_id =
        par->get_smod().add_point( 0,//par->get_smod().get_npoints(),
            x/node->image->Xsize, y/node->image->Ysize );
    return true;
  }

  if( points.empty() && button == 1 ) {
    initializing = true;
    par->get_smod().add_point( 0, x/node->image->Xsize, y/node->image->Ysize );
    par->get_smod().add_point( 1, x/node->image->Xsize, y/node->image->Ysize );
    par->path_modified();
    active_point_id = 0;
  }

  center_selected = false;
  std::pair<float,float> center = par->get_smod().get_center();
  double cx = center.first*node->image->Xsize;
  double cy = center.second*node->image->Ysize;
  double dx = x - cx;
  double dy = y - cy;
  if( (fabs(dx) < D) && (fabs(dy) < D) ) {
    center_selected = true;
    cxlast = cx; cylast = cy;
    return true;
  }


  // Find handle point
  active_point_id = -1;
  for(unsigned int i = 0; i < points.size(); i++ ) {
    double px = 0, py = 0;
    px = points[i].first*node->image->Xsize;
    py = points[i].second*node->image->Ysize;
    dx = x - px;
    dy = y - py;
    if( (fabs(dx) > D) || (fabs(dy) > D) ) continue;
    active_point_id = i;
    break;
  }
  if( active_point_id >= 0 && button ==  1 && mod_key == (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) ) {
    path_resizing = true;
    path_resizing_last_point_x = x/node->image->Xsize;
    path_resizing_last_point_y = y/node->image->Ysize;
    return true;
  }
  if( active_point_id >= 0 && button == 3 ) {
    par->get_smod().remove_point( active_point_id );
    par->path_modified();
    active_point_id = -1;
    return true;
  }

  // Check if click is along the falloff outline, in which case we initiate
  // the falloff resizing
  if( active_point_id < 0 && button == 1 ) {

    const std::vector< std::pair<int,int> >& path_points = par->get_smod().get_outline();
    const std::vector< std::pair<int,int> >& border_points = par->get_smod().get_border();
    for( unsigned int pi = 0; pi < border_points.size(); pi++ ) {
      double px = border_points[pi].first, py = border_points[pi].second, pw = 1, ph = 1;
      double dx = x - px;
      double dy = y - py;
      if( (fabs(dx) > D) || (fabs(dy) > D) ) continue;

      double px2 = path_points[pi].first, py2 = path_points[pi].second;

      border_resizing = true;
      border_resizing_path_point_x = px2;
      border_resizing_path_point_y = py2;
      border_resizing_lstart = sqrt( (px-px2)*(px-px2) + (py-py2)*(py-py2) );
      border_resizing_size_start = par->get_border_size();
      //std::cout<<"Border selected, lstart="<<border_resizing_lstart<<std::endl;
      return true;
    }
  }

  // Check if click is along the path outline, in which case we add
  // a new spline point
  if( active_point_id < 0 && button == 1 ) {

    const std::vector< std::pair<int,int> >& spline_points = par->get_smod().get_outline();
    std::pair<float,float> outline_scaling =
        par->get_smod().get_outline_scaling( node->image->Xsize, node->image->Ysize );
    int spline_point_id = -1;
    for( unsigned int pi = 0; pi < spline_points.size(); pi++ ) {
      double px = spline_points[pi].first*outline_scaling.first,
          py = spline_points[pi].second*outline_scaling.second, pw = 1, ph = 1;

      // check if we are crossing a control point
      for(unsigned int i = 0; i < points.size(); i++ ) {
        double px2 = 0, py2 = 0;
        px2 = points[i].first*node->image->Xsize;
        py2 = points[i].second*node->image->Ysize;
        double dx2 = px - px2;
        double dy2 = py - py2;
        if( (fabs(dx2) > D) || (fabs(dy2) > D) ) continue;
        spline_point_id = i;
        break;
      }

      double dx = x - px;
      double dy = y - py;
      if( (fabs(dx) > D) || (fabs(dy) > D) ) continue;

      // the point is close enough to the spline, so it can be added after the last crossed control point
      active_point_id = par->get_smod().add_point( spline_point_id+1, x/node->image->Xsize, y/node->image->Ysize );
      par->path_modified();
      break;
    }
  }

  return true;
}


bool PF::PathMaskConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  //std::cout<<"PathMaskConfigGUI::pointer_release_event(): button="<<button<<std::endl;

  if( !get_editing_flag() ) return false;

  border_resizing = false;
  path_resizing = false;
  if( button != 1 && button != 3 ) return false;
  if( initializing ) return false;

  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( !par ) return false;

  // Retrieve the layer associated to the filter
  PF::Layer* layer = get_layer();
  if( !layer ) return false;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return false;

  par->set_modified();
  image->update();

  return false;
}


bool PF::PathMaskConfigGUI::pointer_motion_event( int button, double sx, double sy, int mod_key )
{
  //std::cout<<"PathMaskConfigGUI::pointer_motion_event() called"<<std::endl;
  if( !get_editing_flag() ) return false;

  if( !initializing && button != 1 ) return false;

  if( !path_resizing && !border_resizing && !center_selected && active_point_id < 0 ) return false;

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

  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( !par ) return false;

  const std::vector< std::pair<float,float> >* ppoints = NULL;
  ppoints = &(par->get_smod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;
  const std::pair<float,float> center = par->get_smod().get_center();

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );
  float fx = x/node->image->Xsize, fy = y/node->image->Ysize;

  if( initializing ) {
    active_point_id = par->get_smod().set_point( active_point_id, fx, fy );
    par->path_modified();
    //std::cout<<"PathMaskConfigGUI::pointer_motion_event() finished (initializing)"<<std::endl;
    return true;
  }

  if( path_resizing ) {
    double last_dx = path_resizing_last_point_x - center.first;
    double last_dy = path_resizing_last_point_y - center.second;
    double dx = fx - center.first;
    double dy = fy - center.second;
    double last_l = sqrt( last_dx*last_dx + last_dy*last_dy );
    double l = sqrt( dx*dx + dy*dy );
    double R = l/last_l;
    path_resizing_last_point_x = fx;
    path_resizing_last_point_y = fy;
    for( unsigned int pi = 0; pi < points.size(); pi++ ) {
      double pdx = points[pi].first - center.first;
      double pdy = points[pi].second - center.second;
      pdx *= R; pdy *= R;
      par->get_smod().set_point( pi, center.first+pdx, center.second+pdy );
    }
    par->path_modified();
    //std::cout<<"PathMaskConfigGUI::pointer_motion_event() finished (path_resizing)"<<std::endl;
    return true;
  }

  if( border_resizing ) {
    float l = sqrt( (x-border_resizing_path_point_x)*(x-border_resizing_path_point_x) +
        (y-border_resizing_path_point_y)*(y-border_resizing_path_point_y) );
    float r = l/border_resizing_lstart;
    float new_size = r * border_resizing_size_start;
    //std::cout<<"Border resize: l="<<l<<"  r="<<r<<"  new_size="<<new_size<<std::endl;
    par->set_border_size( MAX(0.01, new_size) );
    par->path_modified();
    //std::cout<<"PathMaskConfigGUI::pointer_motion_event() finished (border_resizing)"<<std::endl;
    return true;
  }

  if( center_selected ) {
    double dx = fx - par->get_smod().get_center().first;
    double dy = fy - par->get_smod().get_center().second;
    for( unsigned int pi = 0; pi < points.size(); pi++ ) {
      double newx = points[pi].first + dx;
      double newy = points[pi].second + dy;
      par->get_smod().set_point( pi, newx, newy );
    }
    par->get_smod().update_center();
    par->path_modified();
  } else {
    if( (int)(points.size()) <= active_point_id ) {
      //std::cout<<"PathMaskConfigGUI::pointer_motion_event() finished (!center_selected)"<<std::endl;
      return false;
    }

    active_point_id = par->get_smod().set_point( active_point_id, fx, fy );
    par->path_modified();
  }

  //std::cout<<"PathMaskConfigGUI::pointer_motion_event() finished"<<std::endl;
  return true;
}




void PF::PathMaskConfigGUI::draw_outline( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out, ClosedSplineCurve& path )
{
  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( !par ) return;

  // Retrieve the layer associated to the filter
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return;

  // Retrieve the pipeline #0 (full resolution preview)
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) return;

  // Find the pipeline node associated to the current layer
  PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
  if( !node ) return;
  if( !node->image ) return;

  path.set_border_size( par->get_border_size() );
  path.update_outline( node->image->Xsize, node->image->Ysize );

  const std::vector< std::pair<int,int> >& spline_points = path.get_outline();
  //std::cout<<"spline_points.size(): "<<spline_points.size()<<std::endl;
  int xlast=-1, ylast=-1;
  for( unsigned int pi = 0; pi < spline_points.size(); pi++ ) {
    //std::cout<<"point #"<<pi<<": "<<spline_points[pi].first<<" "<<spline_points[pi].second<<std::endl;
    double bpx = spline_points[pi].first, bpy = spline_points[pi].second, pw = 1, ph = 1;

    layer2screen( bpx, bpy, pw, ph );

    int ipx = bpx, ipy = bpy;
    if( ipx==xlast && ipy==ylast ) continue;
    xlast = ipx; ylast = ipy;

    int pi2 = (int)(pi) - 1;
    if( pi2 < 0 ) pi2 = spline_points.size() - 1;

    double bpx2 = spline_points[pi2].first, bpy2 = spline_points[pi2].second, pw2 = 1, ph2 = 1;

    layer2screen( bpx2, bpy2, pw2, ph2 );

    if( bpx==bpx2 && bpy==bpy2 ) continue;

    buf_out.draw_point( bpx, bpy, buf_in );
    //buf_out.draw_line( bpx, bpy, bpx2, bpy2, buf_in );
  }

  if( initializing ) return;
  if( par->get_falloff_enabled() == false ) return;

  const std::vector< std::pair<int,int> >& border_points = path.get_border();
  //std::cout<<"border_points.size(): "<<border_points.size()<<std::endl;
  xlast=-1; ylast=-1;
  for( unsigned int pi = 0; pi < border_points.size(); pi++ ) {
    //std::cout<<"point #"<<pi<<": "<<border_points[pi].first<<" "<<border_points[pi].second<<std::endl;
    double bpx = border_points[pi].first, bpy = border_points[pi].second, pw = 1, ph = 1;

    layer2screen( bpx, bpy, pw, ph );

    int ipx = bpx, ipy = bpy;
    if( ipx==xlast && ipy==ylast ) continue;
    xlast = ipx; ylast = ipy;

    int pi2 = (int)(pi) - 1;
    if( pi2 < 0 ) pi2 = border_points.size() - 1;

    double bpx2 = border_points[pi2].first, bpy2 = border_points[pi2].second, pw2 = 1, ph2 = 1;

    layer2screen( bpx2, bpy2, pw2, ph2 );

    if( bpx==bpx2 && bpy==bpy2 ) continue;

    buf_out.draw_point( bpx, bpy, buf_in );
    //buf_out.draw_line( bpx, bpy, bpx2, bpy2, buf_in );
  }
}



bool PF::PathMaskConfigGUI::modify_preview( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out,
    float scale, int xoffset, int yoffset )
{
  PF::PathMaskPar* par = dynamic_cast<PF::PathMaskPar*>(get_par());
  if( !par ) return false;

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
  ppoints = &(par->get_smod().get_points());
  const std::vector< std::pair<float,float> >& points = *ppoints;

  int point_size = 2;


  draw_outline( buf_in, buf_out, par->get_smod() );

  unsigned int ps = points.size();
  //std::cout<<"PathMaskConfigGUI::modify_preview(): ps="<<ps<<std::endl;
  for(unsigned int i = 0; i < ps; i++ ) {
    double px = 0, py = 0, pw = 1, ph = 1;
    //std::cout<<"PathMaskConfigGUI::modify_preview(): point #"<<i<<"="<<
    //    points[i].first<<","<<points[i].second<<std::endl;
    px = points[i].first*node->image->Xsize;
    py = points[i].second*node->image->Ysize;
    layer2screen( px, py, pw, ph );
    //std::cout<<"PathMaskConfigGUI::modify_preview(): point #"<<i<<"="<<px<<","<<py<<std::endl;
    VipsRect point = { (int)px-point_size-1,
        (int)py-point_size-1,
        point_size*2+3, point_size*2+3};
    VipsRect point2 = { (int)px-point_size,
        (int)py-point_size,
        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    if( (int)(i) == active_point_id )
      buf_out.fill( point2, 255, 0, 0 );
    else
      buf_out.fill( point2, 255, 255, 255 );

    continue;

    int i2 = (int)(i) - 1;
    if( i2 < 0 ) i2 = (int)(ps) - 1;

    double px0 = 0, py0 = 0, pw0 = 1, ph0 = 1;
    px0 = points[i2].first*node->image->Xsize;
    py0 = points[i2].second*node->image->Ysize;
    layer2screen( px0, py0, pw0, ph0 );

    //printf("buf_out.draw_line( %f, %f, %f, %f)\n", px0, py0, px, py );
    buf_out.draw_line( px0, py0, px, py, buf_in );
  }
  //std::cout<<std::endl;

  std::pair<float,float> center = par->get_smod().get_center();
  double cx = center.first * node->image->Xsize;
  double cy = center.second * node->image->Ysize;
  double cw = 1, ch = 1;
  layer2screen( cx, cy, cw, ch );
  buf_out.draw_line( cx, cy-10, cx, cy+10, buf_in );
  buf_out.draw_line( cx-10, cy, cx+10, cy, buf_in );


  return true;
}
