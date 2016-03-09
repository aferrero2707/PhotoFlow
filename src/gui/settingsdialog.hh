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

#ifndef PF_SETTINGS_DIALOG_HH
#define PF_SETTINGS_DIALOG_HH

#include <string>

#include <gtkmm.h>

#include "../base/photoflow.hh"


namespace PF {

class SettingsDialog : public Gtk::Dialog
{
  Gtk::Notebook notebook;

  Gtk::VBox about_box, color_box;

  Gtk::ComboBoxText cm_display_profile_type_selector;
  Gtk::Image cm_display_profile_open_img;
  Gtk::Button cm_display_profile_open_button;
  Gtk::Entry cm_display_profile_entry;
  Gtk::HBox cm_display_profile_box;

public:
  sigc::signal<void> signal_cm_modified;

  SettingsDialog();
  virtual ~SettingsDialog();

  void open();

  void load_settings();
  void save_settings();

  void on_button_clicked(int id);
  void on_button_display_profile_open_clicked();
};

}

#endif // GTKMM_EXAMPLE_HELLOWORLD_H
