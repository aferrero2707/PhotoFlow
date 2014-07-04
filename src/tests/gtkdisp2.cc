/* Tiny display-an-image demo program. 
 *
 * This is not supposed to be a complete image viewer, it's just supposed to 
 * show how to display a VIPS image (or the result of a VIPS computation) in a
 * window.
 *
 * 8-bit RGB images only, though it would be easy to fix this.
 *
 * Compile with:

	g++ -g -Wall gtkdisp2.cc \
		`pkg-config vipsCC-7.26 gtkmm-2.4 --cflags --libs` 

 */

#include <stdio.h>

#include <gtkmm.h>

#include <vips/vips>

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
  VImage image;

  /* The derived image we paint to the screen.
   */
  VImage display_image;

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
  static const gboolean edge_detect = FALSE;

  /* Make the image for display from the raw disc image. Could do
   * anything here, really. Uncomment sections to try different effects. 
   * Convert to 8-bit RGB would be a good idea.
   */
  void
  build_display_image ()
  {
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

    /* vips_sink_screen() is not wrapped by C++ ... we need to drop down to C
     * here.
     *
     * We ask for a cache of 1000 64x64 tiles, enough to be able to repaint a
     * 1920 x 1200 screen, plus a bit.
     */
    if (vips_sink_screen (t.image (), display_image.image (), NULL,
			  64, 64, 1000, 0, sink_notify, this))
      verror ();

    /* display_image depends on t .. we need to keep t alive as long
     * as display_image is alive.
     */
    display_image._ref->addref (t._ref);
  }

  void
  expose_rect (GdkRectangle * expose)
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

    get_window ()->draw_rgb_image (get_style ()->get_white_gc (),
				   clip.left, clip.top, clip.width, clip.height,
				   Gdk::RGB_DITHER_MAX, buf, lsk);
  }

  virtual bool on_expose_event (GdkEventExpose * event)
  {
    GdkRectangle *expose;
    int i, n;

    gdk_region_get_rectangles (event->region, &expose, &n);
    for (i = 0; i < n; i++)
      expose_rect (&expose[i]);
    g_free (expose);

    return TRUE;
  }

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

  void
  set_image (VImage new_image)
  {
    image = new_image;

    /* Reset the display image.
     */
    VImage null;
    display_image = null;
    if (region)
      {
	g_object_unref (region);
	region = NULL;
      }

    build_display_image ();
    region = vips_region_new (display_image.image ());
    set_size_request (display_image.Xsize (), display_image.Ysize ());
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

  VImage image(argv[1]);

  Gtk::Window window;
  window.set_default_size (500, 500);
  Gtk::ScrolledWindow scrolled_window;
  window.add (scrolled_window);
  ImageArea area;
  area.set_image (image);
  scrolled_window.add (area);
  window.show_all ();

  Gtk::Main::run (window);

  return 0;
}
