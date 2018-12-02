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

//#include <vips/vips>

#include "../base/options.hh"
#include "../base/photoflow.hh"
#include "../base/pipeline.hh"
#include "../base/image.hh"

#include "doublebuffer.hh"
#include "sampler.hh"

#ifndef NDEBUG
#define DEBUG_DISPLAY
#endif



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

class ImageArea : public PipelineSink, public Gtk::DrawingArea
{

#ifdef GTKMM_2
  Gtk::Adjustment* hadj;
  Gtk::Adjustment* vadj;
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::Adjustment> hadj;
  Glib::RefPtr<Gtk::Adjustment> vadj;
#endif

  /* The derived image we paint to the screen.
   */
  VipsImage* display_image;
  VipsImage* outimg;

  unsigned int xoffset, yoffset;

  /* The region we prepare from to draw the pixels,
   */
  VipsRegion* region;

  DoubleBuffer double_buffer;
  PixelBuffer temp_buffer;

  GCond* draw_done;
  GMutex* draw_mutex;

  GMutex* preview_size_mutex;
  VipsRect preview_size;
  Glib::Dispatcher signal_set_size;

  bool draw_requested;

  Glib::Dispatcher signal_queue_draw;

  /* The cache mask. 
   */
  VipsImage* mask;

  /* Read from the cache mask with this. 
   */
  VipsRegion* mask_region;

  bool softproof_enabled;
  PF::ProcessorBase* softproof_conversion;
  PF::ProcessorBase* ocio_view;
  PF::ProcessorBase* convert2display;
  display_profile_t current_display_profile_type;
  Glib::ustring current_display_profile_name;
  ICCProfile* current_display_profile;

  PF::ProcessorBase* uniform;
  PF::ProcessorBase* maskblend;
  PF::ProcessorBase* invert;
  PF::ProcessorBase* convert_format;
  PF::ProcessorBase* clipping_warning;

  bool highlights_warning_enabled, shadows_warning_enabled;
  bool softproof_bpc_enabled, sim_black_ink_enabled, sim_paper_color_enabled, gamut_warning_enabled;
  bool softproof_clip_negative_enabled, softproof_clip_overflow_enabled;
  cmsUInt32Number softproof_intent;
  float adaptation_state;

  bool display_merged;
  bool display_mask;
  int displayed_layer;
  int selected_layer;
  int edited_layer;

	float shrink_factor;

	float target_area_center_x, target_area_center_y;

  long int pending_pixels;

  SamplerGroup* samplers;

  /* We send this packet of data from the bg worker thread to the main GUI
   * thread when a tile has been calculated.
   */
  typedef struct {
    ImageArea * image_area;
    VipsRect rect;
    guchar* buf;
    int lsk;
  } Update;

  //static gboolean set_size_cb (Update * update);
  void set_size_cb ();

  static gboolean queue_draw_cb (Update * update);

public:

  ImageArea( Pipeline* v );
  virtual ~ImageArea();

  unsigned int get_xoffset() { return xoffset; }
  unsigned int get_yoffset() { return yoffset; }

	VipsImage* get_display_image() { return display_image; }

  DoubleBuffer& get_double_buffer() { return double_buffer; }

#ifdef GTKMM_2
  //void expose_rect (const VipsRect& area);
  bool on_expose_event (GdkEventExpose * event);
#endif

#ifdef GTKMM_3
  //void expose_rect (const VipsRect& area, const Cairo::RefPtr<Cairo::Context>& cr);
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
  void submit_area( const VipsRect& area );
  void process_start( const VipsRect& area );
  void process_area( const VipsRect& area );
  void process_end( const VipsRect& area );
  void draw_area();

  Glib::RefPtr< Gdk::Pixbuf > modify_preview();

  void set_highlights_warning( bool flag ) { highlights_warning_enabled = flag; }
  void set_shadows_warning( bool flag ) { shadows_warning_enabled = flag; }

  void set_softproof_bpc( bool flag ) { softproof_bpc_enabled = flag; }
  void set_sim_black_ink( bool flag ) { sim_black_ink_enabled = flag; }
  void set_sim_paper_color( bool flag ) { sim_paper_color_enabled = flag; }
  void set_clip_negative( bool flag ) { softproof_clip_negative_enabled = flag; }
  void set_clip_overflow( bool flag ) { softproof_clip_overflow_enabled = flag; }
  void set_gamut_warning( bool flag ) { gamut_warning_enabled = flag; }
  void set_softproof_intent( cmsUInt32Number i ) { softproof_intent = i; }
  void set_softproof_adaptation_state( float s ) { adaptation_state = s; }
  void enable_softproof() { softproof_enabled = true; }
  void disable_softproof() { softproof_enabled = false; }
  PF::ProcessorBase* get_softproof_conversion() { return softproof_conversion; }

	float get_shrink_factor() { return shrink_factor; }
	void set_shrink_factor( float val ) { shrink_factor = val; }
	void set_target_area_center( float x, float y ) {
	  target_area_center_x = x; target_area_center_y = y;
	}

  void set_adjustments( 
#ifdef GTKMM_2
		       Gtk::Adjustment* h, Gtk::Adjustment* v
#endif
#ifdef GTKMM_3
		       Glib::RefPtr<Gtk::Adjustment> h,
		       Glib::RefPtr<Gtk::Adjustment> v
#endif
			)
  {
    hadj = h; vadj = v;
  }

#ifdef GTKMM_2
  Gtk::Adjustment*
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::Adjustment>
#endif
  get_hadj() { return hadj; }
#ifdef GTKMM_2
  Gtk::Adjustment*
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::Adjustment>
#endif
  get_vadj() { return vadj; }

  void update( VipsRect* area );

  void sink( const VipsRect& area );

  void dispose();

  void set_samplers(SamplerGroup* s) { samplers = s; }

  void set_edited_layer( int id ) {
    int old_id = edited_layer;
    edited_layer = id;
    if( old_id != edited_layer ) {
      ////update( NULL );
      //if( get_pipeline() && get_pipeline()->get_image() )
      //  get_pipeline()->get_image()->update();
    }
  }
  int get_edited_layer() { return edited_layer; }

  void set_displayed_layer( int id ) {
    int old_id = displayed_layer;
    displayed_layer = id;
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::set_displayed_layer(): id="<<id<<"  old_id="<<old_id<<"  display_merged="<<display_merged<<std::endl;
#endif
    if( !display_merged && (old_id != displayed_layer) ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
#ifdef DEBUG_DISPLAY
        std::cout<<"ImageArea::set_displayed_layer(): get_pipeline()->get_image()->update() called."<<std::endl;
#endif
        get_pipeline()->get_image()->update();
      }
    }
  }
  int get_displayed_layer() { return displayed_layer; }

  void set_selected_layer( int id ) {
    int old_id = selected_layer;
    selected_layer = id;
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::set_selected_layer(): id="<<id<<"  old_id="<<old_id<<"  display_merged="<<display_merged<<std::endl;
#endif
    if( !display_merged && (old_id != selected_layer) ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
#ifdef DEBUG_DISPLAY
        std::cout<<"ImageArea::set_selected_layer(): get_pipeline()->get_image()->update() called."<<std::endl;
#endif
        get_pipeline()->get_image()->update();
      }
    }
  }
  int get_selected_layer() { return selected_layer; }

  void set_display_merged( bool val )
  {
    bool old_val = display_merged;
    display_merged = val;
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::set_display_merged(): val="<<val<<"  old_val="<<old_val<<std::endl;
#endif
    if( display_merged != old_val ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
#ifdef DEBUG_DISPLAY
        std::cout<<"ImageArea::set_display_merged(): get_pipeline()->get_image()->update() called."<<std::endl;
#endif
        get_pipeline()->get_image()->update();
      }
    }
  }
  bool get_display_merged() { return display_merged; }

  void set_display_mask( bool val )
  {
    bool old_val = display_mask;
    display_mask = val;
#ifdef DEBUG_DISPLAY
    std::cout<<"ImageArea::set_display_mask(): val="<<val<<"  old_val="<<old_val<<std::endl;
#endif
    if( display_mask != old_val ) {
      //update( NULL );
      if( get_pipeline() && get_pipeline()->get_image() ) {
#ifdef DEBUG_DISPLAY
        std::cout<<"ImageArea::set_display_mask(): get_pipeline()->get_image()->update() called."<<std::endl;
#endif
        get_pipeline()->get_image()->update();
      }
    }
  }
  bool get_display_mask() { return display_mask; }

  virtual void on_realize() 
  {
    Gtk::DrawingArea::on_realize();

    //get_window()->set_back_pixmap( Glib::RefPtr<Gdk::Pixmap>(), FALSE );
    //set_double_buffered( TRUE );

    Gtk::Allocation allocation = get_allocation();
#ifndef NDEBUG
    std::cout<<"DrawingArea size: "<<allocation.get_width()<<","
	     <<allocation.get_height()<<std::endl;
#endif
  }
};

}

#endif
