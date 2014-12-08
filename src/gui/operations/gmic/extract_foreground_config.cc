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


PF::GmicExtractForegroundConfigDialog::GmicExtractForegroundConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Dream Smoothing (G'MIC)"  ),
  updateButton( "Update" )
{
  //controlsBox.pack_start( iterations_slider );
  controlsBox.pack_start( updateButton );

  updateButton.signal_clicked().connect( sigc::mem_fun(this, &GmicExtractForegroundConfigDialog::on_update) );
  
  add_widget( controlsBox );
}



PF::GmicExtractForegroundPar* PF::GmicExtractForegroundConfigDialog::get_par()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::GmicExtractForegroundPar* par = dynamic_cast<GmicExtractForegroundPar*>( get_layer()->get_processor()->get_par() );
    return par;
  }
  return NULL;
}


void PF::GmicExtractForegroundConfigDialog::on_update()
{
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return;

  par->refresh();
  get_layer()->get_image()->lock();
  std::cout<<"  updating image"<<std::endl;
  get_layer()->get_image()->update();
  get_layer()->get_image()->unlock();
}


void PF::GmicExtractForegroundConfigDialog::open()
{
  OperationConfigDialog::open();
}



bool PF::GmicExtractForegroundConfigDialog::pointer_release_event( int button, double x, double y, int mod_key )
{
  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return false;

  std::cout<<"GmicExtractForegroundConfigDialog::pointer_release_event(): button="<<button<<std::endl;

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
    par->get_bg_points().get().push_back( std::make_pair((int)x, (int)y) );
    return true;
  }

  return false;
}


bool PF::GmicExtractForegroundConfigDialog::modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                                                            float scale, int xoffset, int yoffset )
{
  if( !get_mapped() ) 
    return false;

  // We only draw on top of the preview image if we are mapped

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to outout
  buf_out.copy( buf_in );

  int point_size = 2;

  PF::GmicExtractForegroundPar* par = get_par();
  if( !par ) return false;

  std::list< std::pair<int,int> >::iterator i;
  for(i = par->get_fg_points().get().begin(); i != par->get_fg_points().get().end(); i++ ) {
    
    VipsRect point = { i->first*scale-point_size-1+xoffset, 
                       i->second*scale-point_size-1+yoffset,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { i->first*scale-point_size+xoffset, 
                        i->second*scale-point_size+yoffset,
                        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    buf_out.fill( point2, 0, 255, 0 );
  }
  for(i = par->get_bg_points().get().begin(); i != par->get_bg_points().get().end(); i++ ) {
    
    VipsRect point = { i->first*scale-point_size-1+xoffset, 
                       i->second*scale-point_size-1+yoffset,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { i->first*scale-point_size+xoffset, 
                        i->second*scale-point_size+yoffset,
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
