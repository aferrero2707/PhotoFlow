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

#ifndef PF_IMAGE_INFO_HH
#define PF_IMAGE_INFO_HH

#include <stdio.h>
#include <iostream>

#include <queue>

#include <gtkmm.h>

//#include <vips/vips>

#include "../base/photoflow.hh"
#include "../base/pipeline.hh"
#include "../base/image.hh"

#include "doublebuffer.hh"



namespace PF
{

class ImageInfo : public PipelineSink, public Gtk::Frame
{

  /* The derived image we paint to the screen.
   */
  VipsImage* display_image;
  VipsImage* outimg;

  Gtk::TextView textview;

public:

  /* We send this packet of data from the bg worker thread to the main GUI
   * thread when a tile has been calculated.
   */
  typedef struct {
    ImageInfo * info;
    Glib::ustring* text;
  } Update;

  static gboolean queue_draw_cb (Update * update);

  ImageInfo( Pipeline* v );
  virtual ~ImageInfo();

  void set_text(const Glib::ustring& text);

  void update( VipsRect* area );
};

}

#endif
