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

#include <queue>

#include <gtkmm.h>

#include <vips/vips>

#include "../base/photoflow.hh"
#include "../base/view.hh"
#include "../base/image.hh"

#include "../operations/image_reader.hh"
#include "../operations/convert2srgb.hh"
#include "../operations/convertformat.hh"

#include "../gui/operations/imageread_config.hh"


/*
  The ImageArea performs the image update aynchronously, inside a dedicated thread.
  Upon a drawing request from the Gtk system (on_expose_event or on_draw)
  the rectangles are inserted in a waiting queue and the processing thread is waken.
  The processing thread takes each rectangle, updates the corresponding image region
  and installs an idle callback function that takes care of drawing the region on the display.
  The processing thread then waits until the region is drawn before processing the
  next rectangle.
 */

namespace PF
{

class ImageArea : public ViewSink, public Gtk::DrawingArea
{

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

  PF::ProcessorBase* convert_format;

  long int pending_pixels;

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

public:

  ImageArea( View* v );
  virtual ~ImageArea();

#ifdef GTKMM_2
  void expose_rect (const VipsRect& area);
  bool on_expose_event (GdkEventExpose * event);
#endif

#ifdef GTKMM_3
    void expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr);
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif

  void update();

  virtual void on_realize() 
  {
    Gtk::DrawingArea::on_realize();

    //get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
    //set_double_buffered( FALSE );

    Gtk::Allocation allocation = get_allocation();
#ifndef NDEBUG
    std::cout<<"DrawingArea size: "<<allocation.get_width()<<","
	     <<allocation.get_height()<<std::endl;
#endif
  }
};

}

#endif
