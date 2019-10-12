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

#include "../base/photoflow.hh"
#include "imageeditor.hh"
#include "softproofdialog.hh"


PF::SoftProofDialog::SoftProofDialog(PF::ImageEditor* ed):
      Gtk::Dialog( _("Soft Proofing Setup"),false),
      proofed_profile_frame( _("proofed profile settings") ),
      bpc_button( _("black point compensation") ),
      adaptation_label(_("adaptation state")),
      display_profile_frame( _("display profile settings") ),
      paper_sim_frame( _("paper simulation") ),
      sim_black_ink_button( _("sim. black ink") ),
      sim_paper_color_button( _("sim. paper color") ),
      //clipping_frame( _("clipping") ),
      clip_negative_button( _("clip negative") ),
      clip_overflow_button( _("clip overflow") ),
      gamut_warning_button( _("gamut warning") ),
      profile_open_button(Gtk::Stock::OPEN),
      close_button(_("Close")), editor( ed )
{
  set_default_size(300,150);

  //add_button( _("Close"), 0 );

  // ICC profile settings
  profile_model = Gtk::ListStore::create(profile_columns);
  profile_selector.set_model( profile_model );

  Gtk::TreeModel::iterator ri = profile_model->append();
  Gtk::TreeModel::Row row = *(ri);
  row[profile_columns.col_id] = -100;
  row[profile_columns.col_text] = _("profile from image");
  profile_selector.set_active( row );
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_sRGB;
  row[profile_columns.col_value] = "sRGB";
  row[profile_columns.col_text] = "sRGB";
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_REC2020;
  row[profile_columns.col_value] = "REC2020";
  row[profile_columns.col_text] = "Rec.2020";
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_ADOBE;
  row[profile_columns.col_value] = "ADOBE";
  row[profile_columns.col_text] = "AdobeRGB";
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_PROPHOTO;
  row[profile_columns.col_value] = "PROPHOTO";
  row[profile_columns.col_text] = "ProPhotoRGB";
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_ACEScg;
  row[profile_columns.col_value] = "ACEScg";
  row[profile_columns.col_text] = "ACEScg";
  ri = profile_model->append();
  row = *(ri);
  row[profile_columns.col_id] = PF::PROF_TYPE_ACES;
  row[profile_columns.col_value] = "ACES";
  row[profile_columns.col_text] = "ACES";
  row = *(ri);
  row[profile_columns.col_id] = -200;
  row[profile_columns.col_text] = _("profile from disk");

  profile_selector.pack_start(profile_columns.col_text);
  //profile_selector.set_active( 0 );

  proofed_profile_box.pack_start( profile_selector, Gtk::PACK_SHRINK, 5 );
  profile_box.pack_start( profile_entry, Gtk::PACK_SHRINK );
  profile_box.pack_start( profile_open_button, Gtk::PACK_SHRINK );
  proofed_profile_box.pack_start( profile_box, Gtk::PACK_SHRINK, 5 );

  clip_negative_button.set_active( true );
  clip_overflow_button.set_active( true );
  clipping_hbox.pack_start( clip_negative_button, Gtk::PACK_SHRINK, 5 );
  clipping_hbox.pack_start( clip_overflow_button, Gtk::PACK_SHRINK, 5 );
  //clipping_frame.add( clipping_hbox );
  //sim_clipping_hbox.pack_start( clipping_frame, Gtk::PACK_SHRINK, 5 );

  // rendering intent settings
  intent_model = Gtk::ListStore::create(intent_columns);
  intent_selector.set_model( intent_model );

  ri = intent_model->append();
  row = *(ri);
  row[intent_columns.col_id] = INTENT_RELATIVE_COLORIMETRIC;
  row[intent_columns.col_value] = "INTENT_RELATIVE_COLORIMETRIC";
  row[intent_columns.col_text] = "relative colorimetric";
  intent_selector.set_active( row );
  ri = intent_model->append();
  row = *(ri);
  row[intent_columns.col_id] = INTENT_PERCEPTUAL;
  row[intent_columns.col_value] = "INTENT_PERCEPTUAL";
  row[intent_columns.col_text] = "perceptual";
  ri = intent_model->append();
  row = *(ri);
  row[intent_columns.col_id] = INTENT_ABSOLUTE_COLORIMETRIC;
  row[intent_columns.col_value] = "INTENT_ABSOLUTE_COLORIMETRIC";
  row[intent_columns.col_text] = "absolute colorimetric";
  ri = intent_model->append();
  row = *(ri);
  row[intent_columns.col_id] = INTENT_SATURATION;
  row[intent_columns.col_value] = "INTENT_SATURATION";
  row[intent_columns.col_text] = "saturation";

  intent_selector.pack_start(intent_columns.col_text);
  intent_selector.set_active( 0 );

  proofed_profile_box.pack_start( intent_selector, Gtk::PACK_SHRINK, 5 );

  bpc_button.set_active();
  proofed_profile_box.pack_start( bpc_button, Gtk::PACK_SHRINK, 5 );

  //get_vbox()->pack_start( sim_clipping_hbox, Gtk::PACK_SHRINK, 5 );
  proofed_profile_box.pack_start( clipping_hbox, Gtk::PACK_SHRINK, 5 );
  //proofed_profile_box.pack_start( clipping_frame, Gtk::PACK_SHRINK, 5 );

  proofed_profile_box.pack_start( gamut_warning_button, Gtk::PACK_SHRINK, 5 );
  proofed_profile_frame.add( proofed_profile_box );

  get_vbox()->pack_start( proofed_profile_frame, Gtk::PACK_SHRINK, 5 );


  paper_sim_vbox.pack_start( sim_black_ink_button, Gtk::PACK_SHRINK, 5 );
  paper_sim_vbox.pack_start( sim_paper_color_button, Gtk::PACK_SHRINK, 5 );
  //paper_sim_frame.add( paper_sim_vbox );
  //sim_clipping_hbox.pack_start( paper_sim_frame, Gtk::PACK_SHRINK, 5 );

  display_profile_box.pack_start( paper_sim_vbox, Gtk::PACK_SHRINK, 5 );
  //display_profile_box.pack_start( paper_sim_frame, Gtk::PACK_SHRINK, 5 );

  adaptation_slider.set_range(0,1);
  adaptation_slider.set_value(0);
  adaptation_slider.set_digits(2);
  adaptation_slider.set_increments(0.1,0.01);

  display_profile_box.pack_start( adaptation_label, Gtk::PACK_SHRINK, 0 );
  display_profile_box.pack_start( adaptation_slider, Gtk::PACK_SHRINK, 0 );
  display_profile_frame.add( display_profile_box );

  get_vbox()->pack_start( display_profile_frame, Gtk::PACK_SHRINK, 15 );

  close_button_box.pack_end( close_button, Gtk::PACK_SHRINK, 5 );
#ifdef GTKMM_3
  get_content_area()->pack_end( close_button_box, Gtk::PACK_SHRINK );
#else
  get_vbox()->pack_end( close_button_box, Gtk::PACK_SHRINK );
#endif

  close_button.signal_clicked().connect( sigc::mem_fun(*editor,
      &PF::ImageEditor::soft_proof_disable) );


  profile_selector.signal_changed().
    connect(sigc::mem_fun(*this, &SoftProofDialog::on_profile_selector_changed));

  intent_selector.signal_changed().
    connect(sigc::mem_fun(*this, &SoftProofDialog::on_intent_selector_changed));

  bpc_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_bpc_toggled) );

  sim_black_ink_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_sim_black_ink_toggled) );
  sim_paper_color_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_sim_paper_color_toggled) );

  adaptation_slider.signal_value_changed().
    connect(sigc::mem_fun(*this,
        &PF::SoftProofDialog::on_adaptation_changed));


  clip_negative_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_clip_negative_toggled) );
  clip_overflow_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_clip_overflow_toggled) );

  gamut_warning_button.signal_toggled().connect( sigc::mem_fun(*this,
      &PF::SoftProofDialog::on_gamut_warning_toggled) );

  signal_response().connect( sigc::mem_fun(*this,
      &SoftProofDialog::on_button_clicked) );

  profile_open_button.signal_clicked().connect(sigc::mem_fun(*this,
                &SoftProofDialog::on_button_profile_open_clicked) );

  //signal_delete_event().connect( sigc::mem_fun(*this,
  //    &Gtk::Widget::hide_on_delete) );

  show_all_children();

  set_deletable ( false );
}


PF::SoftProofDialog::~SoftProofDialog()
{
  std::cout<<"SoftProofDialog::~SoftProofDialog() called."<<std::endl;
}


void PF::SoftProofDialog::open()
{
  //op_tree.update_model();
  show_all();
}

void PF::SoftProofDialog::close()
{
  hide();
}


void PF::SoftProofDialog::on_show()
{
  Gtk::Dialog::on_show();
  //on_profile_selector_changed();
  update_intent();
  update_bpc();
  update_adaptation();
  update_sim_black_ink();
  update_sim_paper_color();
  update_clip_negative();
  update_clip_overflow();
  update_gamut_warning();
  if( editor->get_image() ) editor->get_image()->update();
}


void PF::SoftProofDialog::on_hide()
{
  std::cout<<"SoftProofDialog::on_hide() called."<<std::endl;
  Gtk::Dialog::on_hide();
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return;
  imageArea->disable_softproof();
  if( editor->get_image() ) editor->get_image()->update();
}


bool PF::SoftProofDialog::on_delete_event( GdkEventAny* any_event )
{
  std::cout<<"SoftProofDialog::on_delete_event() called."<<std::endl;
  editor->soft_proof_disable();
  //on_hide();
  return true;
  //return Dialog::on_delete_event(any_event);
}


void PF::SoftProofDialog::on_button_clicked(int id)
{
  switch(id) {
  case 0:
    //hide_all();
    hide();
    break;
  }
}


bool PF::SoftProofDialog::update_profile()
{
  Gtk::TreeModel::iterator iter = profile_selector.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      PF::ImageArea* imageArea = editor->get_image_area();
      if( !imageArea ) return false;
      PF::ProcessorBase* softproof_conv = imageArea->get_softproof_conversion();
      if( !softproof_conv ) return false;
      PF::OpParBase* softproof_par = softproof_conv->get_par();
      if( !softproof_par ) return false;
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      int id = row[profile_columns.col_id];
      switch( id ) {
      case -100:
        imageArea->disable_softproof();
        profile_box.hide();
        intent_selector.hide();
        bpc_button.hide();
        clipping_frame.hide();
        gamut_warning_button.hide();
        break;
      case -200:
        if( !custom_profile_name.empty() ) {
          imageArea->enable_softproof();
          softproof_par->set_property_value( "profile_mode", "FROM_DISK" );
          softproof_par->set_property_value( "profile_name", custom_profile_name );
        } else {
          imageArea->disable_softproof();
        }
        profile_box.show();
        intent_selector.show();
        bpc_button.show();
        clipping_frame.show();
        gamut_warning_button.show();
        break;
      default: {
        imageArea->enable_softproof();
        //softproof_par->set_property_value( "profile_mode", "CUSTOM" );
        std::string prop_val = row[profile_columns.col_value];
        softproof_par->set_property_value( "profile_mode", prop_val );
        softproof_par->set_property_value( "trc_type", "TRC_STANDARD" );
        profile_box.hide();
        intent_selector.show();
        bpc_button.show();
        clipping_frame.show();
        gamut_warning_button.show();
        break;
      }
      }
    }
    return true;
  }
  return false;
}


void PF::SoftProofDialog::on_profile_selector_changed()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_profile() ) image->update();
}


bool PF::SoftProofDialog::update_intent()
{
  Gtk::TreeModel::iterator iter = intent_selector.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      PF::ImageArea* imageArea = editor->get_image_area();
      if( !imageArea ) return false;
      PF::ProcessorBase* softproof_conv = imageArea->get_softproof_conversion();
      if( !softproof_conv ) return false;
      PF::OpParBase* softproof_par = softproof_conv->get_par();
      if( !softproof_par ) return false;

      std::string prop_val = row[intent_columns.col_value];
      softproof_par->set_property_value( "rendering_intent", prop_val );
    }
    return true;
  }
  return false;
}


void PF::SoftProofDialog::on_intent_selector_changed()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_intent() ) image->update();
}


bool PF::SoftProofDialog::update_bpc()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_softproof_bpc( bpc_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_bpc_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_bpc() ) image->update();
}


bool PF::SoftProofDialog::update_adaptation()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_softproof_adaptation_state( adaptation_slider.get_value() );
  return true;
}


void PF::SoftProofDialog::on_adaptation_changed()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_adaptation() ) image->update();
}


bool PF::SoftProofDialog::update_sim_black_ink()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_sim_black_ink( sim_black_ink_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_sim_black_ink_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_sim_black_ink() ) image->update();
}


bool PF::SoftProofDialog::update_sim_paper_color()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_sim_paper_color( sim_paper_color_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_sim_paper_color_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_sim_paper_color() ) image->update();
}


bool PF::SoftProofDialog::update_clip_negative()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_clip_negative( clip_negative_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_clip_negative_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_clip_negative() ) image->update();
}


bool PF::SoftProofDialog::update_clip_overflow()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_clip_overflow( clip_overflow_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_clip_overflow_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_clip_overflow() ) image->update();
}


bool PF::SoftProofDialog::update_gamut_warning()
{
  PF::ImageArea* imageArea = editor->get_image_area();
  if( !imageArea ) return false;
  imageArea->set_gamut_warning( gamut_warning_button.get_active() );
  return true;
}


void PF::SoftProofDialog::on_gamut_warning_toggled()
{
  PF::Image* image = editor->get_image();
  if( !image ) return;
  if( update_gamut_warning() ) image->update();
}


void PF::SoftProofDialog::on_button_profile_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose an ICC profile",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_icc_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

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
      custom_profile_name = filename.c_str();
      profile_entry.set_text( filename.c_str() );
      on_profile_selector_changed();
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

