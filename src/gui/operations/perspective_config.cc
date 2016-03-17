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

#include "../../operations/scale.hh"

#include "perspective_config.hh"


PF::PerspectiveConfigGUI::PerspectiveConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Perspective Correction" ),
  active_point_id( -1 )
{
  add_widget( controlsBox );
}



void PF::PerspectiveConfigGUI::do_update()
{
  OperationConfigGUI::do_update();
}



bool PF::PerspectiveConfigGUI::pointer_press_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  if( button != 1 ) return false;

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

  PF::PerspectivePar* par = dynamic_cast<PF::PerspectivePar*>(get_par());
  if( !par ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );
  x /= node->image->Xsize;
  y /= node->image->Ysize;
  w /= node->image->Xsize;
  h /= node->image->Ysize;

  // Find handle point
  std::vector< std::pair<int,int> >::iterator i;
  active_point_id = -1;
  for(unsigned int i = 0; i < par->get_keystones().size(); i++ ) {
    double dx = x - par->get_keystones()[i].first;
    double dy = y - par->get_keystones()[i].second;
    if( (fabs(dx) > 0.01) || (fabs(dy) > 0.01) ) continue;
    active_point_id = i;
    break;
  }
  if( active_point_id >= 0 ) {
    return true;
  }
  return false;
}


bool PF::PerspectiveConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  return false;
}


bool PF::PerspectiveConfigGUI::pointer_motion_event( int button, double sx, double sy, int mod_key )
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

  PF::PerspectivePar* par = dynamic_cast<PF::PerspectivePar*>(get_par());
  if( !par ) return false;
  if( par->get_keystones().size() <= active_point_id ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );
  x /= node->image->Xsize;
  y /= node->image->Ysize;
  w /= node->image->Xsize;
  h /= node->image->Ysize;

  par->get_keystones()[active_point_id].first = x;
  par->get_keystones()[active_point_id].second = y;

  return true;
}




bool PF::PerspectiveConfigGUI::modify_preview( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out,
                                                            float scale, int xoffset, int yoffset )
{
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

  PF::PerspectivePar* par = dynamic_cast<PF::PerspectivePar*>(get_par());
  if( !par ) return false;

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to outout
  buf_out.copy( buf_in );

  if( par->get_keystones().size() == 4 ) {
    for( int i = 0; i < 4; i++ ) {
      double px1 = par->get_keystones()[i].first,
          py1 = par->get_keystones()[i].second,
          pw1 = 1, ph1 = 1;
      px1 *= node->image->Xsize;
      py1 *= node->image->Ysize;
      layer2screen( px1, py1, pw1, ph1 );

      int i2 = i+1;
      if( i2 >= 4 ) i2 = 0;
      double px2 = par->get_keystones()[i2].first,
          py2 = par->get_keystones()[i2].second,
          pw2 = 1, ph2 = 1;
      px2 *= node->image->Xsize;
      py2 *= node->image->Ysize;
      layer2screen( px2, py2, pw2, ph2 );
      buf_out.draw_line( px1, py1, px2, py2, buf_in );
    }
  }

  int point_size = 2;

  for(unsigned int i = 0; i < par->get_keystones().size(); i++ ) {
    double px = par->get_keystones()[i].first,
        py = par->get_keystones()[i].second,
        pw = 1, ph = 1;
    px *= node->image->Xsize;
    py *= node->image->Ysize;
    layer2screen( px, py, pw, ph );
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
  }

  return true;
}
