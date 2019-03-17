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

#ifndef CONVERT_COLORSPACE_CONFIG_DIALOG_HH
#define CONVERT_COLORSPACE_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../operation_config_gui.hh"
#include "../../operations/raw_developer.hh"


namespace PF {

  class ConvertColorspaceConfigGUI: public OperationConfigGUI
  {
    Gtk::VBox outputControlsBox;
    
    Selector outProfileModeSelector;
    Gtk::HBox outProfileModeSelectorBox;

    Selector outProfileTypeSelector;
    Gtk::HBox outProfileTypeSelectorBox;

    Selector outTRCTypeSelector;
    Gtk::HBox outTRCTypeSelectorBox;

    Selector intentSelector;
    Gtk::HBox intentSelectorBox;

    CheckBox clip_negative_checkbox;
    CheckBox clip_overflow_checkbox;
    Gtk::HBox clip_negative_box;
    Gtk::HBox clip_overflow_box;

    CheckBox gamut_mapping_checkbox;
    Gtk::HBox gamut_mapping_box;

    Slider saturation_intent_slider;
    Gtk::HBox saturation_intent_box;

    CheckBox bpcButton;
    Gtk::HBox bpcButtonBox;

    Slider adaptationStateSlider;
    Gtk::HBox adaptationStateBox;

    CheckBox assignButton;
    Gtk::HBox assignButtonBox;

    Gtk::HBox outProfHBox;
    Gtk::VBox outProfVBox;
    Gtk::Label outProfLabel;
    Gtk::Entry outProfFileEntry;
    Gtk::Button outProfOpenButton;

    Gtk::CheckButton gamutWarningButton;
    Gtk::Label gamutWarningLabel;
    Gtk::HBox gamutWarningButtonBox;

    
  public:
    ConvertColorspaceConfigGUI( Layer* l );
    
    void open();
    void do_update();

    void on_out_button_open_clicked();
    void on_out_filename_changed();

    void on_gamut_warning_toggled();
  };

}

#endif
