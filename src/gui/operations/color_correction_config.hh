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

#ifndef COLOR_CORRECTION_CONFIG_DIALOG_HH
#define COLOR_CORRECTION_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"


namespace PF {

  class ColorCorrectionConfigGUI: public OperationConfigGUI
{
  Gtk::VBox controlsBox;

  Gtk::Frame offs_frame, slope_frame, pow_frame;
  Gtk::VBox offs_box, slope_box, pow_box;
  Gtk::Button load_button, save_button;
  Gtk::HBox button_hbox;
  Gtk::Alignment offs_padding, slope_padding, pow_padding;

  Slider offs_slider;
  Slider r_offs_slider;
  Slider g_offs_slider;
  Slider b_offs_slider;
  Slider slope_slider;
  Slider r_slope_slider;
  Slider g_slope_slider;
  Slider b_slope_slider;
  Slider pow_slider;
  Slider r_pow_slider;
  Slider g_pow_slider;
  Slider b_pow_slider;
  Slider saturation_slider;
  CheckBox is_log;

public:
  ColorCorrectionConfigGUI( Layer* l );

  bool has_preview() { return false; }

  void on_button_load();
  void on_button_save();
  void load_preset(std::string filename);
  void save_preset(std::string filename);
};

}

#endif
