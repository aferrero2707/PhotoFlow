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

#include <iostream>
#include <vips/vips.h>

#include "settingsdialog.hh"

/*
#include "../operations/vips_operation.hh"
#include "../operations/image_reader.hh"
#include "../operations/brightness_contrast.hh"
#include "../operations/invert.hh"
#include "../operations/gradient.hh"
#include "../operations/convert2lab.hh"
#include "../operations/clone.hh"
#include "../operations/curves.hh"

#include "../gui/operations/brightness_contrast_config.hh"
#include "../gui/operations/imageread_config.hh"
#include "../gui/operations/vips_operation_config.hh"
#include "../gui/operations/clone_config.hh"
#include "../gui/operations/curves_config.hh"
 */

PF::SettingsDialog::SettingsDialog():
      Gtk::Dialog( _("Settings"),true),
      cm_display_profile_open_button(Gtk::Stock::OPEN)
{
  set_default_size(600,400);

  add_button( _("OK"), 1 );
  add_button( _("Cancel"), 0 );

  signal_response().connect( sigc::mem_fun(*this,
      &SettingsDialog::on_button_clicked) );

  notebook.append_page( color_box, "Color management" );
  notebook.append_page( about_box, "About" );

  cm_display_profile_type_selector.insert( 0, "sRGB" );
  cm_display_profile_type_selector.insert( 1, "System (not working)" );
  cm_display_profile_type_selector.insert( 2, "Custom" );
  cm_display_profile_type_selector.set_active( 0 );
  cm_display_profile_type_selector.set_size_request( 30, -1 );

  //cm_display_profile_button.add( cm_display_profile_img );
  cm_display_profile_box.pack_start( cm_display_profile_entry, Gtk::PACK_SHRINK );
  cm_display_profile_box.pack_start( cm_display_profile_open_button, Gtk::PACK_SHRINK );

  color_box.pack_start( cm_display_profile_type_selector, Gtk::PACK_SHRINK );
  color_box.pack_start( cm_display_profile_box, Gtk::PACK_SHRINK );

  get_vbox()->pack_start( notebook );

  cm_display_profile_open_button.signal_clicked().connect(sigc::mem_fun(*this,
                &SettingsDialog::on_button_display_profile_open_clicked) );

  show_all_children();

  load_settings();
}


PF::SettingsDialog::~SettingsDialog()
{
}


void PF::SettingsDialog::open()
{
  //op_tree.update_model();
  show_all();
}


void PF::SettingsDialog::load_settings()
{
  cm_display_profile_type_selector.set_active( PF::PhotoFlow::Instance().get_options().get_display_profile_type() );
  cm_display_profile_entry.set_text( PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() );
}


void PF::SettingsDialog::save_settings()
{
  bool cm_dpy_modified = false;
  if( cm_display_profile_type_selector.get_active_row_number() != (int)PF::PhotoFlow::Instance().get_options().get_display_profile_type() ) {
    cm_dpy_modified = true;
  }
  if( cm_display_profile_entry.get_text() != PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() ) {
    cm_dpy_modified = true;
  }
  if( cm_dpy_modified ) signal_cm_modified.emit();

  std::cout<<"cm_display_profile_type_selector.get_active_row_number(): "<<cm_display_profile_type_selector.get_active_row_number()<<std::endl;
  PF::PhotoFlow::Instance().get_options().set_display_profile_type( cm_display_profile_type_selector.get_active_row_number() );
  PF::PhotoFlow::Instance().get_options().set_custom_display_profile_name( cm_display_profile_entry.get_text() );
  PF::PhotoFlow::Instance().get_options().save();
}


void PF::SettingsDialog::on_button_clicked(int id)
{
  switch(id) {
  case 0:
    //hide_all();
    hide();
    break;
  case 1:
    //hide_all();
    save_settings();
    hide();
    break;
  }
}


void PF::SettingsDialog::on_button_display_profile_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose an ICC profile",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  if( !cm_display_profile_entry.get_text().empty() ) {
    gchar* dirname = g_path_get_dirname( cm_display_profile_entry.get_text().c_str() );
    dialog.set_current_folder( dirname );
    g_free( dirname );
  }

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
    {
      std::cout << "Open clicked." << std::endl;

      //Notice that this is a std::string, not a Glib::ustring.
      std::string filename = dialog.get_filename();
      std::cout << "File selected: " <<  filename << std::endl;
      cm_display_profile_entry.set_text( filename.c_str() );
      break;
    }
  case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      break;
    }
  default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      break;
    }
  }
}
