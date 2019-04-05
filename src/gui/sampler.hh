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

#ifndef PF_COLOR_SAMPLER_HH
#define PF_COLOR_SAMPLER_HH

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

class Sampler : public PipelineSink, public Gtk::Frame
{

  /* The derived image we paint to the screen.
   */
  VipsImage* display_image;
  VipsImage* outimg;

  int sampler_x, sampler_y, sampler_size;

  bool display_merged;
  int active_layer;
  int edited_layer;

  Gtk::HBox hbox;
  Gtk::Label label_value1;
  Gtk::Label label_value2;
  Gtk::Label label_value3;
  Gtk::Label label_value4;
  Gtk::VBox labels_box;
  Gtk::Label LCH_label_value1;
  Gtk::Label LCH_label_value2;
  Gtk::Label LCH_label_value3;
  Gtk::VBox LCH_labels_box;

  Gtk::CheckButton check;
  bool enabled;
  bool grabbed;
  int id;

  ICCTransform transform;

public:

  /* We send this packet of data from the bg worker thread to the main GUI
   * thread when a tile has been calculated.
   */
  typedef struct {
    Sampler * sampler;
    float val[4];
    float lch[3];
    VipsInterpretation type;
  } Update;

  static gboolean queue_draw_cb (Update * update);

  Sampler( Pipeline* v, Glib::ustring title, int id );
  virtual ~Sampler();

	VipsImage* get_display_image() { return display_image; }

	void set_values(float val[4], float lch[3], VipsInterpretation type);

  void update( VipsRect* area );

  void enable_changed();

  //void sink( const VipsRect& area );

  void set_displayed_layer( int id ) {
    int old_id = active_layer;
    active_layer = id; 
    std::cout<<"Sampler::set_displayed_layer(): id="<<id<<"  old_id="<<old_id<<"  display_merged="<<display_merged<<std::endl;
    if( !display_merged && (old_id != active_layer) ) {
      //update( NULL );
			if( get_pipeline() && get_pipeline()->get_image() ) {
        //std::cout<<"Sampler::set_displayed_layer(): get_pipeline()->get_image()->update() called."<<std::endl;
        get_pipeline()->set_output_layer_id( active_layer );
				//get_pipeline()->get_image()->update();
			}
		}
  }
  int get_active_layer() { return active_layer; }

  void set_display_merged( bool val )
  {
    bool old_val = display_merged;
    display_merged = val;
    std::cout<<"Sampler::set_displayed_merged(): val="<<val<<"  old_val="<<old_val<<std::endl;
    if( display_merged )
      get_pipeline()->set_output_layer_id( -1 );
    if( display_merged != old_val ) {
      //update( NULL );
			if( get_pipeline() && get_pipeline()->get_image() ) {
			  //std::cout<<"Sampler::set_displayed_merged(): get_pipeline()->get_image()->update() called."<<std::endl;
				//get_pipeline()->get_image()->update();
			}
		}
  }
  bool get_display_merged() { return display_merged; }



  virtual bool pointer_press_event( int button, double x, double y, double D, int mod_key );
  virtual bool pointer_release_event( int button, double x, double y, int mod_key );
  virtual bool pointer_motion_event( int button, double x, double y, int mod_key );

  virtual bool modify_preview( PixelBuffer& buf_out, float scale, int xoffset, int yoffset );


};


class SamplerGroup: public Gtk::Frame//ScrolledWindow
{
  Gtk::HBox row1, row2, row3, row4;
  Gtk::VBox box;
  Sampler s1, s2, s3, s4, s5, s6, s7, s8;

public:
  SamplerGroup( Pipeline* v );

  int get_sampler_num() { return 8; }
  Sampler& get_sampler(int i);
};

}

#endif
