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


/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  
  extern int
  vips_sink_screen2( VipsImage *in, VipsImage *out, VipsImage *mask, 
		     int tile_width, int tile_height, 
		     int max_tiles, 
		     int priority,
		     VipsSinkNotify notify, void *a );
  
  extern void vips_invalidate_area( VipsImage *image, VipsRect* r  );

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#ifndef NDEBUG
#define DEBUG_DISPLAY
#endif

PF::ImageArea::ImageArea( View* v ):
  ViewSink( v ),
  hadj( NULL ),
  vadj( NULL ),
  xoffset( 0 ),
  yoffset( 0 ),
  pending_pixels( 0 ),
  display_merged( true ),
  active_layer( -1 )
{
  display_image = NULL;
  region = NULL;
  mask = NULL;
  mask_region = NULL;
  convert2srgb = new PF::Processor<PF::Convert2sRGBPar,PF::Convert2sRGBProc>();
  uniform = new PF::Processor<PF::UniformPar,PF::Uniform>();
  if( uniform->get_par() ) {
    uniform->get_par()->get_R().set( 1 );
    uniform->get_par()->get_G().set( 0 );
    uniform->get_par()->get_B().set( 0 );
  }
  invert = new PF::Processor<PF::InvertPar,PF::Invert>();
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
  std::cout<<"PF::ImageArea::expose_rect(): xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif
  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return;

  VipsRect image = {xoffset, yoffset, display_image->Xsize, display_image->Ysize};
  //VipsRect area = {expose->x, expose->y, expose->width, expose->height};
  VipsRect clip, area_clip;
  
  vips_rect_intersectrect (&image, &area, &area_clip);
  if( (area_clip.width > 0) &&
      (area_clip.height > 0) ) { 

    clip.left = area_clip.left - xoffset;
    clip.top = area_clip.top - yoffset;
    clip.width = area_clip.width;
    clip.height = area_clip.height;
  
  
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
#ifdef DEBUG_DISPLAY
	  std::cout<<"PF::ImageArea::expose_rect(): found paointed pixel @ "<<x+clip.left<<","<<y+clip.top<<std::endl;
#endif
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
				    area_clip.left, area_clip.top, area_clip.width, area_clip.height,
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
      Gdk::Cairo::set_source_pixbuf( cr, pixbuf, area_clip.left, area_clip.top );
      cr->rectangle( area_clip.left, area_clip.top, area_clip.width, area_clip.height );
      cr->fill();
#endif
      pending_pixels -= clip.width*clip.height;
#ifdef DEBUG_DISPLAY
      std::cout<<"PF::ImageArea::expose_rect(): rectangle done ("<<clip.width*clip.height<<")"<<std::endl;
      //std::cout<<"PF::ImageArea::expose_rect(): pending_pixels="<<pending_pixels<<std::endl;
#endif
    } else {
      pending_pixels += clip.width*clip.height;
    }
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

  VipsImage* image = NULL;
  if( display_merged || (active_layer<0) ) {
    image = get_view()->get_output();
  } else {
    PF::ViewNode* node = get_view()->get_node( active_layer );
    if( !node ) return;
    if( !(node->image) ) return;

    if( node->processor &&
	node->processor->get_par() &&
	!(node->processor->get_par()->is_map()) ) {
      image = node->image;
    } else {
      PF::Layer* container_layer = 
	get_view()->get_image()->get_layer_manager().
	get_container_layer( active_layer );
      if( !container_layer ) return;

      PF::ViewNode* container_node = 
	get_view()->get_node( container_layer->get_id() );
      if( !container_node ) return;
      if( container_node->input_id < 0 ) return;

      PF::Layer* input_layer = 
	get_view()->get_image()->get_layer_manager().
	get_layer( container_node->input_id );
      if( !input_layer ) return;

      PF::ViewNode* input_node = 
	get_view()->get_node( input_layer->get_id() );
      if( !input_node ) return;
      if( !(input_node->image) ) return;

      image = input_node->image;
    }
  }
  if( !image ) return;

  unsigned int level = get_view()->get_level();
  outimg = image;

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
    
  if( !display_merged && (active_layer>=0) ) {
    PF::ViewNode* node = get_view()->get_node( active_layer );
    if( !node ) return;
    if( !(node->image) ) return;

    if( node->processor &&
	node->processor->get_par() &&
	node->processor->get_par()->is_map() ) {

      invert->get_par()->set_image_hints( node->image );
      invert->get_par()->set_format( get_view()->get_format() );
      in.clear(); in.push_back( node->image );
      VipsImage* mapinverted = invert->get_par()->build(in, 0, NULL, NULL, level );
      g_object_unref( node->image );

      uniform->get_par()->set_image_hints( srgbimg );
      uniform->get_par()->set_format( get_view()->get_format() );
      uniform->get_par()->set_blend_mode( PF::PF_BLEND_NORMAL );
      uniform->get_par()->set_opacity( 0.8 );
      in.clear(); in.push_back( srgbimg );
      VipsImage* mapimage = uniform->get_par()->build(in, 0, NULL, mapinverted, level );
      g_object_unref( srgbimg );
      g_object_unref( mapinverted );
      srgbimg = mapimage;
    }
  }


  in.clear();
  in.push_back( srgbimg );
  convert_format->get_par()->set_image_hints( srgbimg );
  convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
  outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  g_object_unref( srgbimg );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): srgbimg="<<srgbimg<<"   ref_count="<<G_OBJECT( srgbimg )->ref_count<<std::endl;
#endif

  display_image = im_open( "display_image", "p" );
  mask = vips_image_new();

  if (vips_sink_screen2 (outimg, display_image, mask,
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

  if( hadj && vadj ) {
    unsigned int display_width = display_image->Xsize, 
      display_height = display_image->Ysize;
    if( display_image->Xsize < hadj->get_page_size() ) {
      xoffset = (hadj->get_page_size()-display_image->Xsize)/2;
      xoffset = 0;
      display_width = hadj->get_page_size();
    } else {
      xoffset = 0;
    }
    if( display_image->Ysize < vadj->get_page_size() ) {
      yoffset = (vadj->get_page_size()-display_image->Ysize)/2;
      yoffset = 0;
      display_height = vadj->get_page_size();
    } else {
      yoffset = 0;
    }
    set_size_request (display_width, display_height);
  } else {
    xoffset = yoffset = 0;
  }

  // Request a complete redraw of the image area. 
  //set_processing( true );
  queue_draw();
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): queue_draw() called"<<std::endl;
#endif
}



void PF::ImageArea::update( const VipsRect& area ) 
{
#ifndef NDEBUG
  std::cout<<"PF::ImageArea::update( const VipsRect& area ) called"<<std::endl;
#endif

  PF::View* view = get_view();
  if( !view ) return;
  int level = view->get_level();
  float fact = 1.0f;
  for( unsigned int i = 0; i < level; i++ )
    fact /= 2.0f;

  VipsRect scaled_area;
  scaled_area.left = area.left * fact;
  scaled_area.top = area.top * fact;
  scaled_area.width = area.width * fact;
  scaled_area.height = area.height * fact;

  vips_invalidate_area( display_image, &scaled_area );
  // display_image->kill = 1;
  // update(); return;
  // vips_image_write_to_file( outimg, "/tmp/test.tif" );
  // vips_image_invalidate_all( display_image );
  queue_draw_area( scaled_area.left, scaled_area.top, scaled_area.width, scaled_area.height );
}
