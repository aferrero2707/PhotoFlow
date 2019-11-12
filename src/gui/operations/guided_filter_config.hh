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

#ifndef GUIDED_FILTER_CONFIG_DIALOG_HH
#define GUIDED_FILTER_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
//#include "../../operations/guided_filter.hh"


namespace PF {

  class GuidedFilterConfigGUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    Slider threshold_slider, radius_slider, subsampling_slider;
    CheckBox perceptual_cbox;

  public:
    GuidedFilterConfigGUI( Layer* l );
    
    bool is_perceptual() { return perceptual_cbox.get_active(); }

    bool has_preview() { return true; }
  };

}

#endif
