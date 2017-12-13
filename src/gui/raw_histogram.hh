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

#ifndef RAW_HISTOGRAM_HH
#define RAW_HISTOGRAM_HH

#include <stdio.h>
#include <iostream>

#include <queue>

#include <gtkmm.h>

//#include <vips/vips>

#include "../base/photoflow.hh"
#include "../base/pipeline.hh"
#include "../base/image.hh"



namespace PF
{

class RawHistogram : public Gtk::Dialog
{

  Image* image;
  Gtk::DrawingArea drawing_area;

public:

  typedef unsigned long int* ulong_p;

  ulong_p hist;

  RawHistogram( Image* image );
  virtual ~RawHistogram();

#ifdef GTKMM_2
  //void expose_rect (const VipsRect& area);
  bool on_expose_event (GdkEventExpose * event);
#endif

#ifdef GTKMM_3
  //void expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr);
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
};

}

#endif
