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

#ifndef HUE_SATURATION_CONFIG_DIALOG_HH
#define HUE_SATURATION_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_dialog.hh"
//#include "../widgets/outmode_slider.hh"
#include "../../operations/brightness_contrast.hh"


namespace PF {

  class HueSaturationConfigDialog: public OperationConfigDialog
{
  Gtk::VBox controlsBox;

  Gtk::Frame frame;

  Gtk::HSeparator hline;

  Gtk::Alignment padding1, padding2, padding3, padding4;

  Slider hueSlider, saturationSlider;

  //Gtk::HBox outputModeBox;
  //OutModeSlider outputModeSlider;

public:
  HueSaturationConfigDialog( Layer* l );

  bool has_preview() { return true; }
};

}

#endif
