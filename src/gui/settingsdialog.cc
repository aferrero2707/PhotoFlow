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


PF::SettingsDialog::SettingsDialog():
      Gtk::Dialog( _("Settings"),true),
      cm_working_profile_open_button(Gtk::Stock::OPEN),
      cm_display_profile_open_button(Gtk::Stock::OPEN),
      cm_working_profile_frame( _("Working RGB Colorspace") ),
      cm_display_profile_frame( _("Display Profile") ),
      cm_display_profile_bpc_selector( _("black point compensation") ),
      apply_default_preset_label(_("apply default processing profile")),
      save_sidecar_files_label(_("save sidecar files")),
      ui_layers_list_on_right_label(_("place layers list on the right"))
{
  set_default_size(600,400);

  add_button( _("OK"), 1 );
  add_button( _("Cancel"), 0 );

  signal_response().connect( sigc::mem_fun(*this,
      &SettingsDialog::on_button_clicked) );

  notebook.append_page( general_box, _("General") );
  notebook.append_page( color_box, _("Color management") );
  notebook.append_page( about_box, _("About") );


  // Working colorspace settings
  cm_working_profile_model = Gtk::ListStore::create(cm_working_profile_columns);
  cm_working_profile_type_selector.set_model( cm_working_profile_model );
  cm_working_profile_type_selector.pack_start(cm_working_profile_columns.col_value);

  Gtk::TreeModel::iterator ri = cm_working_profile_model->append();
  Gtk::TreeModel::Row row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_sRGB;
  row[cm_working_profile_columns.col_value] = "sRGB";
  ri = cm_working_profile_model->append();
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_REC2020;
  row[cm_working_profile_columns.col_value] = "Rec.2020";
  ri = cm_working_profile_model->append();
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_ADOBE;
  row[cm_working_profile_columns.col_value] = "AdobeRGB";
  ri = cm_working_profile_model->append();
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_PROPHOTO;
  row[cm_working_profile_columns.col_value] = "ProPhotoRGB";
  ri = cm_working_profile_model->append();
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_ACEScg;
  row[cm_working_profile_columns.col_value] = "ACEScg";
  ri = cm_working_profile_model->append();
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_ACES;
  row[cm_working_profile_columns.col_value] = "ACES";
  row = *(ri);
  row[cm_working_profile_columns.col_id] = PF::PROF_TYPE_XYZ;
  row[cm_working_profile_columns.col_value] = "XYZ D50";

  cm_working_trc_model = Gtk::ListStore::create(cm_working_trc_columns);
  cm_working_trc_type_selector.set_model( cm_working_trc_model );
  cm_working_trc_type_selector.pack_start(cm_working_trc_columns.col_value);

  ri = cm_working_trc_model->append();
  row = *(ri);
  row[cm_working_trc_columns.col_id] = PF::PF_TRC_LINEAR;
  row[cm_working_trc_columns.col_value] = _("linear");
  ri = cm_working_trc_model->append();
  row = *(ri);
  row[cm_working_trc_columns.col_id] = PF::PF_TRC_STANDARD;
  row[cm_working_trc_columns.col_value] = _("standard");
  //ri = cm_working_trc_model->append();
  //row = *(ri);
  //row[cm_working_trc_columns.col_id] = PF::PF_TRC_sRGB;
  //row[cm_working_trc_columns.col_value] = "sRGB";

  //cm_working_profile_type_selector.set_active( 1 );
  cm_working_profile_type_selector.set_size_request( 150, -1 );
  cm_working_trc_type_selector.set_active( 0 );
  cm_working_trc_type_selector.set_size_request( 150, -1 );

  cm_working_profile_box2.pack_start(cm_working_profile_type_selector, Gtk::PACK_SHRINK);
  cm_working_profile_box2.pack_start(cm_working_trc_type_selector, Gtk::PACK_SHRINK);
  cm_working_profile_box.pack_start( cm_working_profile_entry, Gtk::PACK_SHRINK );
  cm_working_profile_box.pack_start( cm_working_profile_open_button, Gtk::PACK_SHRINK );

  cm_working_profile_frame_box.pack_start( cm_working_profile_box2, Gtk::PACK_SHRINK, 5 );
  //cm_working_profile_frame_box.pack_start( cm_working_profile_box, Gtk::PACK_SHRINK, 5 );
  cm_working_profile_frame.add( cm_working_profile_frame_box );

  // Display profile settings
  cm_display_profile_model = Gtk::ListStore::create(cm_display_profile_columns);
  cm_display_profile_type_selector.set_model( cm_display_profile_model );
  cm_display_profile_type_selector.pack_start(cm_display_profile_columns.col_value);

  ri = cm_display_profile_model->append();
  row = *(ri);
  row[cm_display_profile_columns.col_id] = 0;
  row[cm_display_profile_columns.col_value] = "sRGB";

  ri = cm_display_profile_model->append();
  row = *(ri);
  row[cm_display_profile_columns.col_id] = 1;
/*
#ifdef __APPLE__
  row[cm_display_profile_columns.col_value] = _("System");
#else
  row[cm_display_profile_columns.col_value] = _("System (not working)");
#endif
*/
  ri = cm_display_profile_model->append();
  row = *(ri);
  row[cm_display_profile_columns.col_id] = 2;
  row[cm_display_profile_columns.col_value] = _("Custom");

  //cm_display_profile_type_selector.insert( 0, "sRGB" );
  //cm_display_profile_type_selector.insert( 1, "System (not working)" );
  //cm_display_profile_type_selector.insert( 2, "Custom" );
  //cm_display_profile_type_selector.set_active( 0 );
  cm_display_profile_type_selector.set_size_request( 30, -1 );

  //cm_display_profile_button.add( cm_display_profile_img );
  cm_display_profile_box.pack_start( cm_display_profile_entry, Gtk::PACK_SHRINK );
  cm_display_profile_box.pack_start( cm_display_profile_open_button, Gtk::PACK_SHRINK );

  cm_display_profile_frame_box.pack_start( cm_display_profile_type_selector, Gtk::PACK_SHRINK, 5 );
  cm_display_profile_frame_box.pack_start( cm_display_profile_box, Gtk::PACK_SHRINK, 5 );

  cm_display_profile_intent_model = Gtk::ListStore::create(cm_display_profile_intent_columns);
  cm_display_profile_intent_selector.set_model( cm_display_profile_intent_model );
  cm_display_profile_intent_selector.pack_start(cm_display_profile_intent_columns.col_value);

  ri = cm_display_profile_intent_model->append();
  row = *(ri);
  row[cm_display_profile_intent_columns.col_id] = 0;
  row[cm_display_profile_intent_columns.col_value] = "perceptual";

  ri = cm_display_profile_intent_model->append();
  row = *(ri);
  row[cm_display_profile_intent_columns.col_id] = 1;
  row[cm_display_profile_intent_columns.col_value] = "relative colorimetric";

  ri = cm_display_profile_intent_model->append();
  row = *(ri);
  row[cm_display_profile_intent_columns.col_id] = 2;
  row[cm_display_profile_intent_columns.col_value] = "absolute colorimetric";

  ri = cm_display_profile_intent_model->append();
  row = *(ri);
  row[cm_display_profile_intent_columns.col_id] = 3;
  row[cm_display_profile_intent_columns.col_value] = "saturation";

  cm_display_profile_intent_selector.set_active( 0 );
  cm_display_profile_intent_selector.set_size_request( 30, -1 );

  cm_display_profile_frame_box.pack_start( cm_display_profile_bpc_selector, Gtk::PACK_SHRINK, 5 );

  cm_display_profile_frame_box.pack_start( cm_display_profile_intent_selector, Gtk::PACK_SHRINK, 5 );


  cm_display_profile_frame.add( cm_display_profile_frame_box );


  color_box.pack_start( cm_working_profile_frame, Gtk::PACK_SHRINK,10 );
  color_box.pack_start( cm_display_profile_frame, Gtk::PACK_SHRINK,0 );

  apply_default_preset_hbox.pack_start( apply_default_preset_check, Gtk::PACK_SHRINK );
  apply_default_preset_hbox.pack_start( apply_default_preset_label, Gtk::PACK_SHRINK );
  apply_default_preset_label.set_tooltip_text(_("Apply default processing preset to RAW files"));
  general_box.pack_start( apply_default_preset_hbox, Gtk::PACK_SHRINK );

  save_sidecar_files_hbox.pack_start( save_sidecar_files_check, Gtk::PACK_SHRINK );
  save_sidecar_files_hbox.pack_start( save_sidecar_files_label, Gtk::PACK_SHRINK );
  save_sidecar_files_label.set_tooltip_text(_("Save sidecar files when exporting to raster formats"));
  general_box.pack_start( save_sidecar_files_hbox, Gtk::PACK_SHRINK );

  ui_layers_list_on_right_hbox.pack_start( ui_layers_list_on_right_check, Gtk::PACK_SHRINK );
  ui_layers_list_on_right_hbox.pack_start( ui_layers_list_on_right_label, Gtk::PACK_SHRINK );
  ui_layers_list_on_right_hbox.set_tooltip_text(_("Put the layers list panel to the right (restart needed)"));
  //general_box.pack_start( ui_layers_list_on_right_hbox, Gtk::PACK_SHRINK );


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
  profile_type_t ptype = PF::PhotoFlow::Instance().get_options().get_working_profile_type();
  Glib::RefPtr< Gtk::TreeModel > model = cm_working_profile_type_selector.get_model();
  Gtk::TreeModel::Children rows = model->children();
  Gtk::TreeModel::iterator ri = cm_working_profile_model->append();
  for( ri = rows.begin(); ri != rows.end(); ri++ ) {
    Gtk::TreeModel::Row row = *(ri);
    if( row[cm_working_profile_columns.col_id] == (int)(ptype) ) {
      cm_working_profile_type_selector.set_active( ri );
      break;
    }
  }

  TRC_type ttype = PF::PhotoFlow::Instance().get_options().get_working_trc_type();
  model = cm_working_trc_type_selector.get_model();
  rows = model->children();
  ri = cm_working_trc_model->append();
  for( ri = rows.begin(); ri != rows.end(); ri++ ) {
    Gtk::TreeModel::Row row = *(ri);
    if( row[cm_working_trc_columns.col_id] == (int)(ttype) ) {
      cm_working_trc_type_selector.set_active( ri );
      break;
    }
  }

  cm_display_profile_type_selector.set_active( PF::PhotoFlow::Instance().get_options().get_display_profile_type() );
  cm_display_profile_entry.set_text( PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() );

  switch( PF::PhotoFlow::Instance().get_options().get_display_profile_intent() ){
  case INTENT_PERCEPTUAL: cm_display_profile_intent_selector.set_active(0); break;
  case INTENT_RELATIVE_COLORIMETRIC: cm_display_profile_intent_selector.set_active(1); break;
  case INTENT_ABSOLUTE_COLORIMETRIC: cm_display_profile_intent_selector.set_active(2); break;
  case INTENT_SATURATION: cm_display_profile_intent_selector.set_active(3); break;
  }

  if( PF::PhotoFlow::Instance().get_options().get_display_profile_bpc() > 0 )
    cm_display_profile_bpc_selector.set_active( true );
  else
    cm_display_profile_bpc_selector.set_active( false );

  std::cout<<"PF::PhotoFlow::Instance().get_options().get_apply_default_preset(): "<<PF::PhotoFlow::Instance().get_options().get_apply_default_preset()<<std::endl;
  apply_default_preset_check.set_active( PF::PhotoFlow::Instance().get_options().get_apply_default_preset() != 0 );

  std::cout<<"PF::PhotoFlow::Instance().get_options().get_save_sidecar_files(): "<<PF::PhotoFlow::Instance().get_options().get_save_sidecar_files()<<std::endl;
  save_sidecar_files_check.set_active( PF::PhotoFlow::Instance().get_options().get_save_sidecar_files() != 0 );

  std::cout<<"PF::PhotoFlow::Instance().get_options().get_ui_layers_list_placement(): "<<
      PF::PhotoFlow::Instance().get_options().get_ui_layers_list_placement()<<std::endl;
  ui_layers_list_on_right_check.set_active( PF::PhotoFlow::Instance().get_options().get_ui_layers_list_placement() ==
      PF::PF_LAYERS_LIST_PLACEMENT_RIGHT );
}


void PF::SettingsDialog::save_settings()
{
  bool cm_dpy_modified = false;

  Gtk::TreeModel::iterator ri = cm_working_profile_type_selector.get_active();
  Gtk::TreeModel::Row row = *(ri);
  int ptype = row[cm_working_profile_columns.col_id];
  PF::PhotoFlow::Instance().get_options().set_working_profile_type( ptype );

  ri = cm_working_trc_type_selector.get_active();
  row = *(ri);
  int ttype = row[cm_working_trc_columns.col_id];
  PF::PhotoFlow::Instance().get_options().set_working_trc_type( ttype );

  if( cm_display_profile_type_selector.get_active_row_number() != (int)PF::PhotoFlow::Instance().get_options().get_display_profile_type() ) {
    cm_dpy_modified = true;
  }
  if( cm_display_profile_entry.get_text() != PF::PhotoFlow::Instance().get_options().get_custom_display_profile_name() ) {
    cm_dpy_modified = true;
  }

  std::cout<<"cm_display_profile_type_selector.get_active_row_number(): "<<cm_display_profile_type_selector.get_active_row_number()<<std::endl;
  PF::PhotoFlow::Instance().get_options().set_display_profile_type( cm_display_profile_type_selector.get_active_row_number() );
  PF::PhotoFlow::Instance().get_options().set_custom_display_profile_name( cm_display_profile_entry.get_text() );

  int new_display_profile_intent;
  switch( cm_display_profile_intent_selector.get_active_row_number() ){
  case 0: new_display_profile_intent = INTENT_PERCEPTUAL; break;
  case 1: new_display_profile_intent = INTENT_RELATIVE_COLORIMETRIC; break;
  case 2: new_display_profile_intent = INTENT_ABSOLUTE_COLORIMETRIC; break;
  case 3: new_display_profile_intent = INTENT_SATURATION; break;
  }
  if( new_display_profile_intent != PF::PhotoFlow::Instance().get_options().get_display_profile_intent() ) {
    cm_dpy_modified = true;
  }
  PF::PhotoFlow::Instance().get_options().set_display_profile_intent( new_display_profile_intent );

  if( cm_display_profile_bpc_selector.get_active() !=
      PF::PhotoFlow::Instance().get_options().get_display_profile_bpc() ) {
    cm_dpy_modified = true;
  }
  PF::PhotoFlow::Instance().get_options().set_display_profile_bpc( cm_display_profile_bpc_selector.get_active() );

  if( cm_dpy_modified ) signal_cm_modified.emit();

  if( apply_default_preset_check.get_active() )
    PF::PhotoFlow::Instance().get_options().set_apply_default_preset( 1 );
  else
    PF::PhotoFlow::Instance().get_options().set_apply_default_preset( 0 );

  if( save_sidecar_files_check.get_active() )
    PF::PhotoFlow::Instance().get_options().set_save_sidecar_files( 1 );
  else
    PF::PhotoFlow::Instance().get_options().set_save_sidecar_files( 0 );

  if( ui_layers_list_on_right_check.get_active() )
    PF::PhotoFlow::Instance().get_options().set_ui_layers_list_placement( PF::PF_LAYERS_LIST_PLACEMENT_RIGHT );
  else
    PF::PhotoFlow::Instance().get_options().set_ui_layers_list_placement( PF::PF_LAYERS_LIST_PLACEMENT_LEFT );

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
