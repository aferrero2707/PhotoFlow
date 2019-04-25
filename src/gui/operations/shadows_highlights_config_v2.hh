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

#ifndef SHADOWS_HIGHLIGHTS_CONFIG_DIALOG_V2_HH
#define SHADOWS_HIGHLIGHTS_CONFIG_DIALOG_V2_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"


namespace PF {

  class ShadowsHighlightsConfigV2GUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    Gtk::VBox controlsBox2;
    Gtk::HBox equalizerBox;
    Gtk::HBox globalBox;
    Gtk::Alignment padding1, padding2, padding3;
    Gtk::Frame shadows_frame, highlights_frame, blur_frame;
    Gtk::VBox shadows_box, highlights_box, blur_box;

    Slider amount_slider;
    Slider strength_s_slider, range_s_slider, strength_h_slider, range_h_slider;
    Slider anchor_slider;
    Slider median_smooth_gain_slider, median_smooth_exponent_slider;
    CheckBox fast_approx_box;
    CheckBox do_median_box;
    CheckBox do_median_smoothing_box;
    CheckBox do_guided_box;
    CheckBox show_residual_box;

    Gtk::VBox guidedControlsBox;
    Slider guidedRadiusSlider, guidedThresholdSlider;

  public:
    ShadowsHighlightsConfigV2GUI( Layer* l );
    
    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
