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

#ifndef CURVES_CONFIG_DIALOG_HH
#define CURVES_CONFIG_DIALOG_HH

#include "../operation_config_gui.hh"
#include "../../operations/curves.hh"
#include "../widgets/curveeditor.hh"
#include "../widgets/outmode_slider.hh"

namespace PF {

  class CurvesConfigGUI: public OperationConfigGUI
{
  //#ifdef GTKMM_2
  Gtk::HBox curvesBox;
  Gtk::HBox selectorsBox;
  //#endif

  Gtk::ComboBoxText rgbCurveSelector, labCurveSelector, cmykCurveSelector;
  CheckBox RGB_is_linear_check;

  CurveEditor greyCurveEditor;

  CurveEditor rgbCurveEditor;
  CurveEditor RCurveEditor;
  CurveEditor GCurveEditor;
  CurveEditor BCurveEditor;

  CurveEditor LCurveEditor;
  CurveEditor aCurveEditor;
  CurveEditor bCurveEditor;

  CurveEditor CCurveEditor;
  CurveEditor MCurveEditor;
  CurveEditor YCurveEditor;
  CurveEditor KCurveEditor;

  Gtk::Alignment padding1, padding2, padding3;
  Gtk::HSeparator hline;
  Gtk::HBox outputModeBox;
  OutModeSlider outputModeSlider;

public:
  CurvesConfigGUI(Layer* layer);
  virtual ~CurvesConfigGUI();

  bool has_preview() { return true; }

  void switch_curve();

  void do_update();

  bool pointer_press_event( int button, double x, double y, int mod_key );
  bool pointer_release_event( int button, double x, double y, int mod_key );
  bool pointer_motion_event( int button, double x, double y, int mod_key );
};

}

#endif
