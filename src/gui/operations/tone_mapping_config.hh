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

#ifndef TONE_MAPPING_CONFIG_DIALOG_HH
#define TONE_MAPPING_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/tone_mapping.hh"


namespace PF {

  class ToneMappingConfigGUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    Gtk::VBox controlsBox2;

    ExposureSlider exposureSlider;
    Selector modeSelector;
    Gtk::VBox gammaControlsBox;
    Slider gamma_slider;

    Gtk::VBox filmicControlsBox;
    Slider filmic_A_slider;
    Slider filmic_B_slider;
    Slider filmic_C_slider;
    Slider filmic_D_slider;
    Slider filmic_E_slider;
    Slider filmic_F_slider;
    Slider filmic_W_slider;

    Slider lumi_blend_frac_slider;

  public:
    ToneMappingConfigGUI( Layer* l );
    
    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
