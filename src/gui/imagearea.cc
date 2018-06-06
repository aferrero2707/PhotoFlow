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
#include "../operations/operations.hh"
#include "../operations/icc_transform.hh"
#include "../operations/convert_colorspace.hh"
//#include "../operations/image_reader.hh"
//#include "../operations/convert2srgb.hh"
#include "../operations/uniform.hh"
#include "../operations/invert.hh"
#include "../operations/blender.hh"
#include "../operations/convertformat.hh"
#include "../operations/clipping_warning.hh"

#include "../gui/operation_config_gui.hh"
//#include "../gui/operations/imageread_config.hh"

#include "imagearea.hh"

#define DISPLAY_TILE_SIZE 128


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

//#define OPTIMIZE_SCROLLING


static std::ostream& operator <<( std::ostream& str, const VipsRect& r )
{
  str<<r.width<<","<<r.height<<"+"<<r.left<<"+"<<r.top;
  return str;
}


gboolean PF::ImageArea::queue_draw_cb (PF::ImageArea::Update * update)
{
  if( false && (update->rect.width > 0) && (update->rect.height > 0) ) {
    update->image_area->queue_draw_area (update->rect.left,//+update->image_area->get_xoffset(),
                                         update->rect.top,//+update->image_area->get_yoffset(),
                                         update->rect.width,
                                         update->rect.height);
    //std::cout<<"queue_draw_cb(): queued area: "<<update->rect<<std::endl;
  } else {
    update->image_area->queue_draw();
    //std::cout<<"queue_draw_cb(): queue_draw() called."<<std::endl;
  }
  /*
  */
  g_free (update);

  return FALSE;
}



//gboolean PF::ImageArea::set_size_cb (PF::ImageArea::Update * update)
void PF::ImageArea::set_size_cb ()
{
  //std::cout<<"set_size_cb() called."<<std::endl;
  /*
  std::cout<<"set_size_cb(1): ->get_hadj()->get_value()="<<->get_hadj()->get_value()<<std::endl;
  std::cout<<"                get_lower()="<<->get_hadj()->get_lower()<<std::endl;
  std::cout<<"                get_upper()="<<->get_hadj()->get_upper()<<std::endl;
  std::cout<<"                get_page_size()="<<->get_hadj()->get_page_size()<<std::endl;
  std::cout<<"set_size_cb(1): ->get_vadj()->get_value()="<<get_vadj()->get_value()<<std::endl;
  std::cout<<"                get_lower()="<<get_vadj()->get_lower()<<std::endl;
  std::cout<<"                get_upper()="<<get_vadj()->get_upper()<<std::endl;
  std::cout<<"                get_page_size()="<<get_vadj()->get_page_size()<<std::endl;
  std::cout<<"set_size_request("<<preview_size.width<<","<<preview_size.height<<")"<<std::endl;
  */
  //std::cout<<"set_size_cb(): preview_size="<<preview_size<<std::endl;

  g_mutex_lock(preview_size_mutex);

  set_size_request(preview_size.width,preview_size.height);
  // We need to change the upper limits explicitely, since the adjustments are not updated immediately
  // when calling set_size_request(). Otherwise the subsequent set_value() calls might modify the values
  // internally to make sure that value+page_size does not exceed the upper limit.
  if( preview_size.width > get_hadj()->get_page_size() )
      get_hadj()->set_upper( preview_size.width );
  else
    get_hadj()->set_upper( get_hadj()->get_page_size() );
  if( preview_size.height > get_vadj()->get_page_size() )
      get_vadj()->set_upper( preview_size.height );
  else
    get_vadj()->set_upper( get_vadj()->get_page_size() );
  /*
  std::cout<<"set_size_cb(2): preview_size.left="<<preview_size.left<<std::endl;
  std::cout<<"set_size_cb(2): preview_size.top="<<preview_size.top<<std::endl;
  */
  get_hadj()->set_value( preview_size.left );
  get_vadj()->set_value( preview_size.top );

  g_mutex_unlock(preview_size_mutex);

  /*
  std::cout<<"set_size_cb(2): get_hadj()->get_value()="<<get_hadj()->get_value()<<std::endl;
  std::cout<<"                get_lower()="<<get_hadj()->get_lower()<<std::endl;
  std::cout<<"                get_upper()="<<get_hadj()->get_upper()<<std::endl;
  std::cout<<"                get_page_size()="<<get_hadj()->get_page_size()<<std::endl;
  std::cout<<"set_size_cb(2): get_vadj()->get_value()="<<get_vadj()->get_value()<<std::endl;
  std::cout<<"                get_lower()="<<get_vadj()->get_lower()<<std::endl;
  std::cout<<"                get_upper()="<<get_vadj()->get_upper()<<std::endl;
  std::cout<<"                get_page_size()="<<get_vadj()->get_page_size()<<std::endl;
  std::cout<<std::endl;
  */

  //queue_draw();
  //std::cout<<"set_size_cb(): queue_draw() called."<<std::endl;

  /*
  // Rectangle corresponding to the preview area
  VipsRect preview_area = {
      static_cast<int>(get_hadj()->get_value()),
      static_cast<int>(get_vadj()->get_value()),
      static_cast<int>(get_hadj()->get_page_size()),
      static_cast<int>(get_vadj()->get_page_size())
  };*/

  //queue_draw_area (preview_area.left,//+get_xoffset(),
  //                                     preview_area.top,//+get_yoffset(),
  //                                     preview_area.width,
  //                                     preview_area.height);
  queue_draw();
  //std::cout<<"set_size_cb(): queue_draw() called"<<std::endl;

  /*
  PF::ImageArea::Update* update2 = g_new (Update, 1);
  update2->image_area = update->image_area;
  //update2->rect.width = update2->rect.height = 0;
  update2->rect = preview_area;
#ifdef DEBUG_DISPLAY
  std::cout<<"set_size_cb(): installing queue_draw callback."<<std::endl;
#endif
  //gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update2);
#ifdef DEBUG_DISPLAY
  std::cout<<"set_size_cb(): queue_draw() called"<<std::endl;
#endif
  */
  //g_free (update);
  //return FALSE;
}



PF::ImageArea::ImageArea( Pipeline* v ):
  PipelineSink( v ),
  hadj( NULL ),
  vadj( NULL ),
  xoffset( 0 ),
  yoffset( 0 ),
  pending_pixels( 0 ),
  draw_requested( false ),
  softproof_enabled( false ),
  current_display_profile_type( PF_DISPLAY_PROF_MAX ),
  current_display_profile( NULL ),
  highlights_warning_enabled( false ),
  shadows_warning_enabled( false ),
  softproof_bpc_enabled( false ),
  sim_black_ink_enabled( false ),
  sim_paper_color_enabled( false ),
  gamut_warning_enabled( false ),
  softproof_clip_negative_enabled( true ),
  softproof_clip_overflow_enabled( true ),
  adaptation_state(1),
  display_merged( true ),
  display_mask( false ),
  displayed_layer( -1 ),
  selected_layer( -1 ),
  edited_layer( -1 ),
	shrink_factor( 1 ),
	target_area_center_x( -1 ),
	target_area_center_y( -1 )
{
  outimg = NULL;
  display_image = NULL;
  region = NULL;
  mask = NULL;
  mask_region = NULL;
  //convert2display = new PF::Processor<PF::Convert2sRGBPar,PF::Convert2sRGBProc>();
  softproof_conversion = new_convert_colorspace();
  convert2display = new_icc_transform();
  uniform = new_uniform();
  if( uniform->get_par() ) {
    PF::UniformPar* uniform_par = dynamic_cast<PF::UniformPar*>( uniform->get_par() );
    if( uniform_par ) {
      uniform_par->get_R().set( 1 );
      uniform_par->get_G().set( 0 );
      uniform_par->get_B().set( 0 );
    }
  }
  maskblend = new_blender();
  invert = new_invert();
  convert_format = new_convert_format();
  clipping_warning = new_clipping_warning();

  set_size_request( 1, 1 );

  draw_done = vips_g_cond_new();
  draw_mutex = vips_g_mutex_new();

  preview_size_mutex = vips_g_mutex_new();
  signal_set_size.connect(sigc::mem_fun(*this, &ImageArea::set_size_cb));

  //get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
  //set_double_buffered( TRUE );

  signal_queue_draw.connect(sigc::mem_fun(*this, &ImageArea::queue_draw));
}

PF::ImageArea::~ImageArea ()
{
#ifndef NDEBUG
  std::cout<<"Deleting image area"<<std::endl;
  std::cout<<"~ImageArea finished"<<std::endl;
#endif
  //delete pf_image;
}


void PF::ImageArea::dispose()
{
  std::cout<<"mageArea::dispose() called"<<std::endl;
  PF_UNREF( region, "ImageArea::dispose()" );
  PF_UNREF( display_image, "ImageArea::dispose()" );
  PF_UNREF( outimg, "ImageArea::dispose()" );
  delete convert2display;
  delete convert_format;
  delete invert;
  delete uniform;
  std::cout<<"mageArea::dispose() fnished"<<std::endl;
}



// Submit the given area to the image processor
void PF::ImageArea::submit_area( const VipsRect& area )
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::submit_area(): called"<<std::endl;
  std::cout<<"                              display_image="<<display_image<<std::endl;
  std::cout<<"                              xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
  std::cout<<"                              area="<<area<<std::endl;
  // Rectangle corresponding to the preview area
  VipsRect preview_area = {
      static_cast<int>(hadj->get_value()), static_cast<int>(vadj->get_value()),
      static_cast<int>(hadj->get_page_size()), static_cast<int>(vadj->get_page_size())
  };
  std::cout<<"                              preview_area="<<preview_area<<std::endl;
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
#ifdef DEBUG_DISPLAY
  std::cout<<"Active buffer copied into inactive one"<<std::endl;
#endif

  /* OBSOLETE
	// Fill borders with black
	int right_border = area.width - display_image->Xsize - xoffset;
	int bottom_border = area.height - display_image->Ysize - yoffset;
	VipsRect top = {0, 0, area.width, static_cast<int>(yoffset) };
	VipsRect bottom = {0, area.height-bottom_border-1, area.width, bottom_border };
	VipsRect left = {0, static_cast<int>(yoffset), static_cast<int>(xoffset), display_image->Ysize };
	VipsRect right = {area.width-right_border-1, static_cast<int>(yoffset), right_border, display_image->Ysize };
	double_buffer.get_inactive().fill( top, 0 );
	double_buffer.get_inactive().fill( bottom, 0 );
	double_buffer.get_inactive().fill( left, 0 );
	double_buffer.get_inactive().fill( right, 0 );
	*/
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
  std::cout<<"                               image width="<<display_image->Xsize<<std::endl;
  std::cout<<"                                    height="<<display_image->Ysize<<std::endl;
  std::cout<<"                               xoffset="<<xoffset<<"  yoffset="<<yoffset<<std::endl;
#endif
  double_buffer.lock();
  double_buffer.swap();
  double_buffer.get_active().set_dirty(false);
  double_buffer.get_inactive().set_dirty(true);
#ifdef DEBUG_DISPLAY
  std::cout<<"Buffer swapped"<<std::endl;
#endif

  //g_mutex_lock(draw_mutex);
  //draw_area.left = area.left;
  //draw_area.top = area.top;
  //draw_area.width = area.width;
  //draw_area.height = area.height;
  //g_mutex_unlock(draw_mutex);

  signal_queue_draw.emit();

  /*
  Update * update = g_new (Update, 1);
  update->image_area = this;
  update->rect.left = area.left;
  update->rect.top = area.top;
  update->rect.width = area.width;
  update->rect.height = area.height;
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::process_end(): installing draw callback."<<std::endl;
#endif
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
  */
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
#ifdef DEBUG_DISPLAY
  std::cout<<"Preparing area "<<parea->width<<","<<parea->height<<"+"<<parea->left<<"+"<<parea->top<<" for display"<<std::endl;
  std::cout<<"  display_image: w="<<display_image->Xsize<<" h="<<display_image->Ysize<<std::endl;
  std::cout<<"  region->im: w="<<region->im->Xsize<<" h="<<region->im->Ysize<<std::endl;
#endif

  /*
  for(int y = (parea->top/DISPLAY_TILE_SIZE + 1)*DISPLAY_TILE_SIZE;
      y < (parea->top+parea->height); y+=DISPLAY_TILE_SIZE) {
    for(int x = (parea->left/DISPLAY_TILE_SIZE + 1)*DISPLAY_TILE_SIZE;
        x < (parea->left+parea->width); x+=DISPLAY_TILE_SIZE) {

      VipsRect r = {x, y, DISPLAY_TILE_SIZE, DISPLAY_TILE_SIZE};
      std::cout<<"Before vips_region_prepare(): "<<r.width<<"x"<<r.height<<"+"<<r.left<<","<<r.top<<std::endl;
      if (vips_region_prepare (region, &r))
        return;
      std::cout<<"After vips_region_prepare()"<<std::endl;

      VipsRect area_clip;
      vips_rect_intersectrect (&(region->valid), &r, &area_clip);

      #ifdef DEBUG_DISPLAY
        std::cout<<"Before copying region "<<parea->width<<","<<parea->height<<"+"<<parea->left<<"+"<<parea->top<<" into inactive buffer"<<std::endl;
      #endif
      double_buffer.get_inactive().copy( region, area_clip, xoffset, yoffset);
          //xoffset+(x-parea->left), yoffset+(y-parea->top) );
    }
  }
  return;
  */

  //if( region && region->buffer ) region->buffer->done = 0;
  if (vips_region_prepare (region, parea))
    return;

  VipsRect area_clip;
  vips_rect_intersectrect (&(region->valid), &area, &area_clip);

  #ifdef DEBUG_DISPLAY
    std::cout<<"Before copying region "<<parea->width<<","<<parea->height<<"+"<<parea->left<<"+"<<parea->top<<" into inactive buffer"<<std::endl;
  #endif
  double_buffer.get_inactive().copy( region, area_clip, xoffset, yoffset );
#ifdef DEBUG_DISPLAY
  std::cout<<"Region "<<parea->width<<","<<parea->height<<"+"<<parea->left<<"+"<<parea->top<<" copied into inactive buffer"<<std::endl;
#endif
}


// Pass the active Pixbuf to the current layer dialog to eventually
// draw additional informations on the preview image
Glib::RefPtr< Gdk::Pixbuf > PF::ImageArea::modify_preview()
{
  Glib::RefPtr< Gdk::Pixbuf > current_pxbuf = double_buffer.get_active().get_pxbuf();
  if( !current_pxbuf ) {
    return current_pxbuf;
  }

  unsigned int level = get_pipeline()->get_level();
  float zoom_fact = 1.0f;
  for( unsigned int i = 0; i < level; i++ )
    zoom_fact /= 2.0f;
  zoom_fact *= get_shrink_factor();

  // Resize the output buffer to match the input one
  temp_buffer.resize( double_buffer.get_active().get_rect() );

  // Copy pixel data from input to outout
  temp_buffer.copy( double_buffer.get_active() );
  current_pxbuf = temp_buffer.get_pxbuf();

  //std::cout<<"ImageArea::modify_preview() called. edited_layer="<<edited_layer<<std::endl;
  if( edited_layer >= 0 ) {
    PF::Image* image = get_pipeline()->get_image();
    if( image ) {
      PF::Layer* layer = image->get_layer_manager().get_layer( edited_layer );
      if( layer && layer->is_visible() &&
          layer->get_processor() &&
          layer->get_processor()->get_par() ) {
        PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
        PF::OperationConfigGUI* config = dynamic_cast<PF::OperationConfigGUI*>( ui );
        if( config && config->get_editing_flag() == true ) {
          //std::cout<<"Calling config->modify_preview()"<<std::endl;
          if( config->modify_preview(double_buffer.get_active(), temp_buffer, zoom_fact, xoffset, yoffset) )
            current_pxbuf = temp_buffer.get_pxbuf();
        }
      }
    }
  }

  //std::cout<<"ImageArea::modify_preview() called. samplers="<<samplers<<std::endl;
  if( samplers ) {
    for(int i = 0; i < samplers->get_sampler_num(); i++) {
      samplers->get_sampler(i).modify_preview(temp_buffer, zoom_fact, xoffset, yoffset);
    }
  }

  return current_pxbuf;
}


// Copy the given buffer to screen
void PF::ImageArea::draw_area()
{
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::draw_area(): before drawing pixbuf"<<std::endl;
#endif
  //getchar();
  double_buffer.lock();

  if( !(double_buffer.get_active().get_pxbuf()) ) {
    draw_requested = false;
    double_buffer.unlock();
    return;
  }

  Glib::RefPtr< Gdk::Pixbuf > current_pxbuf = modify_preview();
  /*
  Glib::RefPtr< Gdk::Pixbuf > current_pxbuf = double_buffer.get_active().get_pxbuf();
  if( displayed_layer >= 0 ) {
    PF::Image* image = get_pipeline()->get_image();
    if( image ) {
      PF::Layer* layer = image->get_layer_manager().get_layer( displayed_layer );
      if( layer &&
          layer->get_processor() &&
          layer->get_processor()->get_par() ) {
        PF::OperationConfigUI* ui = layer->get_processor()->get_par()->get_config_ui();
        PF::OperationConfigGUI* config = dynamic_cast<PF::OperationConfigGUI*>( ui );
        if( config && config->get_editing_flag() == true ) {
          int level = get_pipeline()->get_level();
          float zoom_fact = 1.0f;
          for( unsigned int i = 0; i < level; i++ )
            zoom_fact /= 2.0f;
          zoom_fact *= get_shrink_factor();
          if( config->modify_preview(double_buffer.get_active(), temp_buffer, zoom_fact, xoffset, yoffset) )
            current_pxbuf = temp_buffer.get_pxbuf();
        }
      }
    }
  }
  */

  //std::cout<<"PF::ImageArea::draw_area(): drawing area "
  //     <<double_buffer.get_active().get_rect().width<<","<<double_buffer.get_active().get_rect().height
  //     <<"+"<<double_buffer.get_active().get_rect().left<<"+"<<double_buffer.get_active().get_rect().top
  //     <<std::endl;
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window ) {
    draw_requested = false;
    double_buffer.unlock();
    return;
  }
  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
  cr->rectangle( double_buffer.get_active().get_rect().left,
       double_buffer.get_active().get_rect().top,
       double_buffer.get_active().get_rect().width*3,
       double_buffer.get_active().get_rect().height*3 );
  cr->clip();

  Gdk::Cairo::set_source_pixbuf( cr, current_pxbuf, 
         double_buffer.get_active().get_rect().left,
         double_buffer.get_active().get_rect().top );
#ifdef DEBUG_DISPLAY
  std::cout<<"Active buffer copied to screen"<<std::endl;
#endif
  cr->rectangle( double_buffer.get_active().get_rect().left,
       double_buffer.get_active().get_rect().top,
       double_buffer.get_active().get_rect().width,
       double_buffer.get_active().get_rect().height );
  cr->fill();

  cr->paint();
  draw_requested = false;
  double_buffer.unlock();
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::draw_area(): after drawing pixbuf"<<std::endl;
#endif
  //getchar();
}



#ifdef GTKMM_2
bool PF::ImageArea::on_expose_event (GdkEventExpose * event)
{
  //return true;
  //std::cout<<"ImageArea::on_expose_event() called"<<std::endl;

  // We draw only if there is already a VipsImage attached to this display
  if( !display_image ) return true;

  GdkRectangle expose_area;
  gdk_region_get_clipbox( event->region, &expose_area );

  VipsRect draw_area = { expose_area.x, expose_area.y, expose_area.width, expose_area.height };
  int draw_area_right = draw_area.left + draw_area.width - 1;
  int draw_area_bottom = draw_area.top + draw_area.height - 1;

  std::cout<<"ImageArea::on_expose_event(): draw_area="<<draw_area<<std::endl;
  // Immediately draw the buffered image, to avoid flickering
  // If the requested area is fully contained witin the current preview buffer,
  // we do not submit any further redraw request
  double_buffer.lock();
  //std::cout<<"  draw_area: "<<draw_area<<std::endl;
  //std::cout<<"  buffer_area: "<<double_buffer.get_active().get_rect()<<std::endl;
  //std::cout<<"  buffer_dirty: "<<double_buffer.get_active().is_dirty()<<std::endl;
  if( double_buffer.get_active().get_rect().width > 0 &&
      double_buffer.get_active().get_rect().height > 0 ) {

    Glib::RefPtr< Gdk::Pixbuf > pixbuf = modify_preview();
    std::cout<<"ImageArea::on_expose_event(): before get_window()->draw_pixbuf()"<<std::endl;
    /*
    get_window()->draw_pixbuf( get_style ()->get_white_gc (), pixbuf,
        0, 0,
        double_buffer.get_active().get_rect().left,
        double_buffer.get_active().get_rect().top,
        double_buffer.get_active().get_rect().width,
        double_buffer.get_active().get_rect().height,
        Gdk::RGB_DITHER_MAX, 0, 0 );
    */
    Cairo::RefPtr< Cairo::Context > cr = get_window()->create_cairo_context();
    Gdk::Cairo::set_source_pixbuf( cr, pixbuf,
        double_buffer.get_active().get_rect().left,
        double_buffer.get_active().get_rect().top );
    cr->paint();
    std::cout<<"ImageArea::on_expose_event(): after get_window()->draw_pixbuf()"<<std::endl;
  }

  bool repaint_needed = true;
  if( vips_rect_includesrect(&(double_buffer.get_active().get_rect()), &draw_area) ) {
    //std::cout<<"  vips_rect_includesrect(): double_buffer.get_active().get_rect()="
    //    <<double_buffer.get_active().get_rect()
    //    <<"  draw_area="<<draw_area
    //    <<std::endl;
    repaint_needed = false;
  }
  if( double_buffer.get_active().is_dirty() )
    repaint_needed = true;
  double_buffer.unlock();
  //std::cout<<"  repaint_needed="<<repaint_needed<<std::endl;
  draw_requested = repaint_needed;
  if( !repaint_needed )
    return true;

  // Rectangle corresponding to the preview area
  VipsRect preview_area = {
		  static_cast<int>(hadj->get_value()), static_cast<int>(vadj->get_value()),
		  static_cast<int>(hadj->get_page_size()), static_cast<int>(vadj->get_page_size())
  };
  int preview_area_right = preview_area.left + preview_area.width - 1;
  int preview_area_bottom = preview_area.top + preview_area.height - 1;

  // Total area to be allocated for the preview buffer
  // This might be larger than the preview area, because gtk might
  // ask for pixels that are outside of it (for example when scrolling)
  VipsRect area_tot;
  area_tot.left = MIN( draw_area.left, preview_area.left );
  area_tot.top = MIN( draw_area.top, preview_area.top );
  area_tot.width = MAX( draw_area_right, preview_area_right ) - area_tot.left + 1;
  area_tot.height = MAX( draw_area_bottom, preview_area_bottom ) - area_tot.top + 1;
  //std::cout<<"ImageArea::on_expose_event(): area_tot="<<area_tot<<std::endl;

  /* OBSOLETE
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
  */

  xoffset = 0;
  yoffset = 0;

  // Submit the re-computation of the requested area
  ProcessRequestInfo request;
  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_START;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_start request."<<std::endl;
  //std::cout<<"ImageArea::on_expose_event(): request.area="<<request.area<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_draw(): redraw_start request submitted."<<std::endl;

#ifdef OPTIMIZE_SCROLLING
  GdkRectangle *expose;
  int i, n;

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::on_expose_event(): called."<<std::endl;
#endif
  //getchar();

  gdk_region_get_rectangles (event->region, &expose, &n);
  int xmin=1000000, xmax=0, ymin=1000000, ymax=0;
  for (i = 0; i < n; i++) {
#ifdef DEBUG_DISPLAY
    std::cout<<"PF::ImageArea::on_expose_event(): region #"<<i<<": ("
	     <<expose[i].x<<","<<expose[i].y<<") ("
	     <<expose[i].width<<","<<expose[i].height<<")"<<std::endl;
#endif
    if( expose[i].x < xmin ) xmin = expose[i].x;
    if( expose[i].x > xmax ) xmax = expose[i].x;
    if( expose[i].y < ymin ) ymin = expose[i].y;
    if( expose[i].y > ymax ) ymax = expose[i].y;
  }

  //area_tot = {event->area.x, event->area.y, event->area.width, event->area.height};
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
#else

  //submit_area( draw_area );
  submit_area( area_tot );

#endif


  request.sink = this;
  //request.area = draw_area;
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

  //std::cout<<"  get_hadj()->get_value()="<<get_hadj()->get_value()<<std::endl;
  //std::cout<<"  get_vadj()->get_value()="<<get_vadj()->get_value()<<std::endl;

  draw_requested = false;

  // Total area requested for drawing
  double x1, y1, x2, y2;
  cr->get_clip_extents( x1, y1, x2, y2 );
  int ix1=x1, iy1=y1, ix2=x2, iy2=y2;
  VipsRect draw_area = { ix1, iy1, ix2-ix1+1, iy2-iy1+1 };
  int draw_area_right = draw_area.left + draw_area.width - 1;
  int draw_area_bottom = draw_area.top + draw_area.height - 1;

  // Immediately draw the buffered image, to avoid flickering
  // If the requested area is fully contained witin the current preview buffer,
  // we do not submit any further redraw request
  double_buffer.lock();
  //std::cout<<"  draw_area: "<<draw_area<<std::endl;
  //std::cout<<"  buffer_area: "<<double_buffer.get_active().get_rect()<<std::endl;
  if( double_buffer.get_active().get_rect().width > 0 &&
      double_buffer.get_active().get_rect().height > 0 ) {
    Glib::RefPtr< Gdk::Pixbuf > pixbuf = modify_preview();
    Gdk::Cairo::set_source_pixbuf( cr, pixbuf,
        double_buffer.get_active().get_rect().left,
        double_buffer.get_active().get_rect().top );
    cr->paint();
  }

  bool repaint_needed = true;
  if( vips_rect_includesrect(&(double_buffer.get_active().get_rect()), &draw_area) )
    repaint_needed = false;
  if( double_buffer.get_active().is_dirty() )
    repaint_needed = true;
  double_buffer.unlock();
  //std::cout<<"  repaint_needed="<<repaint_needed<<std::endl;
  if( !repaint_needed )
    return true;

  // Rectangle corresponding to the preview area
  VipsRect preview_area = {
      static_cast<int>(hadj->get_value()), static_cast<int>(vadj->get_value()),
      static_cast<int>(hadj->get_page_size()), static_cast<int>(vadj->get_page_size())
  };
  int preview_area_right = preview_area.left + preview_area.width - 1;
  int preview_area_bottom = preview_area.top + preview_area.height - 1;

  // Total area to be allocated for the preview buffer
  // This might be larger than the preview area, because gtk might
  // ask for pixels that are outside of it (for example when scrolling)
  VipsRect area_tot;
  area_tot.left = MIN( draw_area.left, preview_area.left );
  area_tot.top = MIN( draw_area.top, preview_area.top );
  area_tot.width = MAX( draw_area_right, preview_area_right ) - area_tot.left + 1;
  area_tot.height = MAX( draw_area_bottom, preview_area_bottom ) - area_tot.top + 1;
	//std::cout<<"ImageArea::on_draw(): area_tot="<<area_tot<<std::endl;

  /* OBSOLETE
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
  */

  xoffset = 0;
  yoffset = 0;

	// Submit the re-computation of the requested area
  ProcessRequestInfo request;
  request.sink = this;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_START;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_start request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_draw(): redraw_start request submitted."<<std::endl;

#ifdef OPTIMIZE_SCROLLING
  area_tot.left = ix1;
	area_tot.top = iy1;
	area_tot.width = ix2+1-ix1;
	area_tot.height = iy2+1-iy1;
  std::cout<<"drawing rectangle "<<ix2-ix1+1<<","<<iy2-iy1+1<<"+"<<ix1<<"+"<<iy1<<std::endl;
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
  //submit_area( draw_area );
  submit_area( area_tot );
  //std::cout<<"  submitted redraw area: "<<draw_area<<std::endl;
#endif


  request.sink = this;
  //request.area = draw_area;
  request.area = area_tot;
  request.request = PF::IMAGE_REDRAW_END;
  //std::cout<<"PF::ImageArea::on_expose_event(): submitting redraw_end request."<<std::endl;
  PF::ImageProcessor::Instance().submit_request( request );
  //std::cout<<"PF::ImageArea::on_draw(): redraw_end request submitted."<<std::endl;
#ifdef DEBUG_DISPLAY    
  std::cout<<std::endl;
#endif
  
  return true;
}
#endif



static VipsImage* convert_raw_data( VipsImage* raw )
{
  if( !raw ) return NULL;

  // The image to be displayed has two channels, we assume in this case that it represents RAW data
  // and we only show the first channel (the second channel contains the color information)
  VipsImage* band = NULL;
  if( vips_extract_band( raw, &band, 0, NULL ) ) {
    std::cout<<"ImageArea::update(): vips_extract_band() failed."<<std::endl;
    return NULL;
  }
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::convert_raw_data(): vips_extract_band() done."<<std::endl;
#endif

  VipsInterpretation interpretation = VIPS_INTERPRETATION_MULTIBAND;
  vips_image_init_fields( band,
      raw->Xsize, raw->Ysize,
      1, raw->BandFmt,
      raw->Coding,
      interpretation,//raw->Type,
      1.0, 1.0);

  VipsImage* norm = NULL;
  double par1 = 1.0f/65535.0f;
  double par2 = 0;
  if( vips_linear( band, &norm, &par1, &par2, 1, NULL ) ) {
    PF_UNREF( band, "ImageArea::update(): band unref after vips_linear() failure" );
    std::cout<<"ImageArea::update(): vips_linear() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( band, "ImageArea::update(): band unref after vips_linear()" );
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::convert_raw_data(): vips_linear() done."<<std::endl;
#endif

  VipsImage* gamma = NULL;
  float exp = 2.2;
  if( vips_gamma( norm, &gamma, "exponent", exp, NULL ) ) {
    PF_UNREF( norm, "ImageArea::update(): norm unref after vips_gamma() failure" );
    std::cout<<"ImageArea::update(): vips_gamma() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( norm, "ImageArea::update(): norm unref after vips_gamma()" );
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::convert_raw_data(): vips_gamma() done."<<std::endl;
#endif

  VipsImage* cast = gamma;
  /*
if( vips_cast_ushort( gamma, &cast, NULL ) ) {
  PF_UNREF( gamma, "ImageArea::update(): gamma unref after vips_cast_ushort() failure" );
  std::cout<<"ImageArea::update(): vips_cast_ushort() failed."<<std::endl;
  return NULL;
}
PF_UNREF( gamma, "ImageArea::update(): gamma unref after vips_cast_ushort()" );
   */

  VipsImage* bandv[3] = {cast, cast, cast};
  VipsImage* out = NULL;
  if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
    PF_UNREF( cast, "ImageArea::update(): cast unref after bandjoin failure" );
    std::cout<<"ImageArea::update(): vips_bandjoin() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( cast, "ImageArea::update(): cast unref after bandjoin" );
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::convert_raw_data(): vips_bandjoin() done."<<std::endl;
#endif

  VipsImage* out2;
  VipsCoding coding = VIPS_CODING_NONE;
  interpretation = VIPS_INTERPRETATION_RGB;
  vips_copy( out, &out2,
       "coding", coding,
       "interpretation", interpretation,
       NULL );
  g_object_unref( out );

  return out2;
}



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
  bool do_merged = display_merged && (!display_mask);
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::update(): do_merged="<<do_merged<<"  display_merged="<<display_merged<<"  displayed_layer="<<displayed_layer<<"  display_mask="<<display_mask<<std::endl;
#endif
  if( !do_merged ) {
    if( displayed_layer < 0 && (!display_mask) ) do_merged = true;
    else {
      int layer_id = -1;
      if( display_mask ) {
        layer_id = selected_layer;
      } else {
        layer_id = displayed_layer;
      }
      PF::PipelineNode* node = get_pipeline()->get_node( layer_id );
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): layer_id="<<layer_id<<"  node="<<node<<std::endl;
#endif
      if( !node ) do_merged = true;
      if( get_pipeline()->get_image() ) {
        PF::Layer* temp_layer = get_pipeline()->get_image()->get_layer_manager().get_layer( layer_id );
        if( !temp_layer ) do_merged = true;
        if( !(temp_layer->is_visible()) ) do_merged = true;
      }
    }
  }
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::update(): do_merged(2)="<<do_merged<<std::endl;
#endif
  if( do_merged ) {
    image = get_pipeline()->get_output();
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::update(): image="<<image<<std::endl;
    std::cout<<"                     image->Bands="<<image->Bands<<std::endl;
    std::cout<<"                     image->BandFmt="<<image->BandFmt<<std::endl;
    std::cout<<"                     image size: "<<image->Xsize<<"x"<<image->Ysize<<std::endl;
#endif
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "ImageArea::update(): merged image ref" );
    } else {
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): calling convert_raw_data()"<<std::endl;
#endif
      image = convert_raw_data( image );
    }
  } else {
    int layer_id = -1;
    if( display_mask ) {
      layer_id = selected_layer;
    } else {
      layer_id = displayed_layer;
    }
    PF::PipelineNode* node = get_pipeline()->get_node( layer_id );
    //PF::PipelineNode* node = get_pipeline()->get_node( displayed_layer );
    if( !node ) return;
    if( !(node->blended) ) return;

    if( node->processor &&
				node->processor->get_par() &&
				!(display_mask) ) {
      image = node->blended;
      std::cout<<"ImageArea::update(): displaying layer "<<layer_id<<std::endl;
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): node->image("<<node->image<<")->Xsize="<<node->image->Xsize
               <<"    node->image->Ysize="<<node->image->Ysize<<std::endl;    
      std::cout<<"ImageArea::update(): node->blended("<<node->blended<<")->Xsize="<<node->blended->Xsize
               <<"    node->blended->Ysize="<<image->Ysize<<std::endl;    
#endif
    } else {
      // We need to find the first non-mask layer that contains the active mask layer
      /*
      PF::Layer* container_layer = NULL;
      int temp_id = displayed_layer;
      while( !container_layer ) {
				container_layer = 
          get_pipeline()->get_image()->get_layer_manager().
          get_container_layer( temp_id );
        if( !container_layer ) return;
        if( !container_layer->get_processor() ) return;
        if( !container_layer->get_processor()->get_par() ) return;
        if( container_layer->get_processor()->get_par()->is_map() == false ) break;
        temp_id = container_layer->get_id();
        container_layer = NULL;
      }

      PF::PipelineNode* container_node = 
				get_pipeline()->get_node( container_layer->get_id() );
				*/
      PF::Layer* ll = get_pipeline()->get_image()->get_layer_manager().
          get_layer( selected_layer );
      if( !ll ) return;
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): displaying mask of layer \""<<ll->get_name()<<"\""<<std::endl;
#endif
      PF::PipelineNode* selected_node =
        get_pipeline()->get_node( selected_layer );
      if( !selected_node ) return;
      if( ll->get_processor()->get_par()->needs_input() ) {
        if( selected_node->input_id < 0 ) return;

        PF::Layer* input_layer = 
          get_pipeline()->get_image()->get_layer_manager().
          get_layer( selected_node->input_id );
        if( !input_layer ) return;
        
        PF::PipelineNode* input_node = 
          get_pipeline()->get_node( input_layer->get_id() );
        if( !input_node ) return;
        if( !(input_node->image) ) return;
        
        image = input_node->image;
      } else {
        if( !selected_node->image ) return;

        image = selected_node->image;
      }

      /*
      */
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): image("<<image<<")->Xsize="<<image->Xsize<<"    image->Ysize="<<image->Ysize<<std::endl;    
#endif
    }
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "ImageArea::update(): active image ref" );
    } else {
      image = convert_raw_data( image );
    }
  }
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::update(): image="<<image<<std::endl;
#endif
  if( !image ) return;

  unsigned int level = get_pipeline()->get_level();
  //outimg = image;

  //VIPS_UNREF( region ); 
  //VIPS_UNREF( display_image ); 
  PF_UNREF( outimg, "ImageArea::update() outimg unref" );
  PF_UNREF( region, "ImageArea::update() region unref" );
  PF_UNREF( display_image, "ImageArea::update() display_image unref" );

  std::vector<VipsImage*> in;
  /**/
  ClippingWarningPar* clipping_warning_par = dynamic_cast<ClippingWarningPar*>( clipping_warning->get_par() );
  if( !clipping_warning_par ) return;
  clipping_warning_par->set_highlights_warning( highlights_warning_enabled );
  clipping_warning_par->set_shadows_warning( shadows_warning_enabled );
  clipping_warning_par->set_image_hints( image );
  clipping_warning_par->set_format( image->BandFmt );
  in.clear(); in.push_back( image );
  VipsImage* wclipimg = clipping_warning->get_par()->build(in, 0, NULL, NULL, level );
  PF_UNREF( image, "ImageArea::update() image unref after clipping warning" );
  /**/

  std::cout<<"ImageArea::update(): embedded profile:"<<std::endl;
  print_embedded_profile( wclipimg );

  // Display profile management
  cmsHPROFILE icc_display_profile;
  PF::Options& options = PF::PhotoFlow::Instance().get_options();
  if( options.get_display_profile_type() != current_display_profile_type ||
      options.get_custom_display_profile_name().c_str() != current_display_profile_name ) {

    current_display_profile_type = options.get_display_profile_type();
    current_display_profile_name = options.get_custom_display_profile_name().c_str();

    //std::cout<<"ImageArea::update(): current_display_profile_type="<<current_display_profile_type<<std::endl;
    switch( current_display_profile_type ) {
    case PF_DISPLAY_PROF_sRGB: {
      current_display_profile = PF::ICCStore::Instance().get_profile( PF::PROF_TYPE_sRGB, PF::PF_TRC_STANDARD );
      icc_display_profile = current_display_profile->get_profile();
      char tstr[1024];
      cmsGetProfileInfoASCII(icc_display_profile, cmsInfoDescription, "en", "US", tstr, 1024);
  #ifndef NDEBUG
      std::cout<<"ImageArea::update(): current_display_profile: "<<tstr<<std::endl;
  #endif
      break;
    }
    case PF_DISPLAY_PROF_CUSTOM:
      current_display_profile = PF::ICCStore::Instance().get_profile( options.get_custom_display_profile_name() );
      icc_display_profile = current_display_profile->get_profile();
      //std::cout<<"ImageArea::update(): opening display profile from disk: "<<options.get_custom_display_profile_name()
      //    <<" -> "<<current_display_profile<<std::endl;
      break;
    case PF_DISPLAY_PROF_SYSTEM: {
#ifdef __APPLE__
      current_display_profile = PF::ICCStore::Instance().get_system_monitor_profile();
      if( !current_display_profile ) {
        std::cout<<"ImageArea::update(): system display profile not set, reverting to sRGB"<<std::endl;
        current_display_profile = PF::ICCStore::Instance().get_profile( PF::PROF_TYPE_sRGB, PF::PF_TRC_STANDARD );
      }
      icc_display_profile = current_display_profile->get_profile();
      char tstr[1024];
      cmsGetProfileInfoASCII(icc_display_profile, cmsInfoDescription, "en", "US", tstr, 1024);
  #ifndef NDEBUG
      std::cout<<"ImageArea::update(): current_display_profile: "<<tstr<<std::endl;
  #endif
#endif
      break;
    }
    default: break;
    }
  }

  std::cout<<"ImageArea::update(): softproof_enabled="<<softproof_enabled<<std::endl;
  if( softproof_enabled ) {
    PF::ConvertColorspacePar* cc_par =
        dynamic_cast<PF::ConvertColorspacePar*>( softproof_conversion->get_par() );
    if( cc_par ) {
      cc_par->set_gamut_warning( gamut_warning_enabled );
      cc_par->set_clip_negative( softproof_clip_negative_enabled );
      cc_par->set_clip_overflow( softproof_clip_overflow_enabled );
      cc_par->set_bpc(softproof_bpc_enabled);
    }
    softproof_conversion->get_par()->set_image_hints( wclipimg );
    softproof_conversion->get_par()->set_format( get_pipeline()->get_format() );
    in.clear(); in.push_back( wclipimg );
    VipsImage* tmpimg = softproof_conversion->get_par()->build(in, 0, NULL, NULL, level );
    PF_UNREF( wclipimg, "ImageArea::update() wclipimg unref after softproof_conversion" );
    wclipimg = tmpimg;
  }

  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2display->get_par() );
  //std::cout<<"ImageArea::update(): icc_par="<<icc_par<<std::endl;
  if( icc_par && current_display_profile ) {
    //std::cout<<"ImageArea::update(): setting display profile: PhF profile="<<current_display_profile
    //    <<"  ICC profile="<<icc_display_profile<<std::endl;
    icc_par->set_out_profile( current_display_profile );
  }
  icc_par->set_intent( options.get_display_profile_intent() );
  icc_par->set_bpc( options.get_display_profile_bpc() );
  icc_par->set_adaptation_state( 1.0 );
  //icc_par->set_bpc( true );
  if( softproof_enabled && sim_paper_color_enabled ) {
    icc_par->set_intent( INTENT_ABSOLUTE_COLORIMETRIC );
    icc_par->set_bpc( false );
    icc_par->set_adaptation_state( adaptation_state );
  } else if( softproof_enabled && sim_black_ink_enabled ) {
    icc_par->set_intent( INTENT_RELATIVE_COLORIMETRIC );
    icc_par->set_bpc( false );
    //icc_par->set_adaptation_state( adaptation_state );
  }
  convert2display->get_par()->set_image_hints( wclipimg );
  convert2display->get_par()->set_format( get_pipeline()->get_format() );
  in.clear(); in.push_back( wclipimg );
  VipsImage* srgbimg = convert2display->get_par()->build(in, 0, NULL, NULL, level );
  PF_UNREF( wclipimg, "ImageArea::update() wclipimg unref" );
  // "image" is managed by photoflow, therefore it is not necessary to unref it
  // after the call to convert2display: the additional reference is owned by
  // "srgbimg" and will be removed when "srgbimg" is deleted.
  // This works also in the case when "image" and "srgbimg" are the same object,
  // as the additional reference will be removed when calling 
  // g_object_unref(srgbimg) later on

#ifndef NDEBUG
  PF_PRINT_REF( srgbimg, "ImageArea::update(): srgbimg after convert2display: " );
  std::cout<<"ImageArea::update(): image="<<image<<"   ref_count="<<G_OBJECT( image )->ref_count<<std::endl;
#endif
  //outimg = srgbimg;

  if( /*!display_merged &&*/ display_mask && (selected_layer>=0) ) {
    PF::Layer* l = get_pipeline()->get_image()->get_layer_manager().get_layer( selected_layer );
    if( !l ) return;
    std::list<PF::Layer*> mask_layers = l->get_omap_layers();
    PF::Layer* first_visible = NULL;
    for( std::list<PF::Layer*>::reverse_iterator li = mask_layers.rbegin();
        li != mask_layers.rend(); li++ ) {
      if( (*li) == NULL || !(*li)->is_visible() ) continue;
      first_visible = *li;
      break;
    }
    if( first_visible ) {
      PF::Layer* ml = first_visible;
      if( !ml ) return;
      PF::PipelineNode* node = get_pipeline()->get_node( ml->get_id() );
      if( !node ) return;
      if( !(node->blended) ) return;

#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): node->blended("<<node->blended<<")->Xsize="<<node->blended->Xsize
          <<"    node->blended->Ysize="<<node->blended->Ysize<<std::endl;
#endif
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

      PF::set_icc_profile( redimage, current_display_profile );

      PF::BlenderPar* maskblend_par = dynamic_cast<PF::BlenderPar*>( maskblend->get_par() );
      if( maskblend_par ) {
        maskblend_par->set_image_hints( srgbimg );
        maskblend_par->set_format( get_pipeline()->get_format() );
        maskblend_par->set_blend_mode( PF::PF_BLEND_NORMAL );
        maskblend_par->set_opacity( 0.8 );
      }
      in.clear(); 
      in.push_back( srgbimg );
      in.push_back( redimage );
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): srgbimg->Xsize="<<srgbimg->Xsize<<"    srgbimg->Ysize="<<srgbimg->Ysize<<std::endl;    
      std::cout<<"ImageArea::update(): redimage->Xsize="<<redimage->Xsize<<"    redimage->Ysize="<<redimage->Ysize<<std::endl;    
#endif      
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): building maskblend operation..."<<std::endl;
#endif
      VipsImage* blendimage = maskblend->get_par()->build(in, 0, NULL, mapinverted, level );
#ifdef DEBUG_DISPLAY
      std::cout<<"ImageArea::update(): ... maskblend done"<<std::endl;
#endif
      PF_UNREF( srgbimg, "ImageArea::update() srgbimg unref" );
      PF_UNREF( mapinverted, "ImageArea::update() mapinverted unref" );
      PF_UNREF( redimage, "ImageArea::update() redimage unref" );

      srgbimg = blendimage;
    }
  }

/*
  in.clear();
  in.push_back( srgbimg );
  convert_format->get_par()->set_image_hints( srgbimg );
  convert_format->get_par()->set_format( VIPS_FORMAT_UCHAR );
  outimg = convert_format->get_par()->build( in, 0, NULL, NULL, level );
  //g_object_unref( srgbimg );
  PF_UNREF( srgbimg, "ImageArea::update() srgbimg unref" );
*/
  outimg = srgbimg;
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
//    if( vips_affine( outimg, &outimg2,
//                     shrink_factor, 0, 0, shrink_factor, NULL ) )
//      return;
//    if( vips_reduce( outimg, &outimg2, 1.0/shrink_factor, 1.0/shrink_factor,
//        "kernel", VIPS_KERNEL_CUBIC, NULL) ) {
//      std::cout<<std::endl<<std::endl<<"VIPS_REDUCE FAILED!!!!!!!"<<std::endl<<std::endl<<std::endl;
//      return;
//    }
#ifdef DEBUG_DISPLAY
		std::cout<<"ImageArea::update(): before vips_resize()"<<std::endl;
#endif
		if( vips_resize( outimg, &outimg2, shrink_factor, NULL) ) {
      std::cout<<std::endl<<std::endl<<"vips_resize() FAILED!!!!!!!"<<std::endl<<std::endl<<std::endl;
      return;
    }
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::update(): before vips_resize(), outimg: "<<outimg<<"  outimg2: "<<outimg2<<std::endl;
#endif
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

	int tile_size = DISPLAY_TILE_SIZE;
	// make room for at most a 4k display
	int max_csreen_width = 128*30;
	int max_screen_height = 128*20;
  if (vips_sink_screen2 (outimg, display_image, NULL,
												 tile_size, tile_size, (max_csreen_width/tile_size)*(max_screen_height/tile_size),
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

  double_buffer.lock();
  double_buffer.get_active().set_dirty( true );
  double_buffer.get_inactive().set_dirty( true );
  double_buffer.unlock();

  int area_left = hadj->get_value();
  int area_top = vadj->get_value();
  int area_width = hadj->get_page_size();
  int area_height = vadj->get_page_size();
  int image_width, image_height;
  get_size_request( image_width, image_height );
  float area_center_x = 0;
  float area_center_y = 0;
#ifdef DEBUG_DISPLAY
  std::cout<<"#0 area_width="<<area_width<<"  image_width="<<image_width<<std::endl;
  std::cout<<"   area_height="<<area_height<<"  image_height="<<image_height<<std::endl;
#endif
  if( target_area_center_x < 0 || target_area_center_x < 0 ) {
    if( image_width>1 && image_height>1 ) {
      area_center_x = area_left + area_width/2;
      area_center_y = area_top + area_height/2;
#ifdef DEBUG_DISPLAY
      std::cout<<"#1 area_center_x="<<area_center_x<<"  area_center_y="<<area_center_y<<std::endl;
#endif
      area_center_x /= image_width;
      area_center_y /= image_height;
#ifdef DEBUG_DISPLAY
      std::cout<<"#2 area_center_x="<<area_center_x<<"  area_center_y="<<area_center_y<<std::endl;
#endif
    }
  } else {
    area_center_x = target_area_center_x;
    area_center_y = target_area_center_y;
    target_area_center_x = target_area_center_y = -1;
  }

  area_center_x *= outimg->Xsize;
  area_center_y *= outimg->Ysize;
#ifdef DEBUG_DISPLAY
  std::cout<<"#3 area_center_x="<<area_center_x<<"  area_center_y="<<area_center_y<<std::endl;
#endif
  area_left = area_center_x - area_width/2;
  if( area_left < 0 ) area_left = 0;
  area_top = area_center_y - area_height/2;
  if( area_top < 0 ) area_top = 0;
#ifdef DEBUG_DISPLAY
  std::cout<<"#4 area_left="<<area_left<<"  area_top="<<area_top<<std::endl;
#endif

  //Update * update = g_new (Update, 1);
  //update->image_area = this;
  //update->rect.left = area_left;
  //update->rect.top = area_top;
  //update->rect.width = outimg->Xsize;
  //update->rect.height = outimg->Ysize;

  g_mutex_lock(preview_size_mutex);
  preview_size.left = area_left;
  preview_size.top = area_top;
  preview_size.width = outimg->Xsize;
  preview_size.height = outimg->Ysize;
  g_mutex_unlock(preview_size_mutex);
#ifdef DEBUG_DISPLAY
  std::cout<<"   update->rect: "<<update->rect<<std::endl;
  std::cout<<"PF::ImageArea::update(): installing set_size callback."<<std::endl;
#endif
  //gdk_threads_add_idle ((GSourceFunc) set_size_cb, update);
  signal_set_size.emit();
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): set_size() called"<<std::endl;
#endif

  /*
  update = g_new (Update, 1);
  update->image_area = this;
  update->rect.width = update->rect.height = 0;
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): installing queue_draw callback."<<std::endl;
#endif
  //gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
#ifdef DEBUG_DISPLAY
  std::cout<<"PF::ImageArea::update(): queue_draw() called"<<std::endl;
#endif
  */
}



void PF::ImageArea::sink( const VipsRect& area ) 
{
  return;
#ifdef DEBUG_DISPLAY
  std::cout<<"ImageArea::sink( const VipsRect& area ) called"<<std::endl;
    std::cout<<"area="<<area.left<<","<<area.top<<"+"<<area.width<<"+"<<area.height<<std::endl;
#endif

  PF::Pipeline* pipeline = get_pipeline();
  if( !pipeline ) return;
  if( !outimg ) return;
  unsigned int level = pipeline->get_level();
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
	//vips_region_invalidate( region2 );

  VipsRect* parea = (VipsRect*)(&scaled_area);
  VipsRect iarea = {0, 0, display_image->Xsize, display_image->Ysize};

  vips_rect_intersectrect (&iarea, parea, parea);
  if( (parea->width <= 0) ||
      (parea->height <= 0) )
    return;

  vips_invalidate_area( display_image, &scaled_area );
#ifdef DEBUG_DISPLAY
  std::cout<<"Preparing area "<<scaled_area.left<<","<<scaled_area.top<<"+"<<scaled_area.width<<"+"<<scaled_area.height<<std::endl;
#endif
  if (vips_region_prepare (region2, parea)) {
    std::cout<<"ImageArea::sink(): vips_region_prepare() failed."<<std::endl;
    return;
  }
	/*
  unsigned char* pout = (unsigned char*)VIPS_REGION_ADDR( region2, parea->left, parea->top ); 
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
#ifdef DEBUG_DISPLAY
  std::cout<<"Region "<<parea->width<<","<<parea->height<<"+"<<parea->left<<"+"<<parea->top<<" copied into active buffer"<<std::endl;
#endif
  //std::cout<<"ImageArea::sink(): draw_requested="<<draw_requested<<std::endl;
  if( !draw_requested ) {
    Update * update = g_new (Update, 1);
    update->image_area = this;
    update->rect.width = update->rect.height = 0;
    //std::cout<<"ImageArea::sink(): installing idle callback."<<std::endl;
    draw_requested = true;
    gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
  }
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

