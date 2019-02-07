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

  float exposure;

  tone_mapping_method2_t method;
  float gamma, gamma_pivot, exponent;
  float A, B, C, D, E, F, W;
  float f2midgraylock, f2gamma, f2exponent, TS, TL, SS, SL, SA;
  float AL_Lmax, AL_b, AL_Tsize_par, AL_Tlength_par, AL_Tstrength_par,
  AL_Tsize, AL_Tlength, AL_Texp,
  AL_Trange, AL_Tshift, AL_Tmax, AL_Tvshift;
  float LP_slope, LP_linmax, LP_compr, LP_Kstrength, LP_Kmax, LP_Ssmooth;
  float LE_slope, LE_linmax, LE_compr, LE_Kstrength, LE_Kmax, LE_Sslope, LE_Sslope2;
  float HD_lin_slope, HD_lin_pivot, HD_SR, HD_TR, HD_lin_Dmin, HD_lin_Dmax;

  FilmicToneCurve::FullCurve filmic2_curve;

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

    ExposureSlider exposureSlider;
    Selector modeSelector;

    Gtk::Frame log_frame;
    Gtk::VBox gammaControlsBox;
    Slider sh_compr_slider;
    Slider hl_compr_slider;
    Slider log_pivot_slider;

    CheckBox gamut_compression_checkbox;
    CheckBox hue_protection_checkbox;

    Gtk::VBox filmicControlsBox;
    Slider filmic_A_slider;
    Slider filmic_B_slider;
    Slider filmic_C_slider;
    Slider filmic_D_slider;
    Slider filmic_E_slider;
    Slider filmic_F_slider;
    Slider filmic_W_slider;

    Gtk::VBox filmic2ControlsBox;
    CheckBox filmic2_preserve_midgray_checkbox;
    Slider filmic2_gamma_slider;
    Slider filmic2_TL_slider;
    Slider filmic2_TS_slider;
    Slider filmic2_SL_slider;
    Slider filmic2_SS_slider;
    Slider filmic2_SA_slider;

    Gtk::VBox ALControlsBox;
    Slider AL_Lmax_slider;
    Slider AL_b_slider;
    Slider AL_Tsize, AL_Tlength, AL_Tstrength;

    Gtk::VBox LPControlsBox;
    Slider LP_compression;
    Slider LP_slope;
    Slider LP_lin_max;
    Slider LP_knee_strength;
    Slider LP_shoulder_smoothness;

    Gtk::Frame LE_frame;
    Gtk::VBox LEControlsBox;
    Slider LE_compression;
    Slider LE_slope;
    Slider LE_lin_max;
    Slider LE_knee_strength;
    Slider LE_shoulder_slope;
    Slider LE_shoulder_slope2;

    Gtk::VBox HDControlsBox;
    Slider HD_slope;
    Slider HD_shoulder_range;

    Slider gamut_compression_slider;
    Slider gamut_compression_exponent_slider;
    Slider lumi_blend_frac_slider;

  public:
    ToneMappingConfigGUI_V2( Layer* l );
    
    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
