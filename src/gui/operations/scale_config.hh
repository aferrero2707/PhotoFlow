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

#ifndef SCALE_CONFIG_DIALOG_HH
#define SCALE_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_dialog.hh"
#include "../../operations/scale.hh"


namespace PF {

  class ScaleConfigDialog: public OperationConfigDialog
{
  Gtk::VBox controlsBox;

  Selector scale_mode;
  Selector scale_unit;

  Gtk::HBox scale_controls_box;

  Gtk::HBox scale_pixels_box;
  Slider scale_width_pixels_slider;
  Slider scale_height_pixels_slider;

  Gtk::HBox scale_percent_box;
  Slider scale_width_percent_slider;
  Slider scale_height_percent_slider;

  Gtk::HBox scale_mm_box;
  Slider scale_width_mm_slider;
  Slider scale_height_mm_slider;

  Gtk::HBox scale_cm_box;
  Slider scale_width_cm_slider;
  Slider scale_height_cm_slider;

  Gtk::HBox scale_inches_box;
  Slider scale_width_inches_slider;
  Slider scale_height_inches_slider;

  Slider scale_resolution_slider;


public:
  ScaleConfigDialog( Layer* l );

  void do_update();
};

}

#endif
