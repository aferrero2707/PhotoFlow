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

#ifndef HSL_MASK_CONFIG_DIALOG_HH
#define HSL_MASK_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../widgets/curveeditor.hh"

#include "../widgets/layerlist.hh"


namespace PF {

  class HSLMaskConfigGUI: public OperationConfigGUI
{
  Gtk::VBox controlsBox;

  Gtk::Frame frame;

  Gtk::HSeparator hline;

  Gtk::Alignment padding1, padding2, padding3, padding4;

  Gtk::HSeparator sep1, sep2, sep3, sep4;

  Gtk::VBox hueHeq_box, hueSeq_box, hueLeq_box;
  CurveEditor hueHeq, hueSeq, hueLeq;
  CheckBox hueHeq_enable, hueSeq_enable, hueLeq_enable;
  Gtk::HBox hueHeq_enable_box, hueSeq_enable_box, hueLeq_enable_box;
  Gtk::Alignment hueHeq_enable_padding, hueSeq_enable_padding, hueLeq_enable_padding;

  Gtk::VBox adjustment_box[3];
  Gtk::Notebook adjustments_nb, curves_nb[3];

  LayerList layer_list;


public:
  HSLMaskConfigGUI( Layer* l );

  bool has_preview() { return true; }

  void init();
  void do_update();

  bool pointer_press_event( int button, double x, double y, int mod_key );
  bool pointer_release_event( int button, double x, double y, int mod_key );
  bool pointer_motion_event( int button, double x, double y, int mod_key );
};

}

#endif
