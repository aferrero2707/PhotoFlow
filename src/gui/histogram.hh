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

#ifndef HISTOGRAM_HH
#define HISTOGRAM_HH

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

class HistogramArea : public Gtk::DrawingArea
{
public:

  typedef unsigned long int* ulong_p;

  ulong_p hist;
  float hist_min, hist_max;

  HistogramArea(): hist(NULL) {}

#ifdef GTKMM_2
  //void expose_rect (const VipsRect& area);
  bool on_expose_event (GdkEventExpose * event);
#endif

#ifdef GTKMM_3
  //void expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr);
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
};


class Histogram : public PipelineSink, public Gtk::VBox
{

  /* The derived image we paint to the screen.
   */
  VipsImage* image;
  HistogramArea hist_area;
  size_t array_sz;
  float* mem_array;

  Gtk::HBox clip_range_hbox;
  Gtk::Label clip_range_label;
  Gtk::CheckButton clip_range_check;

  bool display_merged;
  int active_layer;
  int edited_layer;

  bool clip_range;

  Glib::Dispatcher signal_queue_draw;

public:

  /* We send this packet of data from the bg worker thread to the main GUI
   * thread when a tile has been calculated.
   */
  typedef struct {
    Histogram * histogram;
    unsigned long int* buf;
  } Update;

  typedef unsigned long int* ulong_p;

  ulong_p hist;
  float hist_min, hist_max;

  //static gboolean queue_draw_cb (Update * update);

  Histogram( Pipeline* v );
  virtual ~Histogram();

  void on_clip_range_check_toggled();

  //VipsImage* get_display_image() { return display_image; }

  void update_histogram();
  void update( VipsRect* area );

  //void sink( const VipsRect& area );

  void set_displayed_layer( int id ) {
    int old_id = active_layer;
    active_layer = id; 
    std::cout<<"Histogram::set_displayed_layer(): id="<<id<<"  old_id="<<old_id<<"  display_merged="<<display_merged<<std::endl;
    if( !display_merged && (old_id != active_layer) ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
        std::cout<<"Histogram::set_displayed_layer(): get_pipeline()->get_image()->update() called."<<std::endl;
        get_pipeline()->set_output_layer_id( old_id );
        get_pipeline()->get_image()->update();
      }
    }
  }
  int get_active_layer() { return active_layer; }

  void set_display_merged( bool val )
  {
    bool old_val = display_merged;
    display_merged = val;
    std::cout<<"Histogram::set_displayed_merged(): val="<<val<<"  old_val="<<old_val<<std::endl;
    if( display_merged )
      get_pipeline()->set_output_layer_id( -1 );
    if( display_merged != old_val ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
        std::cout<<"Histogram::set_displayed_merged(): get_pipeline()->get_image()->update() called."<<std::endl;
        get_pipeline()->get_image()->update();
      }
    }
  }
  bool get_display_merged() { return display_merged; }
};

}

#endif
