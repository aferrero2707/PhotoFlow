/* Tiny display-an-image demo program. 
 * 
 * This code is directly derived from the gtkdisp2.cc program included in the 
 * VIPS distribution; credits go therefore to the VIPS authors.
 *
 * This is not supposed to be a complete image viewer, it's just supposed to 
 * show how to display a VIPS image (or the result of a VIPS computation) in a
 * window.
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

#include <string>

#include <gtkmm.h>

#include <vips/vips>

#include "../src/operations/brightness_contrast.hh"
#include "../src/operations/invert.hh"
#include "../src/operations/gradient.hh"

using namespace vips;

/* We need the C API as well for the region stuff.
 */
#include <vips/vips.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

GType vips_layer_get_type();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

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
  VipsRegion * region;

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

  /* Edit these to add or remove things from the display pipe we build.
   * These should be wired up to something in a GUI.
   */
  static const gboolean zoom_in = FALSE;
  static const gboolean zoom_out = FALSE;
  static const gboolean edge_detect = false;

  /* Make the image for display from the raw disc image. Could do
   * anything here, really. Uncomment sections to try different effects. 
   * Convert to 8-bit RGB would be a good idea.
   */
  void
  build_display_image ()
  {
    /*
    VImage t = image;

    if (zoom_out)
      t = t.subsample (4, 4);

    if (zoom_in)
      t = t.zoom (4, 4);

    if (edge_detect)
      {
	VIMask m (3, 3, 1, 0, -1, -1, -1, -1, 8, -1, -1, -1, -1);

	t = t.conv (m);
      }
    */

    /* vips_sink_screen() is not wrapped by C++ ... we need to drop down to C
     * here.
     *
     * We ask for a cache of 1000 64x64 tiles, enough to be able to repaint a
     * 1920 x 1200 screen, plus a bit.
     */
    /*
    if (vips_sink_screen (t.image (), display_image.image (), NULL,
			  64, 64, 1000, 0, sink_notify, this))
      verror ();
    */
    /* display_image depends on t .. we need to keep t alive as long
     * as display_image is alive.
     */
    //display_image._ref->addref (t._ref);
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

    /* Calculate pixels. If this area is not in cache, we will see black
     * pixels, a background thread will start calculating stuff, and we will
     * get a notify callback from the bg thread when our pixels area ready. If
     * the area is in cache, we see pixels immediately.
     *
     * If we took the trouble, we could use the mask image to see what parts
     * of the resulting region were from cache and what parts were
     * uncalculated.
     */
    if (vips_region_prepare (region, &clip))
      return;
    guchar *buf = (guchar *) VIPS_REGION_ADDR (region, clip.left, clip.top);
    int lsk = VIPS_REGION_LSKIP (region);

    get_window ()->draw_rgb_image (get_style ()->get_white_gc (),
				   clip.left, clip.top, clip.width, clip.height,
				   Gdk::RGB_DITHER_MAX, buf, lsk);
  }

  virtual bool on_expose_event (GdkEventExpose * event)
  {
    GdkRectangle *expose;
    int i, n;

    gdk_region_get_rectangles (event->region, &expose, &n);
    for (i = 0; i < n; i++) {
      expose_rect (&expose[i]);
    }
    g_free (expose);

    return TRUE;
  }
#endif


#ifdef GTKMM_3
  void
  expose_rect (const Cairo::RefPtr<Cairo::Context>& cr,
	       //	       GdkRectangle * expose)
	       cairo_rectangle_t * expose)
  {
    /* Clip against the image size ... we don't want to try painting outside the
     * image area.
     */
    VipsRect image = {0, 0, display_image.Xsize (), display_image.Ysize ()};
    VipsRect area = {expose->x, expose->y, expose->width, expose->height};
    VipsRect clip;

    vips_rect_intersectrect (&image, &area, &clip);
    if (vips_rect_isempty (&clip))
      return;

    /* Calculate pixels. If this area is not in cache, we will see black
     * pixels, a background thread will start calculating stuff, and we will
     * get a notify callback from the bg thread when our pixels area ready. If
     * the area is in cache, we see pixels immediately.
     *
     * If we took the trouble, we could use the mask image to see what parts
     * of the resulting region were from cache and what parts were
     * uncalculated.
     */
    if (vips_region_prepare (region, &clip))
      return;
    guchar *buf = (guchar *) VIPS_REGION_ADDR (region, clip.left, clip.top);
    int lsk = VIPS_REGION_LSKIP (region);

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
      Gdk::Pixbuf::create_from_data(buf,Gdk::COLORSPACE_RGB,false,
				    8,clip.width,clip.height,lsk);
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf, clip.left, clip.top);
    cr->rectangle(clip.left, clip.top, clip.width, clip.height);

    cr->fill();
  }

  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    /*
    GdkRectangle rect;
    if(!gdk_cairo_get_clip_rectangle(cr->cobj(),&rect)) return false;
    expose_rect (cr, &rect);
    //cr->paint();
    return true;
    */

    cairo_rectangle_list_t *list =  cairo_copy_clip_rectangle_list (cr->cobj());
    //for (int i = 0; i < list->num_rectangles; ++i) {
    for (int i = list->num_rectangles-1; i>=0; --i) {
      cairo_rectangle_t *rect = &list->rectangles[i];
      
      expose_rect (cr, rect);
    }
    cairo_rectangle_list_destroy (list);
    return true;
  }
#endif

public:
  ImageArea ()
  {
    region = NULL;
  }

  virtual ~ ImageArea ()
  {
    if (region)
      {
	g_object_unref (region);
        region = NULL;
      }
  }


  #define USE_PF

  void
  set_image (char* filename)
  {
    image = vips_image_new_from_file( filename );

    display_image = im_open( "display_image", "p" );

    PF::ProcessorBase* invert = 
      new PF::Processor<PF::Invert,PF::InvertPar>();

    PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>* bc = 
      new PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>();
    bc->get_params()->set_brightness(0.2);
    bc->get_params()->set_contrast(2.5);

    PF::Processor<PF::Gradient,PF::GradientPar>* gradient = 
      new PF::Processor<PF::Gradient,PF::GradientPar>();
    gradient->get_params()->grayscale_image( image->Xsize, image->Ysize, image->BandFmt );


    std::vector<VipsImage*> in;

    gradient->get_params()->build( in, 0, NULL, NULL );

    in.clear();
    in.push_back(image);
    bc->get_params()->build( in, 0, gradient->get_params()->get_image(), NULL );

    if (vips_sink_screen (bc->get_params()->get_image(), display_image, NULL,
			  64, 64, (2000/64)*(2000/64), 0, sink_notify, this))
      verror ();


    region = vips_region_new (display_image);
    set_size_request (display_image->Xsize, display_image->Ysize);
  }
};




int
main (int argc, char **argv)
{
  Gtk::Main kit (argc, argv);

  if (argc != 2)
    error_exit ("usage: %s <filename>", argv[0]);
  if (vips_init (argv[0]))
    verror ();

  vips_layer_get_type();

  //im_concurrency_set( 1 );

  Gtk::Window window;
  window.set_default_size (1200, 800);
  Gtk::ScrolledWindow scrolled_window;
  window.add ( scrolled_window );
  ImageArea area;
  area.set_image ( argv[1] );
  scrolled_window.add (area);
  window.show_all ();

  Gtk::Main::run (window);

  return 0;
}
