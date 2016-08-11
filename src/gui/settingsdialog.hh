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
  //Tree model columns:
  class DCMModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    DCMModelColumns()
    { add(col_id); add(col_value); }

    Gtk::TreeModelColumn<int> col_id;
    Gtk::TreeModelColumn<Glib::ustring> col_value;
  };

  DCMModelColumns cm_working_profile_columns;
  Glib::RefPtr<Gtk::ListStore> cm_working_profile_model;
  DCMModelColumns cm_working_trc_columns;
  Glib::RefPtr<Gtk::ListStore> cm_working_trc_model;

  DCMModelColumns cm_display_profile_columns;
  Glib::RefPtr<Gtk::ListStore> cm_display_profile_model;

  DCMModelColumns cm_display_profile_intent_columns;
  Glib::RefPtr<Gtk::ListStore> cm_display_profile_intent_model;

  Gtk::Notebook notebook;

  Gtk::VBox about_box, color_box;

  Gtk::ComboBox cm_working_profile_type_selector;
  Gtk::Image cm_working_profile_open_img;
  Gtk::Button cm_working_profile_open_button;
  Gtk::Entry cm_working_profile_entry;
  Gtk::HBox cm_working_profile_box;
  Gtk::HBox cm_working_profile_box2;
  Gtk::ComboBox cm_working_trc_type_selector;

  Gtk::ComboBox cm_display_profile_type_selector;
  Gtk::Image cm_display_profile_open_img;
  Gtk::Button cm_display_profile_open_button;
  Gtk::Entry cm_display_profile_entry;
  Gtk::ComboBox cm_display_profile_intent_selector;
  Gtk::CheckButton cm_display_profile_bpc_selector;
  Gtk::HBox cm_display_profile_box;

  Gtk::Frame cm_working_profile_frame, cm_display_profile_frame;
  Gtk::VBox cm_working_profile_frame_box, cm_display_profile_frame_box;

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
