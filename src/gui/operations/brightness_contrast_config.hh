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

#ifndef BRIGHTNESS_CONTRAST_CONFIG_DIALOG_HH
#define BRIGHTNESS_CONTRAST_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_dialog.hh"
#include "../widgets/outmode_slider.hh"
#include "../../operations/brightness_contrast.hh"


namespace PF {

  class BrightnessContrastConfigDialog: public OperationConfigDialog
{
  //#ifdef GTKMM_2
  Gtk::VBox controlsBox;

  Gtk::Frame frame;

  Gtk::HSeparator hline;

  //Gtk::Label lbrightness, lcontrast;
  Gtk::Alignment padding1, padding2, padding3, padding4;

  //Gtk::Adjustment contrastAdj, brightnessAdj;
  //Gtk::HScale contrastScale, brightnessScale;
  //#endif

  Slider brightnessSlider, contrastSlider;

  Gtk::HBox outputModeBox;
  OutModeSlider outputModeSlider;

public:
  BrightnessContrastConfigDialog( Layer* l );

  void on_brightness_value_changed();
  void on_contrast_value_changed();

  void open();
};

}

#endif
