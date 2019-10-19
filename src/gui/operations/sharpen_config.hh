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

#ifndef SHARPEN_CONFIG_DIALOG_HH
#define SHARPEN_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/sharpen.hh"


namespace PF {

  class SharpenConfigGUI: public OperationConfigGUI
  {
    Gtk::VBox controlsBox;
    
    Selector modeSelector;

    Gtk::VBox usmControlsBox;
    Slider usmRadiusSlider;

    Gtk::VBox eusmControlsBox;
    Slider eusmAmountSlider;
    Slider eusmRadiusSlider;
    Slider eusmThresholdLSlider;
    Slider eusmThresholdHSlider;
    CheckBox eusmDoSumCbox;

    Gtk::VBox rlControlsBox;
    Slider rlSigmaSlider;
    Slider rlIterationsSlider;

    Gtk::VBox textureControlsBox;
    Slider textureStrengthSlider;
    Slider textureRadiusSlider;

  public:
    SharpenConfigGUI( Layer* l );
    
    bool has_preview() { return true; }

    void do_update();
  };

}

#endif
