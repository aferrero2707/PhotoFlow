/* This code is directly derived from the gtkdisp2.cc program included in the 
 * VIPS distribution; credits go therefore to the VIPS authors.
 *
 * 8-bit RGB images only, though it would be easy to fix this.
 *
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

#ifndef IMAGE_AREA_HH
#define IMAGE_AREA_HH

#include <stdio.h>
#include <iostream>

#include <gtkmm.h>

#include <vips/vips>

#include "../base/photoflow.hh"
#include "../base/image.hh"

#include "../operations/vips_operation.hh"
#include "../operations/image_reader.hh"
#include "../operations/brightness_contrast.hh"
#include "../operations/invert.hh"
#include "../operations/gradient.hh"
#include "../operations/convert2rgb.hh"

#include "../gui/operations/brightness_contrast_config.hh"
#include "../gui/operations/imageread_config.hh"
#include "../gui/operations/vips_operation_config.hh"

using namespace vips;

/* We need the C API as well for the region stuff.
 */
#include <vips/vips.h>

static int
image_posteval_test( VipsImage *image, VipsProgress *progress )
{
  /* Spaces at end help to erase the %complete message we overwrite.
   */
  printf( "%s %s: done in %.3gs          \n" , 
	  g_get_prgname(), image->filename, 
	  g_timer_elapsed( progress->start, NULL ) );

  return( 0 );
}


/* Subclass DrawingArea to make a widget that displays a VImage.
 */
class ImageArea : public Gtk::DrawingArea
{
private:
  /* The image we display.
   */
  VipsImage* image;

  /* The derived image we paint to the screen.
   */
  VipsImage* display_image;

  /* The region we prepare from to draw the pixels,
   */
  VipsRegion* region;

  /* The cache mask. 
   */
  VipsImage* mask;

  /* Read from the cache mask with this. 
   */
  VipsRegion* mask_region;

  PF::ProcessorBase* convert2srgb;

  //PF::LayerManager* layer_manager;
  //PF::Image* pf_image;
  PF::View* view;

  /* We send this packet of data from the bg worker thread to the main GUI
   * thread when a tile has been calculated.
   */
  typedef struct {
    ImageArea * image_area;
    VipsRect rect;
  } Update;

  /* The main GUI thread runs this when it's idle and there are tiles that need
   * painting. 
   */
  static gboolean render_cb (Update * update)
  {
    update->image_area->queue_draw_area (update->rect.left, 
                                         update->rect.top,
                                         update->rect.width,
                                         update->rect.height);

    g_free (update);

    return FALSE;
  }

  /* Come here from the vips_sink_screen() background thread when a tile has 
   * been calculated. 
   *
   * We can't paint the screen directly since the main GUI thread might be 
   * doing something. Instead, we add an idle callback which will be
   * run by the main GUI thread when it next hits the mainloop.
   */
  static void
  sink_notify (VipsImage *image, VipsRect *rect, void *client)
  {
    ImageArea * image_area = (ImageArea *) client;
    Update * update = g_new (Update, 1);

    update->rect = *rect;
    update->image_area = image_area;

    g_idle_add ((GSourceFunc) render_cb, update);
  }

#ifdef GTKMM_2
  void
  expose_rect (GdkRectangle * expose)
  {
    /* Clip against the image size ... we don't want to try painting outside the
     * image area.
     */
    VipsRect image = {0, 0, display_image->Xsize, display_image->Ysize};
    VipsRect area = {expose->x, expose->y, expose->width, expose->height};
    VipsRect clip;

    vips_rect_intersectrect (&image, &area, &clip);
    if (vips_rect_isempty (&clip))
      return;

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
      guchar *p = (guchar *) VIPS_REGION_ADDR( region, clip.left, clip.top );
      int lsk = VIPS_REGION_LSKIP( region );

      get_window()->draw_rgb_image( get_style()->get_white_gc(),
             clip.left, clip.top, clip.width, clip.height,
             Gdk::RGB_DITHER_MAX, p, lsk);
    }
  }

  virtual bool on_expose_event (GdkEventExpose * event)
  {
    GdkRectangle *expose;
    int i, n;

    //std::cout<<"ImageArea::on_expose_event() called."<<std::endl;

    gdk_region_get_rectangles (event->region, &expose, &n);
    for (i = 0; i < n; i++)
      expose_rect (&expose[i]);
    g_free (expose);

    return TRUE;
  }
#endif


#ifdef GTKMM_3
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    cairo_rectangle_list_t *list =  cairo_copy_clip_rectangle_list (cr->cobj());
    for (int i = list->num_rectangles - 1; i >= 0; --i) {
      cairo_rectangle_t *rect = &list->rectangles[i];
      
      std::cout<<"ImageArea::on_draw(): rectangle = "<<rect->x<<","<<rect->y
	       <<" -> "<<rect->width<<","<<rect->height<<std::endl;
    }
    std::cout<<std::endl;
    
    // Draw the image in the middle of the drawing area, or (if the image is
    // larger than the drawing area) draw the middle part of the image.
    //Gdk::Cairo::set_source_pixbuf(cr, m_image,
    //  (width - m_image->get_width())/2, (height - m_image->get_height())/2);
    cr->paint();
    
    return true;
  }
#endif


  virtual void on_realize() 
  {
    Gtk::DrawingArea::on_realize();

    get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
    set_double_buffered( FALSE );

    Gtk::Allocation allocation = get_allocation();
    std::cout<<"DrawingArea size: "<<allocation.get_width()<<","
	     <<allocation.get_height()<<std::endl;

  }

public:
  ImageArea ()
  {
    image = NULL;
    view = NULL;
    display_image = NULL;
    region = NULL;
    mask = NULL;
    mask_region = NULL;
    convert2srgb = new PF::Processor<PF::Convert2RGBPar,PF::Convert2RGBProc>();
  }

  virtual ~ ImageArea ()
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

  //PF::LayerManager* get_layer_manager() { return layer_manager; }
  //PF::Image* get_image() { return pf_image; }

  void set_view( PF::View* v ) { view = v; }

  bool on_resize(GdkEventConfigure* event) 
  {
    Gtk::Allocation allocation = get_allocation();
    std::cout<<"on_resize(): DrawingArea size =    "<<allocation.get_width()<<","
	     <<allocation.get_height()<<std::endl;
    std::cout<<"             ScrolledWindow size = "<<get_parent()->get_allocation().get_width()<<","
	     <<get_parent()->get_allocation().get_height()<<std::endl;

    update_image();

    return false;
  }

  void update_image() {
    //PF::View* view = pf_image->get_view(0);

    if( !view || !view->get_output() ) return;

    VipsImage* outimg;
    image = view->get_output();
    outimg = image;

    VIPS_UNREF( display_image ); 
    VIPS_UNREF( mask ); 

    /**/
    std::vector<VipsImage*> in; in.push_back( image );
    VipsImage* srgbimg = convert2srgb->get_par()->build(in, 0, NULL, NULL );
    //g_object_unref( image );
    outimg = srgbimg;
    /**/

    /*
    VipsImage* srgbimg2;
    vips_cast( srgbimg, &srgbimg2, VIPS_FORMAT_UCHAR, NULL );
    g_object_unref( srgbimg );

    outimg = srgbimg2;

    */
    

    display_image = im_open( "display_image", "p" );


    mask = vips_image_new();

    /**/
    if (vips_sink_screen (outimg, display_image, mask,
        64, 64, (2000/64)*(2000/64), 0, sink_notify, this))
      verror ();
    /*
    if (vips_sink_screen (srgbimg, display_image, mask,
			  64, 64, (2000/64)*(2000/64), 0, NULL, this))
      verror ();
    */
    g_object_unref( outimg );
  
    VIPS_UNREF( region ); 
    region = vips_region_new (display_image);
    std::cout<<"Image size: "<<display_image->Xsize<<","
	     <<display_image->Ysize<<std::endl;

    VIPS_UNREF( mask_region ); 
    mask_region = vips_region_new( mask );

    set_size_request (display_image->Xsize, display_image->Ysize);
    queue_draw();
  }
};


#endif
