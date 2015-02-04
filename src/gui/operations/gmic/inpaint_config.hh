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

#ifndef GMIC_INPAINT_CONFIG_DIALOG_HH
#define GMIC_INPAINT_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../operation_config_dialog.hh"


namespace PF {

  class GmicInpaintConfigDialog: public OperationConfigDialog
  {
    Gtk::Button updateButton;
    Gtk::VBox controlsBox;
    
    Slider patch_size;
    Slider lookup_size;
    Slider lookup_factor;
    Slider blend_size;
    Slider blend_threshold;
    Slider blend_decay;
    Slider blend_scales;
    Slider allow_outer_blending;
    Slider pen_size;
    Selector display_mode_selector;

  public:
    GmicInpaintConfigDialog( Layer* l );
    
    void on_update();
    void open();

    void start_stroke();
    void draw_point( double x, double y );

  bool pointer_press_event( int button, double x, double y, int mod_key );
  bool pointer_release_event( int button, double x, double y, int mod_key );
  bool pointer_motion_event( int button, double x, double y, int mod_key );
  };

}

#endif
