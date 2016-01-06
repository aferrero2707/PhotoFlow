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

#ifndef GRADIENT_CONFIG_DIALOG_HH
#define GRADIENT_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/gradient.hh"

#include "../widgets/selector.hh"
#include "../widgets/curveeditor.hh"


namespace PF {

  class GradientConfigGUI: public OperationConfigGUI
  {
    Gtk::HBox hbox;
    Selector typeSelector;
    CheckBox invert_box;
    CheckBox perceptual_box;
    Slider center_x, center_y;

    Gtk::HBox curvesBox;
    Gtk::HBox selectorsBox;

   Gtk::ComboBoxText rgbCurveSelector, labCurveSelector, cmykCurveSelector;

    CurveEditor greyCurveEditor;

    CurveEditor rgbCurveEditor;
    CurveEditor RCurveEditor;
    CurveEditor GCurveEditor;
    CurveEditor BCurveEditor;

    CurveEditor LCurveEditor;
    CurveEditor aCurveEditor;
    CurveEditor bCurveEditor;

    int active_point_id;

  public:
    GradientConfigGUI( Layer* l );
    void switch_curve();
    void do_update();

    bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
    bool pointer_motion_event( int button, double x, double y, int mod_key );
    bool modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out,
        float scale, int xoffset, int yoffset );
  };

}

#endif
