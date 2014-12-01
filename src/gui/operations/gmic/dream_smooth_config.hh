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

#ifndef GMIC_DREAM_SMOOTH_CONFIG_DIALOG_HH
#define GMIC_DREAM_SMOOTH_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../operation_config_dialog.hh"


namespace PF {

  class GmicDreamSmoothConfigDialog: public OperationConfigDialog
  {
    Gtk::Button updateButton;
    Gtk::VBox controlsBox;
    
    Slider iterations_slider;
    Slider prop_interations_slider;
    Slider prop_equalize_slider;
    Selector prop_merging_option_selector;
    Slider prop_opacity_slider;
    Slider prop_reverse_slider;
    Slider prop_smoothness_slider;
    Slider paddingSlider;
       
  public:
    GmicDreamSmoothConfigDialog( Layer* l );
    
    void on_update();
    void open();
  };

}

#endif
