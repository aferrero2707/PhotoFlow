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

#ifndef PF_STATUS_INDICATOR_HH
#define PF_STATUS_INDICATOR_HH

#include <stdio.h>
#include <iostream>
#include <string>

#include <gtkmm.h>



namespace PF
{

class StatusIndicatorLed : public Gtk::DrawingArea
{
  int status;

public:

  StatusIndicatorLed(): status(0) {}

  void set_status(int s)
  {
    status = s;
    queue_draw();
  }

#ifdef GTKMM_2
  //void expose_rect (const VipsRect& area);
  bool on_expose_event (GdkEventExpose * event);
#endif

#ifdef GTKMM_3
  //void expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr);
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
};


class StatusIndicatorWidget: public Gtk::HBox
{
  Gtk::Label label;
  Gtk::Alignment led_alignment;
  StatusIndicatorLed led;

  int cur_status;
  std::string cur_label;

  GMutex* mutex;
  Glib::Dispatcher dispatcher;

public:
  StatusIndicatorWidget();
  ~StatusIndicatorWidget();

  void set_status( std::string label, int status );

  void update();
};

}

#endif
