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

#ifndef CROP_CONFIG_DIALOG_HH
#define CROP_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/crop.hh"


namespace PF {

  enum crop_handle_t {
    CROP_HANDLE_NONE,
    CROP_HANDLE_TOPLEFT,
    CROP_HANDLE_TOPRIGHT,
    CROP_HANDLE_BOTTOMLEFT,
    CROP_HANDLE_BOTTOMRIGHT,
    CROP_HANDLE_LEFT,
    CROP_HANDLE_RIGHT,
    CROP_HANDLE_TOP,
    CROP_HANDLE_BOTTOM,
    CROP_HANDLE_CENTER
  };

  class CropConfigGUI: public OperationConfigGUI
{
  crop_handle_t handle;

  Gtk::VBox controlsBox;
  Gtk::HBox arControlsBox;

  Slider cropLeftSlider;
  Slider cropTopSlider;
  Slider cropWidthSlider;
  Slider cropHeightSlider;
  CheckBox keepARCheckBox;
  Slider cropARWidthSlider;
  Slider cropARHeightSlider;

  int crop_center_dx, crop_center_dy;

  void move_handle( int x, int y );
  void move_left_handle( int x, int y );
  void move_top_handle( int x, int y );
  void move_topleft_handle( int x, int y );
  void move_bottomleft_handle( int x, int y );
  void move_topright_handle( int x, int y );

public:
  CropConfigGUI( Layer* l );

  bool has_preview() { return true; }

  void open();
  bool has_editing_mode() { return true; }

  bool pointer_press_event( int button, double x, double y, int mod_key );
  bool pointer_release_event( int button, double x, double y, int mod_key );
  bool pointer_motion_event( int button, double x, double y, int mod_key );

  virtual bool modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                               float scale, int xoffset, int yoffset );
};

}

#endif
