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


#include "../base/imageprocessor.hh"
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

#define OPTIMIZE_SCROLLING

PF::ImageArea::ImageArea( Pipeline* v ):
  PipelineSink( v ),
  hadj( NULL ),
  vadj( NULL ),
  xoffset( 0 ),
  yoffset( 0 ),
  pending_pixels( 0 ),
  display_merged( true ),
  active_layer( -1 ),
	shrink_factor( 1 )
{
  outimg = NULL;
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
  maskblend = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  invert = new PF::Processor<PF::InvertPar,PF::Invert>();
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  set_size_request( 100, 100 );

  draw_done = vips_g_cond_new();
  draw_mutex = vips_g_mutex_new();

  //get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
  //set_double_buffered( TRUE );

  //signal_queue_draw.connect(sigc::mem_fun(*this, &ImageArea::queue_draw));
}

PF::ImageArea::~ImageArea ()
{
  std::cout<<"Deleting image area"<<std::endl;
  PF_UNREF( region, "ImageArea::~ImageArea()" );
  PF_UNREF( display_image, "ImageArea::~ImageArea()" );
  PF_UNREF( outimg, "ImageArea::~ImageArea()" );
  delete convert2srgb;
  delete convert_format;
  delete invert;
  delete uniform;
  //delete pf_image;
}



// Submit the given area to the image processor
void PF::ImageArea::submit_area( const VipsRect& area )
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::submit_area(): called"<<std::endl;
  std::cout<<"                              display_image="<<display_image<<std::endl;
  std::cout<<"                              xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif

  // Submit request to re-process the area
  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return;

  //VipsRect image = {xoffset, yoffset, display_image->Xsize, display_image->Ysize};
  VipsRect image = {0, 0, display_image->Xsize, display_image->Ysize};
  //VipsRect area = {expose->x, expose->y, expose->width, expose->height};
  VipsRect clip, area_clip;
  
  vips_rect_intersectrect (&image, &area, &area_clip);
  if( (area_clip.width <= 0) ||
      (area_clip.height <= 0) ) 
    return;

  clip.left = area_clip.left;// - xoffset;
  clip.top = area_clip.top;// - yoffset;
  clip.width = area_clip.width;
  clip.height = area_clip.height;
  
  ProcessRequestInfo request;
  request.sink = this;
  request.area = clip;
  request.request = PF::IMAGE_REDRAW;
  //request.done = drawing_done;
  //request.mutex = drawing_mutex;

  //std::cout<<"PF::ImageArea::submit_area(): submitting redraw request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::submit_area(): redraw request submitted."<<std::endl;
}


/* Prepare the inactive buffer for the next drawing operations
   The main thread never uses the inactive buffer, therefore there is
   no need to protect this part of code with a mutex
*/
void PF::ImageArea::process_start( const VipsRect& area )
{
  // Resize the inactive buffer to match the size of the image portion being displayed
  double_buffer.get_inactive().resize( area.left, area.top, area.width, area.height );

  // Copy the active buffer into the inactive one
  // The regions that are not in common should be filled by the
  // subsequent draw operations
  double_buffer.get_inactive().copy( double_buffer.get_active() );

	// Fill borders with black
	int right_border = area.width - display_image->Ysize - xoffset;
	int bottom_border = area.height - display_image->Ysize - yoffset;
	VipsRect top = {0, 0, area.width, yoffset };
	VipsRect bottom = {0, area.height-bottom_border-1, area.width, bottom_border };
	VipsRect left = {0, yoffset, xoffset, display_image->Ysize };
	VipsRect right = {area.width-right_border-1, yoffset, right_border, display_image->Ysize };
	double_buffer.get_inactive().fill( top, 0 );
	double_buffer.get_inactive().fill( bottom, 0 );
	double_buffer.get_inactive().fill( left, 0 );
	double_buffer.get_inactive().fill( right, 0 );
}


/* Terminate a drawing sequence. The buffers get swapped and an idle callback is installed
   to copy the active buffer to screen. As we are modifying the active buffer we need
   to protect the code with the double buffer mutex
*/
void PF::ImageArea::process_end( const VipsRect& area )
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::process_end(): called"<<std::endl;
  std::cout<<"                               display_image="<<display_image<<std::endl;
  std::cout<<"                               xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif
  double_buffer.lock();
  double_buffer.swap();

  Update * update = g_new (Update, 1);
  update->image_area = this;
  //std::cout<<"PF::ImageArea::process_end(): installing idle callback."<<std::endl;
  gdk_threads_add_idle ((GSourceFunc) render_cb, update);
  double_buffer.unlock();
}


// Process the given area and stores the resulting vips buffer
// The function installs an idle callback that copies the buffer to screen.
// Processing of any other area is blocked until the buffer is copied to the screen.
void PF::ImageArea::process_area( const VipsRect& area )
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::process_area(): called"<<std::endl;
  std::cout<<"                               display_image="<<display_image<<std::endl;
  std::cout<<"                               xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif

  VipsRect* parea = (VipsRect*)(&area);
  //vips_invalidate_area( display_image, parea );
  if (vips_region_prepare (region, parea))
    return;

  double_buffer.get_inactive().copy( region, area, xoffset, yoffset );
}


// Copy the given buffer to screen
void PF::ImageArea::draw_area()
{
  //std::cout<<"PF::ImageArea::draw_area(): before drawing pixbuf"<<std::endl;
  //getchar();
  double_buffer.lock();

  if( !(double_buffer.get_active().get_pxbuf()) ) {
    double_buffer.unlock();
    return;
  }

  //std::cout<<"PF::ImageArea::draw_area(): drawing area "
  //	   <<double_buffer.get_active().get_rect().width<<","<<double_buffer.get_active().get_rect().height
  //	   <<"+"<<double_buffer.get_active().get_rect().left<<"+"<<double_buffer.get_active().get_rect().top
  //	   <<std::endl;
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window ) return;
  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->rectangle( double_buffer.get_active().get_rect().left,
  		 double_buffer.get_active().get_rect().top,
  		 double_buffer.get_active().get_rect().width*3,
  		 double_buffer.get_active().get_rect().height*3 );
  cr->clip();
  /**/
  Gdk::Cairo::set_source_pixbuf( cr, double_buffer.get_active().get_pxbuf(), 
				 double_buffer.get_active().get_rect().left,
				 double_buffer.get_active().get_rect().top );
  cr->rectangle( double_buffer.get_active().get_rect().left,
  		 double_buffer.get_active().get_rect().top,
  		 double_buffer.get_active().get_rect().width,
  		 double_buffer.get_active().get_rect().height );
  cr->fill();
  /**/
  cr->paint();
  double_buffer.unlock();
  //std::cout<<"PF::ImageArea::draw_area(): after drawing pixbuf"<<std::endl;
  //getchar();
}



#ifdef GTKMM_2
bool PF::ImageArea::on_expose_event (GdkEventExpose * event)
{
  //return true;

  GdkRectangle *expose;
  int i, n;

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::on_expose_event(): called."<<std::endl;
#endif
  //getchar();
  draw_area();
  gdk_region_get_rectangles (event->region, &expose, &n);
  int xmin=1000000, xmax=0, ymin=1000000, ymax=0;
  for (i = 0; i < n; i++) {
    //#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::on_expose_event(): region #"<<i<<": ("
	     <<expose[i].x<<","<<expose[i].y<<") ("
	     <<expose[i].width<<","<<expose[i].height<<")"<<std::endl;
    //#endif
    if( expose[i].x < xmin ) xmin = expose[i].x;
    if( expose[i].x > xmax ) xmax = expose[i].x;
    if( expose[i].y < ymin ) ymin = expose[i].y;
    if( expose[i].y > ymax ) ymax = expose[i].y;
  }
  //VipsRect area_tot = {xmin, ymin, xmax-xmin+1, ymax-ymin+1};
  VipsRect area_tot = {
    hadj->get_value(), vadj->get_value(),
    hadj->get_page_size(), vadj->get_page_size()
  };
  ProcessRequestInfo request;
  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_START;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_start request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_expose_event(): redraw_start request submitted."<<std::endl;


  area_tot = {event->area.x, event->area.y, event->area.width, event->area.height};
  submit_area( area_tot );

  //getchar();
  for (i = 0; i < n; i++) {
#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::on_expose_event(): processing region ("
	     <<expose[i].x<<","<<expose[i].y<<") ("
	     <<expose[i].width<<","<<expose[i].height<<")"<<std::endl;
#endif
    //expose_rect (&expose[i]);
    VipsRect area = {expose[i].x, expose[i].y, expose[i].width, expose[i].height};
    //submit_area( area );
  }
  g_free( expose );

  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_END;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_end request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_expose_event(): redraw_end request submitted."<<std::endl;
  return TRUE;
}
#endif

#ifdef GTKMM_3
bool PF::ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  //std::cout<<"ImageArea::on_draw() called."<<std::endl;
  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return true;

	// Immediately draw the buffered image, to avoid flickering
  draw_area();

  //return true;

  VipsRect area_tot = {
    hadj->get_value(), vadj->get_value(),
    hadj->get_page_size(), vadj->get_page_size()
  };
	//std::cout<<"ImageArea::on_draw(): area_tot="<<area_tot.width<<","<<area_tot.height
	//				 <<"+"<<area_tot.left<<","<<area_tot.top<<std::endl;

	if( display_image->Xsize < hadj->get_page_size() ) {
		xoffset = (hadj->get_page_size()-display_image->Xsize)/2;
	} else {
		xoffset = 0;
	}
	if( display_image->Ysize < vadj->get_page_size() ) {
		yoffset = (vadj->get_page_size()-display_image->Ysize)/2;
	} else {
		yoffset = 0;
	}

	// Submit the re-computation of the requested area
  ProcessRequestInfo request;
  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_START;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_start request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_expose_event(): redraw_start request submitted."<<std::endl;

#ifdef OPTIMIZE_SCROLLING
	double x1, y1, x2, y2;
	cr->get_clip_extents( x1, y1, x2, y2 );
	int ix1=x1, iy1=y1, ix2=x2, iy2=y2;
  area_tot.left = ix1;
	area_tot.top = iy1;
	area_tot.width = ix2+1-ix1;
	area_tot.height = iy2+1-iy1;
	//std::cout<<"ImageArea::on_draw(): area_tot2="<<area_tot.width<<","<<area_tot.height
	//				 <<"+"<<area_tot.left<<","<<area_tot.top<<std::endl;

  cairo_rectangle_list_t *list =  cairo_copy_clip_rectangle_list (cr->cobj());
  for (int i = list->num_rectangles - 1; i >= 0; --i) {
    cairo_rectangle_t *rect = &list->rectangles[i];

#ifdef DEBUG_DISPLAY    
    std::cout<<"ImageArea::on_draw(): rectangle = "<<rect->width<<","<<rect->height
	     <<"+"<<rect->x<<","<<rect->y<<std::endl;
#endif
    VipsRect area = {rect->x, rect->y, rect->width, rect->height};
    submit_area( area );
  }
#else
  submit_area( area_tot );
#endif


  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_END;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_end request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_expose_event(): redraw_end request submitted."<<std::endl;
#ifdef DEBUG_DISPLAY    
  std::cout<<std::endl;
#endif
  
  return true;
}
#endif



void PF::ImageArea::update( VipsRect* area ) 
{
  //PF::Pipeline* pipeline = pf_image->get_pipeline(0);

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): called"<<std::endl;
#endif
  if( !get_pipeline() ) {
    std::cout<<"ImageArea::update(): error: NULL pipeline"<<std::endl;
    return;
  }
  if( !get_pipeline()->get_output() ) {
    std::cout<<"ImageArea::update(): error: NULL image"<<std::endl;
    return;
  }

  //return;

  VipsImage* image = NULL;
  if( display_merged || (active_layer<0) ) {
    image = get_pipeline()->get_output();
  } else {
    PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
    if( !node ) return;
    if( !(node->blended) ) return;

    if( node->processor &&
				node->processor->get_par() &&
				!(node->processor->get_par()->is_map()) ) {
      image = node->blended;
    } else {
      PF::Layer* container_layer = 
				get_pipeline()->get_image()->get_layer_manager().
				get_container_layer( active_layer );
      if( !container_layer ) return;

      PF::PipelineNode* container_node = 
				get_pipeline()->get_node( container_layer->get_id() );
      if( !container_node ) return;
      if( container_node->input_id < 0 ) return;

      PF::Layer* input_layer = 
				get_pipeline()->get_image()->get_layer_manager().
				get_layer( container_node->input_id );
      if( !input_layer ) return;

      PF::PipelineNode* input_node = 
				get_pipeline()->get_node( input_layer->get_id() );
      if( !input_node ) return;
      if( !(input_node->blended) ) return;

      image = input_node->blended;
    }
  }
  if( !image ) return;

  unsigned int level = get_pipeline()->get_level();
  //outimg = image;

  //VIPS_UNREF( region ); 
  //VIPS_UNREF( display_image ); 
  PF_UNREF( outimg, "ImageArea::update() outimg unref" );
  PF_UNREF( region, "ImageArea::update() region unref" );
  PF_UNREF( display_image, "ImageArea::update() display_image unref" );

  convert2srgb->get_par()->set_image_hints( image );
  convert2srgb->get_par()->set_format( get_pipeline()->get_format() );
  std::vector<VipsImage*> in; in.push_back( image );
  VipsImage* srgbimg = convert2srgb->get_par()->build(in, 0, NULL, NULL, level );
  //PF_UNREF( image, "ImageArea::update() image unref" );
  // "image" is managed by photoflow, therefore it is not necessary to unref it
  // after the call to convert2srgb: the additional reference is owned by
  // "srgbimg" and will be removed when "srgbimg" is deleted.
  // This works also in the case when "image" and "srgbimg" are the same object,
  // as the additional reference will be removed when calling 
  // g_object_unref(srgbimg) later on

#ifndef NDEBUG
  PF_PRINT_REF( srgbimg, "ImageArea::update(): srgbimg after convert2srgb: " );
  std::cout<<"ImageArea::update(): image="<<image<<"   ref_count="<<G_OBJECT( image )->ref_count<<std::endl;
#endif
  //outimg = srgbimg;
    
  if( !display_merged && (active_layer>=0) ) {
    PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
    if( !node ) return;
    if( !(node->blended) ) return;

    if( node->processor &&
				node->processor->get_par() &&
				node->processor->get_par()->is_map() ) {

      invert->get_par()->set_image_hints( node->blended );
      invert->get_par()->set_format( get_pipeline()->get_format() );
      in.clear(); in.push_back( node->blended );
      VipsImage* mapinverted = invert->get_par()->build(in, 0, NULL, NULL, level );
      //g_object_unref( node->image );
      //PF_UNREF( node->image, "ImageArea::update() node->image unref" );

      uniform->get_par()->set_image_hints( srgbimg );
      uniform->get_par()->set_format( get_pipeline()->get_format() );
      //in.clear(); in.push_back( srgbimg );
      VipsImage* redimage = uniform->get_par()->build(in, 0, NULL, NULL, level );
      //g_object_unref( srgbimg );
      //g_object_unref( mapinverted );

      maskblend->get_par()->set_image_hints( srgbimg );
      maskblend->get_par()->set_format( get_pipeline()->get_format() );
      maskblend->get_par()->set_blend_mode( PF::PF_BLEND_NORMAL );
      maskblend->get_par()->set_opacity( 0.8 );
      in.clear(); 
      in.push_back( srgbimg );
      in.push_back( redimage );
      VipsImage* blendimage = maskblend->get_par()->build(in, 0, NULL, mapinverted, level );
      PF_UNREF( srgbimg, "ImageArea::update() srgbimg unref" );
      PF_UNREF( mapinverted, "ImageArea::update() mapinverted unref" );
      PF_UNREF( redimage, "ImageArea::update() redimage unref" );

      srgbimg = blendimage;
    }
  }


  in.clear();
  in.push_back( srgbimg );
  convert_format->get_par()->set_image_hints( srgbimg );
  convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
  outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  //g_object_unref( srgbimg );
  PF_UNREF( srgbimg, "ImageArea::update() srgbimg unref" );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): srgbimg="<<srgbimg<<"   ref_count="<<G_OBJECT( srgbimg )->ref_count<<std::endl;
#endif

  //g_object_unref( outimg );
  //PF_UNREF( outimg, "ImageArea::update() outimg unref" );
#ifndef NDEBUG
  std::cout<<"ImageArea::update(): outimg="<<outimg<<"   ref_count="<<G_OBJECT( outimg )->ref_count<<std::endl;
#endif
  
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): vips_sink_screen() called"<<std::endl;
#endif
#ifdef DEBUG_DISPLAY
  std::cout<<"Image size: "<<outimg->Xsize<<","
					 <<outimg->Ysize<<std::endl;
  std::cout<<"Shrink factor: "<<shrink_factor<<std::endl;
#endif

	if( shrink_factor != 1 ) {
		VipsImage* outimg2;
		//if( vips_shrink( outimg, &outimg2, 
		//										 1.0d/shrink_factor, 1.0d/shrink_factor, NULL ) )
//	return;
			if( vips_affine( outimg, &outimg2, 
											 shrink_factor, 0, 0, shrink_factor, NULL ) )
				return;
			//std::cout<<"outimg: "<<outimg<<"  outimg2: "<<outimg2<<std::endl;
			PF_UNREF( outimg, "ImageArea::update() outimg unref after shrink" );
			outimg = outimg2;
	}

	/*
  if( hadj && vadj ) {
    unsigned int display_width = outimg->Xsize, 
      display_height = outimg->Ysize;
    if( outimg->Xsize < hadj->get_page_size() ) {
      xoffset = (hadj->get_page_size()-outimg->Xsize)/2;
      //xoffset = 0;
      display_width = hadj->get_page_size();
    } else {
      xoffset = 0;
    }
    if( outimg->Ysize < vadj->get_page_size() ) {
      yoffset = (vadj->get_page_size()-outimg->Ysize)/2;
      //yoffset = 0;
      display_height = vadj->get_page_size();
    } else {
      yoffset = 0;
    }
    set_size_request (display_width, display_height);
		std::cout<<"display_width: "<<display_width<<"  display_height: "<<display_height
						 <<"  xoffset: "<<xoffset<<"  yoffset: "<<yoffset<<std::endl;

		
		if( xoffset>0 || yoffset>0 ) {
			VipsImage* outimg2;
			if( vips_embed( outimg, &outimg2, 
											xoffset, yoffset, 
											display_width, display_height, 
											"extend", VIPS_EXTEND_BLACK, NULL ) )
				return;
			std::cout<<"outimg: "<<outimg<<"  outimg2: "<<outimg2<<std::endl;
			PF_UNREF( outimg, "ImageArea::update() outimg unref after embed" );
			outimg = outimg2;
		}
  } else {
    xoffset = yoffset = 0;
  }
	*/

  display_image = im_open( "display_image", "p" );

  region = vips_region_new (display_image);

	int tile_size = 128;
  if (vips_sink_screen2 (outimg, display_image, NULL,
												 tile_size, tile_size, (2000/tile_size)*(2000/tile_size), 
												 //6400, 64, (2000/64), 
												 0, NULL, this))
		return;
	//vips::verror ();

	/*
	if( area ) {
		vips_invalidate_area( display_image, area );
		process_start( *area );
		process_area( *area );
		process_end( *area );
	} else {
		// The whole image was modified, in this case 
		// we process the visible protion
		VipsRect clip;
		clip.left = hadj->get_value();
		clip.top = vadj->get_value();
		clip.width = hadj->get_page_size();
		clip.height = vadj->get_page_size();
		VipsRect img_area = {0, 0, display_image->Xsize, display_image->Ysize};
		vips_rect_intersectrect( &img_area, &clip, &clip );
		vips_invalidate_area( display_image, &img_area );
		vips_region_invalidate( region );
		process_start( clip );
		process_area( clip );
		process_end( clip );
	}

	return;
	*/

  // Request a complete redraw of the image area. 
  //set_processing( true );
  //signal_queue_draw.emit();
  Update * update = g_new (Update, 1);
  update->image_area = this;
  update->rect.width = outimg->Xsize;
  update->rect.height = outimg->Ysize;
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): installing set_size callback."<<std::endl;
#endif
  gdk_threads_add_idle ((GSourceFunc) set_size_cb, update);
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): set_size() called"<<std::endl;
#endif

  update = g_new (Update, 1);
  update->image_area = this;
  update->rect.width = update->rect.height = 0;
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): installing queue_draw callback."<<std::endl;
#endif
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): queue_draw() called"<<std::endl;
#endif
}



void PF::ImageArea::sink( const VipsRect& area ) 
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::sink( const VipsRect& area ) called"<<std::endl;
#endif

  PF::Pipeline* pipeline = get_pipeline();
  if( !pipeline ) return;
  if( !outimg ) return;
  int level = pipeline->get_level();
  float fact = 1.0f;
  for( unsigned int i = 0; i < level; i++ )
    fact /= 2.0f;
  fact *= shrink_factor;

  VipsRect scaled_area;
  scaled_area.left = area.left * fact;
  scaled_area.top = area.top * fact;
  scaled_area.width = area.width * fact + 1;
  scaled_area.height = area.height * fact + 1;

  //vips_image_invalidate_all( pipeline->get_output() );

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update( area ): called"<<std::endl;
  std::cout<<"                               display_image="<<display_image<<std::endl;
  std::cout<<"                               xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif

	//update( NULL );
	/*
  VipsImage* display_image2 = im_open( "display_image2", "p" );
  if (vips_sink_screen2 (outimg, display_image2, NULL,
												 64, 64, (2000/64)*(2000/64), 
												 //6400, 64, (2000/64), 
												 0, NULL, this))
		return;
	*/
  //VipsRegion* region2 = vips_region_new (display_image);
  VipsRegion* region2 = vips_region_new( outimg );
  //vips_invalidate_area( display_image, &scaled_area );
	//vips_region_invalidate( region2 );

  VipsRect* parea = (VipsRect*)(&scaled_area);
#ifdef DEBUG_DISPLAY
  std::cout<<"Preparing area "<<scaled_area.left<<","<<scaled_area.top<<"+"<<scaled_area.width<<"+"<<scaled_area.height<<std::endl;
#endif
  if (vips_region_prepare (region2, parea)) {
    std::cout<<"ImageArea::sink(): vips_region_prepare() failed."<<std::endl;
    return;
  }
  unsigned char* pout = (unsigned char*)VIPS_REGION_ADDR( region2, parea->left, parea->top ); 
	/*
	std::cout<<"Plotting scaled area "<<scaled_area.width<<","<<scaled_area.height
					 <<"+"<<scaled_area.left<<","<<scaled_area.top<<std::endl;
	guint8 *px1 = (guint8 *) VIPS_REGION_ADDR( region, scaled_area.left, scaled_area.top );
	int rs1 = VIPS_REGION_LSKIP( region );
	int bl1 = 3; 
	for( int y = 0; y < scaled_area.height; y++ ) {
		guint8* p1 = px1 + rs1*y;
		for( int x = 0; x < scaled_area.width*bl1; x+=bl1 ) {
			printf("%03d ",(int)p1[x]);
		}
		printf("\n");
	}
	*/

	/*
  // Request a redraw of the modified area. 
  Update * update = g_new (Update, 1);
  update->image_area = this;
	update->rect.left = scaled_area.left;
	update->rect.top = scaled_area.top;
  update->rect.width = scaled_area.width;
	update->rect.height = scaled_area.height;
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update( const VipsRect& area ): installing idle callback."<<std::endl;
#endif
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update( const VipsRect& area ): queue_draw() called"<<std::endl;
#endif
	*/

	/**/
	double_buffer.lock();
	double_buffer.get_active().copy( region2, scaled_area, xoffset, yoffset );
	Update * update = g_new (Update, 1);
	update->image_area = this;
	update->rect.width = update->rect.height = 0;
	//std::cout<<"PF::ImageArea::update( const VipsRect& area ): installing idle callback."<<std::endl;
	gdk_threads_add_idle ((GSourceFunc) render_cb, update);
	double_buffer.unlock();
	PF_UNREF( region2, "ImageArea::sink(): region2" );
	//PF_UNREF( display_image2, "ImageArea::sink(): display_image2" );
	/**/

	/*
	double_buffer.lock();
	double_buffer.get_active().copy( region, area );
	double_buffer.unlock();
	draw_area();
	*/
}
