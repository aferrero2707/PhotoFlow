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

#ifndef GMIC_SPLIT_DETAILS_CONFIG_DIALOG_HH
#define GMIC_SPLIT_DETAILS_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../operation_config_gui.hh"


namespace PF {

  class GmicSplitDetailsConfigGUI: public OperationConfigGUI
  {
    Gtk::Button updateButton;
    Gtk::VBox controlsBox;
    
    Slider prop_nscales_slider;
    Slider prop_base_scale_slider;
    Slider prop_detail_scale_slider;
       
  public:
    GmicSplitDetailsConfigGUI( Layer* l );
    
    void on_update();
    void open();
  };

}

#endif
