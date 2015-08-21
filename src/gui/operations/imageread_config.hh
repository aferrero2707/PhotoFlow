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

#ifndef IMAGE_READ_CONFIG_DIALOG_HH
#define IMAGE_READ_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/image_reader.hh"


namespace PF {

  class ImageReadConfigGUI: public OperationConfigGUI
{
  //#ifdef GTKMM_2
  Gtk::HBox controlsBox;

  Gtk::Label label;
  Gtk::Entry fileEntry;
  Gtk::Button openButton;
  //#endif

public:
  ImageReadConfigGUI( Layer* l );

  void on_button_open_clicked();

  void on_filename_changed();

  void open();
};

}

#endif
