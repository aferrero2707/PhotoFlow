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

#include <version.hh>

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

  notebook.append_page( color_box, _("Color management") );
  notebook.append_page( about_box, _("About") );

  cm_display_profile_model = Gtk::ListStore::create(cm_display_profile_columns);
  cm_display_profile_type_selector.set_model( cm_display_profile_model );
  cm_display_profile_type_selector.pack_start(cm_display_profile_columns.col_value);

  Gtk::TreeModel::iterator ri = cm_display_profile_model->append();
  Gtk::TreeModel::Row row = *(ri);
  row[cm_display_profile_columns.col_id] = 0;
  row[cm_display_profile_columns.col_value] = "sRGB";

  ri = cm_display_profile_model->append();
  row = *(ri);
  row[cm_display_profile_columns.col_id] = 1;
#ifdef __APPLE__
  row[cm_display_profile_columns.col_value] = _("System");
#else
  row[cm_display_profile_columns.col_value] = _("System (not working)");
#endif

  ri = cm_display_profile_model->append();
  row = *(ri);
  row[cm_display_profile_columns.col_id] = 2;
  row[cm_display_profile_columns.col_value] = _("Custom");

  //cm_display_profile_button.add( cm_display_profile_img );
  cm_display_profile_open_label.set_text( _("custom display profile name: ") );
  cm_display_profile_box.pack_start( cm_display_profile_open_label, Gtk::PACK_SHRINK );
  cm_display_profile_box.pack_start( cm_display_profile_entry, Gtk::PACK_SHRINK, 4 );
  cm_display_profile_box.pack_start( cm_display_profile_open_button, Gtk::PACK_SHRINK );

  cm_display_profile_type_selector.set_active( 0 );

  cm_display_profile_type_label.set_text( _("display profile type: ") );
  cm_display_profile_type_box.pack_start( cm_display_profile_type_label, Gtk::PACK_SHRINK );
  cm_display_profile_type_box.pack_start( cm_display_profile_type_selector, Gtk::PACK_SHRINK, 4 );

  color_box.pack_start( cm_display_profile_type_box, Gtk::PACK_SHRINK, 4 );
  color_box.pack_start( cm_display_profile_box, Gtk::PACK_SHRINK, 4 );

  //cm_display_profile_type_selector.set_size_request( 30, -1 );

  Glib::ustring about = PF::version_string;
  Glib::RefPtr< Gtk::TextBuffer > buf = about_textview.get_buffer ();
  buf->set_text( about );
  about_textview.set_wrap_mode(Gtk::WRAP_WORD);
  about_textview.set_left_margin( 5 );
  about_textview.set_right_margin( 5 );
  about_textview.set_editable( false );
  about_textview.set_cursor_visible( false );

  about_box.pack_start( about_textview, Gtk::PACK_EXPAND_WIDGET );

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
  cm_display_profile_type_selector.set_active( 0 );

  Gtk::TreeModel::Children children = cm_display_profile_model->children();
  Gtk::TreeModel::Children::iterator iter;
  int row_id = 0;
  for( iter = children.begin(), row_id = 0; iter != children.end(); iter++, row_id++ ) {
    Gtk::TreeModel::Row row = *iter;
    if( row && (row[cm_display_profile_columns.col_id] == (int)PF::PhotoFlow::Instance().get_options().get_display_profile_type()) ) {
      cm_display_profile_type_selector.set_active( row_id );
      break;
    }
  }

  //cm_display_profile_type_selector.set_active( PF::PhotoFlow::Instance().get_options().get_display_profile_type() );
  cm_display_profile_entry.set_text( PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() );
}


void PF::SettingsDialog::save_settings()
{
  bool cm_dpy_modified = false;

  int dpy_prof_id = (int)PF::PhotoFlow::Instance().get_options().get_display_profile_type();

  Gtk::TreeModel::iterator iter = cm_display_profile_type_selector.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      dpy_prof_id = row[cm_display_profile_columns.col_id];
    }
  }

  if( dpy_prof_id != (int)PF::PhotoFlow::Instance().get_options().get_display_profile_type() ) {
    cm_dpy_modified = true;
  }
  if( cm_display_profile_entry.get_text() != PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() ) {
    cm_dpy_modified = true;
  }
  if( cm_dpy_modified ) signal_cm_modified.emit();

  //std::cout<<"cm_display_profile_type_selector.get_active_row_number(): "<<cm_display_profile_type_selector.get_active_row_number()<<std::endl;
  PF::PhotoFlow::Instance().get_options().set_display_profile_type( dpy_prof_id );
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
  Gtk::FileChooserDialog dialog(_("Please choose an ICC profile"),
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
