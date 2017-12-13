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

#ifndef PATH_MASK_CONFIG_DIALOG_HH
#define PATH_MASK_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/path_mask.hh"

#include "../widgets/selector.hh"
#include "../widgets/curveeditor.hh"


namespace PF {

  class PathMaskConfigGUI: public OperationConfigGUI
  {
    Gtk::HBox hbox;
    CheckBox invert_box, enable_falloff_box;

    Gtk::HBox curvesBox;

    CurveEditor falloffCurveEditor;

    int active_point_id;
    bool center_selected;
    double cxlast, cylast;

    bool path_resizing;
    double path_resizing_last_point_x, path_resizing_last_point_y;


    bool border_resizing;
    double border_resizing_path_point_x, border_resizing_path_point_y;
    int border_resizing_lstart;
    float border_resizing_size_start;

    bool initializing;

    void draw_outline( PixelBuffer& buf_in, PixelBuffer& buf_out, ClosedSplineCurve& path );

  public:
    PathMaskConfigGUI( Layer* l );
    void do_update();
    bool has_editing_mode() { return true; }

    void parameters_reset();

    bool pointer_press_event( int button, double x, double y, int mod_key );
    bool pointer_release_event( int button, double x, double y, int mod_key );
    bool pointer_motion_event( int button, double x, double y, int mod_key );
    bool modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out,
        float scale, int xoffset, int yoffset );
  };

}

#endif
