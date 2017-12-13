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

#include "extract_foreground_config.hh"


PF::GmicExtractForegroundConfigGUI::GmicExtractForegroundConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Dream Smoothing (G'MIC)"  ),
  updateButton( "Update" ),
  editPointsButton( "edit control points" ),
  showMaskButton( "show mask" ),
  showBlendButton( "show blend" ),
  layer_list( this, "Foreground layer:")
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( updateButton );

  Gtk::RadioButton::Group group = editPointsButton.get_group();
  showMaskButton.set_group( group );
  //showBlendButton.set_group( group );

  controlsBox.pack_start( editPointsButton );
  controlsBox.pack_start( showMaskButton );
  //controlsBox.pack_start( showBlendButton );
  editPointsButton.set_active();

  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicExtractForegroundConfigGUI::on_update) );
  
  editPointsButton.signal_clicked().connect( sigc::mem_fun(this, &GmicExtractForegroundConfigGUI::on_edit_points) );
  showMaskButton.signal_clicked().connect( sigc::mem_fun(this, &GmicExtractForegroundConfigGUI::on_show_mask) );
  showBlendButton.signal_clicked().connect( sigc::mem_fun(this, &GmicExtractForegroundConfigGUI::on_show_blend) );
  
  add_widget( controlsBox );
  //add_widget( layer_list );
}



PF::GmicExtractForegroundPar* PF::GmicExtractForegroundConfigGUI::get_par()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::GmicExtractForegroundPar* par = dynamic_cast<GmicExtractForegroundPar*>( get_layer()->get_processor()->get_par() );
    return par;
  }
  return NULL;
}


void PF::GmicExtractForegroundConfigGUI::on_update()
{
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return;

  par->refresh();
  get_layer()->get_image()->lock();
  std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
}


void PF::GmicExtractForegroundConfigGUI::on_edit_points()
{
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return;
  par->set_preview_mode( EXTRACT_FG_PREVIEW_POINTS );
  get_layer()->get_image()->lock();
  std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
}


void PF::GmicExtractForegroundConfigGUI::on_show_mask()
{
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return;
  par->set_preview_mode( EXTRACT_FG_PREVIEW_MASK );
  get_layer()->get_image()->lock();
  std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
}


void PF::GmicExtractForegroundConfigGUI::on_show_blend()
{
  /*
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return;
  par->set_preview_mode( EXTRACT_FG_PREVIEW_BLEND );
  get_layer()->get_image()->lock();
  std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
  */
}


void PF::GmicExtractForegroundConfigGUI::open()
{
  OperationConfigGUI::open();
}



bool PF::GmicExtractForegroundConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return false;

  std::cout<<"GmicExtractForegroundConfigGUI::pointer_release_event(): button="<<button<<std::endl;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  if( button == 1 ) {
    // Add foreground control point
    par->get_fg_points().get().push_back( std::make_pair((int)x, (int)y) );
    return true;
  }

  if( button == 3 ) {
    // Add background control point
    par->get_bg_points().get().push_back( std::make_pair((int)x, (int)y) );
    return true;
  }

  if( button == 2 ) {
    // Remove control point
    std::list< std::pair<int,int> >::iterator i;
    bool found = false;
    for(i = par->get_fg_points().get().begin(); i != par->get_fg_points().get().end(); i++ ) {
      double dx = x - i->first;
      double dy = y - i->second;
      if( (fabs(dx) > 10) || (fabs(dy) > 10) ) continue;
      found = true;
      par->get_fg_points().get().erase( i );
      break;
    }
    if( found ) return true;
    for(i = par->get_bg_points().get().begin(); i != par->get_bg_points().get().end(); i++ ) {
      double dx = x - i->first;
      double dy = y - i->second;
      if( (fabs(dx) > 10) || (fabs(dy) > 10) ) continue;
      found = true;
      par->get_bg_points().get().erase( i );
      break;
    }
    if( found ) return true;
    return false;
  }

  return false;
}


bool PF::GmicExtractForegroundConfigGUI::modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                                                            float scale, int xoffset, int yoffset )
{
  /*
#if defined(_WIN32) || defined(WIN32)
  if( !is_mapped() )
    return false;
#else
  if( !get_mapped() ) 
    return false;
#endif
*/
  // We only draw on top of the preview image if we are mapped

  if( !get_editing_flag() ) return false;

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to outout
  buf_out.copy( buf_in );

  int point_size = 2;

  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return false;

  std::list< std::pair<int,int> >::iterator i;
  for(i = par->get_fg_points().get().begin(); i != par->get_fg_points().get().end(); i++ ) {
/*
    VipsRect point = { i->first*scale-point_size-1+xoffset, 
                       i->second*scale-point_size-1+yoffset,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { i->first*scale-point_size+xoffset, 
                        i->second*scale-point_size+yoffset,
                        point_size*2+1, point_size*2+1};
*/
    double px=i->first, py=i->second, pw=1, ph=1;
    layer2screen( px, py, pw, ph );
    VipsRect point = { (int)px-point_size-1,
                       (int)py-point_size-1,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { (int)px-point_size,
                        (int)py-point_size,
                        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    buf_out.fill( point2, 0, 255, 0 );
  }
  for(i = par->get_bg_points().get().begin(); i != par->get_bg_points().get().end(); i++ ) {
/*
    VipsRect point = { i->first*scale-point_size-1+xoffset, 
                       i->second*scale-point_size-1+yoffset,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { i->first*scale-point_size+xoffset, 
                        i->second*scale-point_size+yoffset,
                        point_size*2+1, point_size*2+1};
*/
    double px=i->first, py=i->second, pw=1, ph=1;
    layer2screen( px, py, pw, ph );
    VipsRect point = { (int)px-point_size-1,
                       (int)py-point_size-1,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { (int)px-point_size,
                        (int)py-point_size,
                        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    buf_out.fill( point2, 255, 0, 0 );
  }

  return true;

  // Create a cairo surface to draw on top of the output buffer
  std::cout<<"width: "<<buf_in.get_pxbuf()->get_width()<<std::endl;
  std::cout<<"stride in: "<<buf_in.get_pxbuf()->get_rowstride()<<std::endl;
  std::cout<<"stride out: "<<buf_out.get_pxbuf()->get_rowstride()<<std::endl;
  Cairo::RefPtr< Cairo::ImageSurface > surf = 
    Cairo::ImageSurface::create( buf_out.get_pxbuf()->get_pixels(),
                                 Cairo::FORMAT_RGB24,
                                 buf_out.get_pxbuf()->get_width(),
                                 buf_out.get_pxbuf()->get_height(),
                                 buf_out.get_pxbuf()->get_rowstride() );
  std::cout<<"Cairo surface created"<<std::endl;
  Cairo::RefPtr< Cairo::Context > cr = 
    Cairo::Context::create( surf );
  std::cout<<"Cairo context created"<<std::endl;

  cr->set_source_rgb( 1.0, 1.0, 1.0 );
  cr->arc( 100, 100, 35, 0, 2*M_PI );
  cr->fill();
  std::cout<<"Cairo circle filled"<<std::endl;
  surf->flush();
  /*
  cairo_surface_t* surf = 
    cairo_image_surface_create_for_data( buf_out.get_pxbuf()->get_pixels(),
                                         CAIRO_FORMAT_RGB24,
                                         buf_out.get_pxbuf()->get_width(),
                                         buf_out.get_pxbuf()->get_height(),
                                         buf_out.get_pxbuf()->get_rowstride() );

  // Create a cairo context
  cairo_t *cr = cairo_create( surf );
  cairo_arc( cr, scale*100, scale*100, scale*4, 0, 2*M_PI );
  cairo_set_source_rgb( cr, 1.0, 0.0, 0.0 );
  cairo_fill( cr );
  */
}
