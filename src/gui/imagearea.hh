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

#include <stdio.h>
#include <iostream>

#include <gtkmm.h>

#include <vips/vips>

#include "../base/layermanager.hh"

#include "../operations/image_reader.hh"
#include "../operations/brightness_contrast.hh"
#include "../operations/invert.hh"
#include "../operations/gradient.hh"

using namespace vips;

/* We need the C API as well for the region stuff.
 */
#include <vips/vips.h>

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

  PF::LayerManager* layer_manager;

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

  void
  expose_image( VipsRect *clip )
  {
    /* Request from the mask first to get an idea of what pixels are
     * available.
     */
    if (vips_region_prepare (mask_region, clip))
      return;
    if (vips_region_prepare (region, clip))
      return;

    /* If the mask is all zero, skip the paint.
     */
    bool found_painted;
    int x, y;

    guchar *p = (guchar *) 
      VIPS_REGION_ADDR( mask_region, clip->left, clip->top );
    int lsk = VIPS_REGION_LSKIP( mask_region );
    found_painted = false; 
    for( y = 0; y < clip->height; y++ ) {
      for( x = 0; x < clip->width; x++ ) 
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
      guchar *p = (guchar *) VIPS_REGION_ADDR( region, clip->left, clip->top );
      int lsk = VIPS_REGION_LSKIP( region );

      get_window()->draw_rgb_image( get_style()->get_white_gc(),
             clip->left, clip->top, clip->width, clip->height,
             Gdk::RGB_DITHER_MAX, p, lsk);
    }
  }

  void
  expose_background( VipsRect *clip )
  {
    get_window()->draw_rectangle( get_style()->get_black_gc(), TRUE,
             clip->left, clip->top, clip->width, clip->height ); 
  }

  virtual bool on_expose_event (GdkEventExpose * event)
  {
    GdkRectangle *expose;
    int i, n;

    //std::cout<<"ImageArea::on_expose_event() called."<<std::endl;

    VipsRect image = {0, 0, display_image->Xsize, display_image->Ysize};
    VipsRect right = {display_image->Xsize, 0, 10000, display_image->Ysize};
    VipsRect below = {0, display_image->Ysize, 10000, 10000}; 

    gdk_region_get_rectangles(event->region, &expose, &n);

    for( i = 0; i < n; i++ ) {
      VipsRect dirty = {expose[i].x, expose[i].y, 
                        expose[i].width, expose[i].height};

      VipsRect clip;

      vips_rect_intersectrect (&image, &dirty, &clip);
      if (!vips_rect_isempty (&clip))
        expose_image (&clip);

      vips_rect_intersectrect (&right, &dirty, &clip);
      if (!vips_rect_isempty (&clip))
        expose_background (&clip);

      vips_rect_intersectrect (&below, &dirty, &clip);
      if (!vips_rect_isempty (&clip))
        expose_background (&clip);
    }

    g_free (expose);

    return TRUE;
  }

  virtual void on_realize() 
  {
    Gtk::DrawingArea::on_realize();

    get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
    set_double_buffered( FALSE );
  }

public:
  ImageArea ()
  {
    image = NULL;
    display_image = NULL;
    region = NULL;
    mask = NULL;
    mask_region = NULL;
  }

  virtual ~ ImageArea ()
  {
    VIPS_UNREF( mask_region );
    VIPS_UNREF( mask );
    VIPS_UNREF( region );
    VIPS_UNREF( display_image );
    VIPS_UNREF( image );
  }

  PF::LayerManager* get_layer_manager() { return layer_manager; }

  void
  set_image (char* filename)
  {
    std::vector<VipsImage*> in;

    PF::Processor<PF::ImageReader,PF::ImageReaderPar>* imgread = 
      new PF::Processor<PF::ImageReader,PF::ImageReaderPar>();
    imgread->get_par()->set_file_name( filename );

    //PF::ProcessorBase* invert = 
    //  new PF::Processor<PF::Invert,PF::InvertPar>();

    PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>* bc = 
      new PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>();
    bc->get_par()->set_brightness(0.2);
    bc->get_par()->set_contrast(2.5);

    PF::Processor<PF::Gradient,PF::GradientPar>* gradient = 
      new PF::Processor<PF::Gradient,PF::GradientPar>();

    /**/
    //PF::LayerManager* lm 
    layer_manager = new PF::LayerManager();

    PF::Layer* limg = layer_manager->new_layer();
    limg->set_processor( imgread );
    limg->set_name( "input image" );

    PF::Layer* lbc = layer_manager->new_layer();
    lbc->set_processor( bc );
    lbc->set_name( "brightness/contrast" );

    PF::Layer* lgrad = layer_manager->new_layer();
    lgrad->set_processor( gradient );
    lgrad->set_name( "vertical gradient" );

    PF::Layer* linv1 = layer_manager->new_layer();
    linv1->set_processor( new PF::Processor<PF::Invert,PF::InvertPar>() );
    linv1->set_name( "invert 1" );

    PF::Layer* linv2 = layer_manager->new_layer();
    linv2->set_processor( new PF::Processor<PF::Invert,PF::InvertPar>() );
    linv2->set_name( "invert 2" );

    layer_manager->get_layers().push_back( limg );
    lbc->imap_insert( lgrad );
    layer_manager->get_layers().push_back( lbc );
    layer_manager->get_layers().push_back( linv1 );
    layer_manager->get_layers().push_back( linv2 );

    //layer_manager->build_chain( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );

    layer_manager->rebuild_all( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );

    update_image();
  }

  void update_image() {
    if( !layer_manager->get_output() ) 
	    return;

    VIPS_UNREF( image ); 
    image = layer_manager->get_output();
    g_object_ref( image ); 

    VIPS_UNREF( display_image ); 
    VIPS_UNREF( mask ); 

    display_image = vips_image_new();
    mask = vips_image_new();

    if (vips_sink_screen (image, display_image, mask,
        64, 64, (2000/64)*(2000/64), 0, sink_notify, this))
      verror ();

    VIPS_UNREF( region ); 
    region = vips_region_new( display_image );
    std::cout << "Image size: " << display_image->Xsize << "," <<
	    display_image->Ysize << std::endl;

    VIPS_UNREF( mask_region ); 
    mask_region = vips_region_new( mask );

    set_size_request( display_image->Xsize, display_image->Ysize );
    queue_draw();
  }
};
