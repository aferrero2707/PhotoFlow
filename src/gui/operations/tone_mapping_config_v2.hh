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

#ifndef TONE_MAPPING_CONFIG_V2_DIALOG_HH
#define TONE_MAPPING_CONFIG_V2_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/tone_mapping_v2.hh"


namespace PF {

class ToneMappingCurveAreaV2: public Gtk::DrawingArea
{
  int border_size;

  bool is_linear;

  float LE_gain, LE_slope, LE_linmax, LE_compr, LE_Kstrength, LE_Kmax, LE_Sslope, LE_Sslope2;

#ifdef GTKMM_2
  bool on_expose_event(GdkEventExpose* event);
#endif
#ifdef GTKMM_3
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
#endif
public:
  ToneMappingCurveAreaV2();

  void set_params(PF::ToneMappingParV2* tmpar);

  float get_curve_value( float );

  void set_display_mode( bool lin )
  {
    bool redraw = (lin != is_linear) ? true : false;
    is_linear = lin;
    if( redraw ) queue_draw();
  }

  virtual void draw_background(const Cairo::RefPtr<Cairo::Context>& cr);
};


  class ToneMappingConfigGUI_V2: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    Gtk::VBox controlsBox2;
    Gtk::HBox globalBox;
    Gtk::HSeparator separator;
    
    Gtk::HBox curve_area_box;
    ToneMappingCurveAreaV2 curve_area;

    CheckBox hue_protection_checkbox;


    Gtk::Frame LE_frame;
    Gtk::VBox LEControlsBox;
    Selector preset_selector;
    Slider LE_gain;
    Slider LE_compression;
    Slider LE_slope;
    Slider LE_lin_max;
    Slider LE_knee_strength;
    Slider LE_shoulder_slope;
    Slider LE_shoulder_slope2;
    Slider LE_shoulder_max;

    Gtk::Frame tc_frame, curve_frame;
    Gtk::VBox tc_box, curve_box;

    Slider saturation_scaling_slider;
    Slider hl_desaturation_slider;
    Slider lumi_blend_frac_slider;

    Gtk::Frame local_contrast_frame;
    Gtk::VBox local_contrast_box;
    Slider local_contrast_slider;
    Slider local_contrast_radius_slider;
    Slider local_contrast_threshold_slider;

  public:
    ToneMappingConfigGUI_V2( Layer* l );
    
    void switch_to_custom();

    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
