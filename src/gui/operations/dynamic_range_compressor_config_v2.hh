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

#ifndef DYNAMIC_RANGE_COMPRESSOR_CONFIG_DIALOG_V2_HH
#define DYNAMIC_RANGE_COMPRESSOR_CONFIG_DIALOG_V2_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/volume.hh"
#include "../widgets/vslider.hh"


namespace PF {

  class DynamicRangeCompressorConfigV2GUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    Gtk::VBox controlsBox2;
    Gtk::HBox equalizerBox;
    Gtk::HBox globalBox;
    Gtk::HBox equalizerCheckboxBox;
    Gtk::Alignment equalizerCheckboxPadding;
    Gtk::Alignment equalizerPadding;
    Gtk::HSeparator separator;
    
    Selector modeSelector;

    Slider amount_slider;
    Slider strength_s_slider, strength_h_slider, anchor_slider;
    Slider local_contrast_slider;
    CheckBox show_residual_box;
    CheckBox enable_equalizer_box;
    VSlider blacks_amount_slider;
    VSlider shadows_amount_slider;
    VSlider midtones_amount_slider;
    VSlider highlights_amount_slider;
    VSlider whites_amount_slider;

    Gtk::VBox usmControlsBox;
    Slider usmRadiusSlider;

    Gtk::VBox bilateralControlsBox;
    Slider bilateralIterationsSlider, bilateralSigmasSlider,
      bilateralSigmarSlider;

    Gtk::VBox guidedControlsBox;
    Slider guidedRadiusSlider, guidedThresholdSlider;

    Slider lumiBlendSlider;

  public:
    DynamicRangeCompressorConfigV2GUI( Layer* l );
    
    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
