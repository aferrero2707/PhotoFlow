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

  Gtk::Image img_open;

  Gtk::Label label;
  Gtk::Entry fileEntry;
  Gtk::Button openButton;
  //#endif

  Gtk::Alignment spacing1, spacing2;
  Gtk::VBox outputControlsBox;

  Selector inProfileModeSelector;
  Gtk::HBox inProfileModeSelectorBox;
  Selector inProfileTypeSelector;
  Gtk::HBox inProfileTypeSelectorBox;
  Selector inTRCModeSelector;
  Gtk::HBox inTRCModeSelectorBox;

  Gtk::HBox inProfHBox;
  Gtk::VBox inProfVBox;
  Gtk::Label inProfLabel;
  Gtk::Entry inProfFileEntry;
  Gtk::Button inProfOpenButton;

  Selector outProfileModeSelector;
  Gtk::HBox outProfileModeSelectorBox;
  Selector outProfileTypeSelector;
  Gtk::HBox outProfileTypeSelectorBox;
  Selector outTRCModeSelector;
  Gtk::HBox outTRCModeSelectorBox;

  Gtk::HBox outProfHBox;
  Gtk::VBox outProfVBox;
  Gtk::Label outProfLabel;
  Gtk::Entry outProfFileEntry;
  Gtk::Button outProfOpenButton;

  Gtk::VBox inProfBox, outProfBox;
  Gtk::Frame inProfFrame, outProfFrame;

public:
  ImageReadConfigGUI( Layer* l );

  void on_button_open_clicked();
  void on_in_button_open_clicked();
  void on_out_button_open_clicked();

  void on_filename_changed();
  void on_in_filename_changed();
  void on_out_filename_changed();

  void open();
  void do_update();
};

}

#endif
