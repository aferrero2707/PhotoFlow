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

#ifndef LENSFUN_CONFIG_DIALOG_HH
#define LENSFUN_CONFIG_DIALOG_HH

#include <gtkmm.h>

#if (BUNDLED_LENSFUN == 1)
#include <lensfun/lensfun.h>
#else
#include <lensfun.h>
#endif

#include "../operation_config_gui.hh"
#include "../widgets/lensfun_selector.hh"
#include "../../operations/lensfun.hh"


namespace PF {

class LensFunConfigGUI: public OperationConfigGUI
{
  Gtk::VBox controlsBox;
  Gtk::HBox hbox;

  CheckBox auto_matching_checkbox;
  CheckBox auto_crop_checkbox;
  PF::LFSelector lf_selector;
  CheckBox enable_distortion_button, enable_tca_button, enable_vignetting_button, enable_all_button;
  Glib::ustring custom_cam_maker, custom_cam_model, custom_lens_model;

public:
  LensFunConfigGUI( Layer* l );

  void do_update();
};

}

#endif
