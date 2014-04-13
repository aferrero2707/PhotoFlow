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


#include "imagearea.hh"


#ifndef NDEBUG
//#define DEBUG_DISPLAY
#endif

PF::ImageArea::ImageArea( View* v ):
  ViewSink( v ),
  pending_pixels( 0 )
{
  display_image = NULL;
  region = NULL;
  mask = NULL;
  mask_region = NULL;
  convert2srgb = new PF::Processor<PF::Convert2sRGBPar,PF::Convert2sRGBProc>();
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  set_size_request( 100, 100 );
}

PF::ImageArea::~ImageArea ()
{
  std::cout<<"Deleting image area"<<std::endl;
  VIPS_UNREF( mask_region );
  VIPS_UNREF( mask );
  VIPS_UNREF( region );
  VIPS_UNREF( display_image );
  //VIPS_UNREF( image );
  delete convert2srgb;
  //delete pf_image;
}



#ifdef GTKMM_2
void PF::ImageArea::expose_rect (const VipsRect& area)
#endif
#ifdef GTKMM_3
void PF::ImageArea::expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr)
#endif
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::expose_rect(): called"<<std::endl;
  std::cout<<"PF::ImageArea::expose_rect(): display_image="<<display_image<<std::endl;
#endif
  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return;

  VipsRect image = {0, 0, display_image->Xsize, display_image->Ysize};
  //VipsRect area = {expose->x, expose->y, expose->width, expose->height};
  VipsRect clip;
  
  vips_rect_intersectrect (&image, &area, &clip);
  
  
  /* Request from the mask first to get an idea of what pixels are
   * available.
   */
  if (vips_region_prepare (mask_region, &clip))
    return;
  if (vips_region_prepare (region, &clip))
    return;
  
  /* If the mask is all zero, skip the paint.
   */
  bool found_painted;
  int x, y;
  
  guchar *p = (guchar *) 
    VIPS_REGION_ADDR( mask_region, clip.left, clip.top );
  int lsk = VIPS_REGION_LSKIP( mask_region );
  found_painted = false; 
  for( y = 0; y < clip.height; y++ ) {
    for( x = 0; x < clip.width; x++ ) 
      if( p[x] ) {
	found_painted = true;
	break;
      }
    if( found_painted )
      break;
    else
      p += lsk;
  }

  if( found_painted ) { 
#ifdef GTKMM_2
    guchar *p = (guchar *) VIPS_REGION_ADDR( region, clip.left, clip.top );
    int lsk = VIPS_REGION_LSKIP( region );
      
    get_window()->draw_rgb_image( get_style()->get_white_gc(),
				  clip.left, clip.top, clip.width, clip.height,
				  Gdk::RGB_DITHER_MAX, p, lsk);
#endif
#ifdef GTKMM_3
    int x, y;
    int line_size = clip.width*3;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
      Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB,
			   false, 8, clip.width, clip.height );
    for( y = 0; y < clip.height; y++ ) {
      guchar *p = (guchar *) VIPS_REGION_ADDR( region, clip.left, clip.top+y );
      guchar *pout = &(pixbuf->get_pixels()[pixbuf->get_rowstride()*y]);
      memcpy( pout, p, line_size );
    }
    Gdk::Cairo::set_source_pixbuf( cr, pixbuf, clip.left, clip.top );
    cr->rectangle( clip.left, clip.top, clip.width, clip.height );
    cr->fill();
#endif
    pending_pixels -= clip.width*clip.height;
#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::expose_rect(): rectangle done ("<<clip.width*clip.height<<")"<<std::endl;
    std::cout<<"PF::ImageArea::expose_rect(): pending_pixels="<<pending_pixels<<std::endl;
#endif
  } else {
    pending_pixels += clip.width*clip.height;
  }

  if( pending_pixels <= 0 ) {
    // There are no more pixels waigin to be drawn, so we can finalize the processing
    set_processing( false );
    //gtk_idle_add( PF::image_rebuild_callback, (gpointer)get_view()->get_image() );
    PF::image_rebuild_callback( (gpointer)get_view()->get_image() );
#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::expose_rect(): PF::image_rebuild_callback() called"<<std::endl;
#endif
  }
}


#ifdef GTKMM_2
bool PF::ImageArea::on_expose_event (GdkEventExpose * event)
{
  GdkRectangle *expose;
  int i, n;

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::on_expose_event(): called."<<std::endl;
#endif
  gdk_region_get_rectangles (event->region, &expose, &n);
  for (i = 0; i < n; i++) {
#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::on_expose_event(): processing region ("
	     <<expose[i].x<<","<<expose[i].y<<") ("
	     <<expose[i].width<<","<<expose[i].height<<")"<<std::endl;
#endif
    //expose_rect (&expose[i]);
    VipsRect area = {expose[i].x, expose[i].y, expose[i].width, expose[i].height};
    expose_rect( area );
  }
  g_free( expose );

  return TRUE;
}
#endif

#ifdef GTKMM_3
bool PF::ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return true;

  cairo_rectangle_list_t *list =  cairo_copy_clip_rectangle_list (cr->cobj());
  for (int i = list->num_rectangles - 1; i >= 0; --i) {
    cairo_rectangle_t *rect = &list->rectangles[i];

#ifdef DEBUG_DISPLAY    
    std::cout<<"ImageArea::on_draw(): rectangle = "<<rect->x<<","<<rect->y
	     <<" -> "<<rect->width<<","<<rect->height<<std::endl;
#endif
    VipsRect area = {rect->x, rect->y, rect->width, rect->height};
    expose_rect( area, cr );
  }
#ifdef DEBUG_DISPLAY    
  std::cout<<std::endl;
#endif
  
  // Draw the image in the middle of the drawing area, or (if the image is
  // larger than the drawing area) draw the middle part of the image.
  //Gdk::Cairo::set_source_pixbuf(cr, m_image,
  //  (width - m_image->get_width())/2, (height - m_image->get_height())/2);
  cr->paint();
  
  return true;
}
#endif



void PF::ImageArea::update() 
{
  //PF::View* view = pf_image->get_view(0);

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): called"<<std::endl;
#endif
  if( !get_view() || !get_view()->get_output() ) return;

  VipsImage* image = get_view()->get_output();
  unsigned int level = get_view()->get_level();
  VipsImage* outimg = image;

  VIPS_UNREF( region ); 
  VIPS_UNREF( mask_region ); 
  VIPS_UNREF( display_image ); 
  VIPS_UNREF( mask ); 

  convert2srgb->get_par()->set_image_hints( image );
  convert2srgb->get_par()->set_format( get_view()->get_format() );
  std::vector<VipsImage*> in; in.push_back( image );
  VipsImage* srgbimg = convert2srgb->get_par()->build(in, 0, NULL, NULL, level );
  g_object_unref( image );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): image="<<image<<"   ref_count="<<G_OBJECT( image )->ref_count<<std::endl;
#endif
  //outimg = srgbimg;

  in.clear();
  in.push_back( srgbimg );
  convert_format->get_par()->set_image_hints( srgbimg );
  convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
  outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  g_object_unref( srgbimg );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): srgbimg="<<srgbimg<<"   ref_count="<<G_OBJECT( srgbimg )->ref_count<<std::endl;
#endif
  //getchar();
  /*
    VipsImage* srgbimg2;
    vips_cast( srgbimg, &srgbimg2, VIPS_FORMAT_UCHAR, NULL );
    g_object_unref( srgbimg );
    outimg = srgbimg2;
  */
    
  display_image = im_open( "display_image", "p" );
  mask = vips_image_new();

  if (vips_sink_screen (outimg, display_image, mask,
			64, 64, (2000/64)*(2000/64), 
			0, sink_notify, this))
    vips::verror ();
  g_object_unref( outimg );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): srgbimg="<<outimg<<"   ref_count="<<G_OBJECT( outimg )->ref_count<<std::endl;
#endif
  
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): vips_sink_screen() called"<<std::endl;
#endif
  region = vips_region_new (display_image);

#ifdef DEBUG_DISPLAY
  std::cout<<"Image size: "<<display_image->Xsize<<","
	   <<display_image->Ysize<<std::endl;
#endif

  mask_region = vips_region_new( mask );

  set_size_request (display_image->Xsize, display_image->Ysize);

  // Request a complete redraw of the image area. 
  //set_processing( true );
  queue_draw();
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): queue_draw() called"<<std::endl;
#endif
}
