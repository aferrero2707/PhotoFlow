/* 
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

#ifndef TONE_MAPPING_CONFIG_V3_DIALOG_HH
#define TONE_MAPPING_CONFIG_V3_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/tone_mapping_v3.hh"
#include "../widgets/curveeditor.hh"


namespace PF {

class ToneMappingCurveAreaV3: public Gtk::DrawingArea
{
  typedef unsigned long int* ulong_p;

  int border_size;

  bool is_linear;
  float exponent;
  float hist_min, hist_max;
  float* mem_array;
  size_t array_sz;
  unsigned long int* hist;

  VipsImage* image;
  ToneMappingParV3* opar;

#ifdef GTKMM_2
  bool on_expose_event(GdkEventExpose* event);
#endif
#ifdef GTKMM_3
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
public:
  ToneMappingCurveAreaV3();

  void set_op(PF::ToneMappingParV3* p);
  void set_image(VipsImage* i);
  void update_histogram();

  float get_curve_value( float );

  void set_display_mode( bool lin )
  {
    bool redraw = (lin != is_linear) ? true : false;
    is_linear = lin;
    if( redraw ) queue_draw();
  }

  virtual void draw_background(const Cairo::RefPtr<Cairo::Context>& cr);
};


class ToneMappingConfigGUI_V3: public OperationConfigGUI
{
  Gtk::Notebook notebook;
    Gtk::VBox controlsBox;
    Gtk::VBox controlsBox2;
    Gtk::HBox globalBox;
    Gtk::HSeparator separator;
    
    Gtk::HBox curve_area_box;
    ToneMappingCurveAreaV3 curve_area;

    CheckBox hue_protection_checkbox;
    CheckBox lc_enable_checkbox;


    Gtk::Frame LE_frame;
    Gtk::VBox LEControlsBox;
    Slider exposure_slider;
    Slider latitude_slider;
    Slider contrast_slider;
    Slider slope2_slider;
    Slider slope3_slider;
    Slider white_point_slider;

    Gtk::Frame tc_frame, contrast_frame, color_frame, curve_frame;
    Gtk::VBox tc_box, contrast_box, color_box, curve_box;

    Slider saturation_scaling_slider;
    Slider sh_desaturation_slider;
    Slider hl_desaturation_slider;
    Slider lumi_blend_frac_slider;

    Gtk::Frame local_contrast_frame;
    Gtk::VBox local_contrast_box;
    Slider local_contrast_slider;
    Slider local_contrast_radius_slider;
    Slider local_contrast_threshold_slider;

    CurveEditor lcCurveEditor;

  public:
    ToneMappingConfigGUI_V3( Layer* l );
    
    void switch_to_custom();

    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
