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
#include "../base/file_util.hh"
#include "../base/fileutils.hh"
#include "../base/iccstore.hh"
#include "imageeditor.hh"
#include "exportdialog.hh"



PF::TextSelector::TextSelector(Glib::ustring l): label(l)
{
  model = Gtk::ListStore::create(columns);
  cbox.set_model( model );
  cbox.pack_start(columns.col_name);
  pack_start( label, Gtk::PACK_SHRINK, 2 );
  pack_start( cbox, Gtk::PACK_SHRINK, 2 );
}


void PF::TextSelector::add_entry(Glib::ustring text, int id)
{
  Gtk::TreeModel::iterator ri;
  ri = model->append();
  (*(ri))[columns.col_name] = text;
  (*(ri))[columns.col_id] = id;
  cbox.set_active( ri );
}


void PF::TextSelector::set_active(int id)
{
  Glib::RefPtr<Gtk::TreeModel> model = cbox.get_model();
  Gtk::TreeModel::Children rows = model->children();
  for(unsigned int i = 0; i < rows.size(); i++) {
    Gtk::TreeModel::Row row = rows[i];
    if( row[columns.col_id] == id ) {
      cbox.set_active(i);
      break;
    }
  }
}


int PF::TextSelector::get_active_id()
{
  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      return( row[columns.col_id] );
    }
  }
  return(-1);
}


Glib::ustring PF::TextSelector::get_active_text()
{
  Glib::ustring result;
  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      result = row[columns.col_name];
    }
  }
  return(result);
}



PF::ExportDialog::ExportDialog():
Gtk::Dialog( _("Export image"),false),
file_button(_("Export to...")),
export_format_selector(_("format:")),
jpeg_quant_table_selector(_("quantization table:")),
tiff_format_selector(_("file format:")),
size_selector(_("output size:")),
units_selector(_("unit:")),
#ifdef GTKMM_2
scale_width_pixels_adjustment( 1920, 1, 10000, 1, 10, 0 ),
scale_height_pixels_adjustment( 1080, 1, 10000, 1, 10, 0 ),
#endif
icc_profile(_("output colorspace:")),
icc_trc(_("encoding:")),
icc_intent(_("intent:")),
editor( NULL ),
export_format(PF::EXPORT_FORMAT_JPEG)
{
  set_default_size(800,600);

  file_hbox.pack_start( file_button, Gtk::PACK_SHRINK, 4 );
  file_hbox.pack_start( file_entry, Gtk::PACK_EXPAND_WIDGET, 8 );

  get_vbox()->pack_start( file_hbox, Gtk::PACK_SHRINK, 2 );

  get_vbox()->pack_start( top_separator, Gtk::PACK_SHRINK, 2 );

  //get_vbox()->pack_start( file_chooser, Gtk::PACK_EXPAND_WIDGET, 15 );


  left_vbox.set_spacing(5);

  export_format_selector.add_entry( _("JPEG (8 bit)"), PF::EXPORT_FORMAT_JPEG );
  export_format_selector.add_entry( _("TIFF (8 bit)"), PF::EXPORT_FORMAT_TIFF_8 );
  export_format_selector.add_entry( _("TIFF (16 bit"), PF::EXPORT_FORMAT_TIFF_16 );
  export_format_selector.add_entry( _("TIFF (32 bit float)"), PF::EXPORT_FORMAT_TIFF_32f );
  export_format_selector.set_active(PF::EXPORT_FORMAT_JPEG);
  left_vbox.pack_start( export_format_selector, Gtk::PACK_SHRINK, 4 );
  //left_vbox.pack_start( format_type_separator, Gtk::PACK_SHRINK, 0 );

  // Jpeg options
  jpeg_options_vbox.set_spacing(2);
  //jpeg_options_label.set_text(_("Jpeg options"));
  jpeg_options_vbox.pack_start( jpeg_options_label, Gtk::PACK_SHRINK );
  jpeg_quality_label.set_text(_("quality:"));
  jpeg_quality_hbox.pack_start( jpeg_quality_label, Gtk::PACK_SHRINK, 2 );
  jpeg_quality_scale.set_range(0,100);
  jpeg_quality_scale.set_increments(1,10);
  jpeg_quality_scale.set_value(80);
  jpeg_quality_scale.set_size_request(100,-1);
  jpeg_quality_hbox.pack_start( jpeg_quality_scale, Gtk::PACK_EXPAND_WIDGET, 2 );
  jpeg_options_vbox.pack_start( jpeg_quality_hbox, Gtk::PACK_SHRINK );
  // chroma subsampling
  jpeg_chroma_subsampling_label.set_text(_("chroma subsampling:"));
  jpeg_chroma_subsampling_hbox.pack_start( jpeg_chroma_subsampling_label, Gtk::PACK_SHRINK, 2 );
  jpeg_chroma_subsampling_hbox.pack_start( jpeg_chroma_subsampling_check, Gtk::PACK_SHRINK, 2 );
  jpeg_options_vbox.pack_start( jpeg_chroma_subsampling_hbox, Gtk::PACK_SHRINK, 5 );
  // quantization table
  jpeg_quant_table_selector.add_entry( _("default"), PF::JPEG_QUANT_TABLE_DEFAULT );
  jpeg_quant_table_selector.add_entry( _("medium"), PF::JPEG_QUANT_TABLE_MEDIUM );
  jpeg_quant_table_selector.add_entry( _("best"), PF::JPEG_QUANT_TABLE_BEST );
  jpeg_quant_table_selector.set_active(PF::JPEG_QUANT_TABLE_BEST);
  //jpeg_options_vbox.pack_start( jpeg_quant_table_selector, Gtk::PACK_SHRINK );
  left_vbox.pack_start( jpeg_options_vbox, Gtk::PACK_SHRINK );

  tiff_options_vbox.set_spacing(2);
  //tiff_format_selector.add_entry( _("8 bit"), 0 );
  //tiff_format_selector.add_entry( _("16 bit"), 1 );
  //tiff_format_selector.add_entry( _("32 bit float"), 2 );
  //tiff_options_vbox.pack_start( tiff_format_selector, Gtk::PACK_SHRINK );
  tiff_compressed_label.set_text(_("compressed"));
  tiff_compressed_hbox.pack_start( tiff_compressed_label, Gtk::PACK_SHRINK, 2 );
  tiff_compressed_hbox.pack_start( tiff_compressed_check, Gtk::PACK_SHRINK, 2 );
  tiff_options_vbox.pack_start( tiff_compressed_hbox, Gtk::PACK_SHRINK );
  tiff_options_vbox.hide();
  left_vbox.pack_start( tiff_options_vbox, Gtk::PACK_SHRINK );

  left_vbox.pack_start( format_options_separator, Gtk::PACK_SHRINK );

  // output size options
  resize_vbox.set_spacing(2);
  //resize_label.set_text(_("output size"));
  //resize_vbox.pack_start( resize_label, Gtk::PACK_SHRINK );

  size_selector.add_entry(_("original"), PF::SIZE_ORIGINAL);
  size_selector.add_entry(_("400 x 300"), PF::SIZE_400_300);
  size_selector.add_entry(_("800 x 600"), PF::SIZE_800_600);
  size_selector.add_entry(_("1280 x 720"), PF::SIZE_1280_720);
  size_selector.add_entry(_("1280 x 800"), PF::SIZE_1280_800);
  size_selector.add_entry(_("1280 x 1024"), PF::SIZE_1280_1024);
  size_selector.add_entry(_("1440 x 900"), PF::SIZE_1440_900);
  size_selector.add_entry(_("1600 x 1200"), PF::SIZE_1600_1200);
  size_selector.add_entry(_("1920 x 1080"), PF::SIZE_1920_1080);
  size_selector.add_entry(_("1920 x 1200"), PF::SIZE_1920_1200);
  size_selector.add_entry(_("2048 x 1400"), PF::SIZE_2048_1400);
  size_selector.add_entry(_("2048 x 2048"), PF::SIZE_2048_2048);
  size_selector.add_entry(_("2K"), PF::SIZE_2K);
  size_selector.add_entry(_("4K"), PF::SIZE_4K);
  size_selector.add_entry(_("5K"), PF::SIZE_5K);
  size_selector.add_entry(_("8K"), PF::SIZE_8K);
  size_selector.add_entry(_("A4 300DPI"), PF::SIZE_A4_300DPI);
  size_selector.add_entry(_("A4 300DPI (portrait)"), PF::SIZE_A4P_300DPI);
  //size_selector.add_entry(_("custom"), PF::SIZE_CUSTOM);
  size_selector.set_active(PF::SIZE_ORIGINAL);

  units_selector.add_entry(_("pixels"), 0);
  units_selector.add_entry(_("percent"), 1);
  units_selector.add_entry(_("mm"), 2);
  units_selector.add_entry(_("cm"), 3);
  units_selector.add_entry(_("inches"), 4);
  units_selector.set_active(0);

  resize_vbox.pack_start( size_selector, Gtk::PACK_SHRINK );
  //resize_vbox.pack_start( units_selector, Gtk::PACK_SHRINK );

#ifdef GTKMM_2
  scale_width_pixels_entry.set_adjustment( &scale_width_pixels_adjustment );
  scale_height_pixels_entry.set_adjustment( &scale_height_pixels_adjustment );
#endif
#ifdef GTKMM_3
  scale_width_pixels_adjustment = Gtk::Adjustment::create( 1920, 1, 10000, 1, 10, 0 );
  scale_width_pixels_entry.set_adjustment( scale_width_pixels_adjustment );
  scale_height_pixels_adjustment = Gtk::Adjustment::create( 1080, 1, 10000, 1, 10, 0 );
  scale_height_pixels_entry.set_adjustment( scale_height_pixels_adjustment );
#endif
  scale_pixels_box.pack_start( scale_width_pixels_entry, Gtk::PACK_SHRINK );
  scale_pixels_box.pack_start( scale_height_pixels_entry, Gtk::PACK_SHRINK );
  //resize_vbox.pack_start( scale_pixels_box, Gtk::PACK_SHRINK );


  resize_sharpening_check.set_active(false);
  resize_sharpening_label_hbox.pack_start( resize_sharpening_check, Gtk::PACK_SHRINK, 5 );
  resize_sharpening_label.set_text(_("post-resize sharpening"));
  resize_sharpening_label_hbox.pack_start( resize_sharpening_label, Gtk::PACK_SHRINK, 5 );
  resize_vbox.pack_start( resize_sharpening_label_hbox, Gtk::PACK_SHRINK, 5 );
  resize_sh_radius_label.set_text(_("radius:"));
  resize_sh_radius_hbox.pack_start( resize_sh_radius_label, Gtk::PACK_SHRINK, 2 );
  resize_sh_radius_scale.set_range(0,5);
  resize_sh_radius_scale.set_increments(0.1,0.5);
  resize_sh_radius_scale.set_value(0.8);
  resize_sh_radius_scale.set_size_request(150,-1);
  resize_sh_radius_hbox.pack_end( resize_sh_radius_scale, Gtk::PACK_SHRINK, 2 );
  resize_vbox.pack_start( resize_sh_radius_hbox, Gtk::PACK_SHRINK, 5 );
  resize_sh_amount_label.set_text(_("amount:"));
  resize_sh_amount_hbox.pack_start( resize_sh_amount_label, Gtk::PACK_SHRINK, 2 );
  resize_sh_amount_scale.set_range(0,100);
  resize_sh_amount_scale.set_increments(1,5);
  resize_sh_amount_scale.set_value(80);
  resize_sh_amount_scale.set_size_request(150,-1);
  resize_sh_amount_hbox.pack_end( resize_sh_amount_scale, Gtk::PACK_SHRINK, 2 );
  //resize_vbox.pack_start( resize_sh_amount_hbox, Gtk::PACK_SHRINK, 5 );

  left_vbox.pack_start( resize_vbox, Gtk::PACK_SHRINK );

  left_vbox.pack_start( size_options_separator, Gtk::PACK_SHRINK );

  icc_profile.add_entry("Working profile", PF::PROF_TYPE_EMBEDDED);
  icc_profile.add_entry("sRGB", PF::PROF_TYPE_sRGB);
  icc_profile.add_entry("Rec.2020", PF::PROF_TYPE_REC2020);
  icc_profile.add_entry("AdobeRGB", PF::PROF_TYPE_ADOBE);
  icc_profile.add_entry("ProPhoto", PF::PROF_TYPE_PROPHOTO);
  icc_profile.add_entry("ACEScg", PF::PROF_TYPE_ACEScg);
  icc_profile.add_entry("ACES", PF::PROF_TYPE_ACES);
  icc_profile.add_entry(_("custom"), PF::PROF_TYPE_FROM_DISK);
  icc_profile.set_active(PF::PROF_TYPE_sRGB);
  icc_vbox.pack_start( icc_profile, Gtk::PACK_SHRINK, 5 );
  icc_trc.add_entry("standard", PF::PF_TRC_STANDARD);
  icc_trc.add_entry("linear", PF::PF_TRC_LINEAR);
  icc_trc.set_active(PF::PF_TRC_STANDARD);
  icc_vbox.pack_start( icc_trc, Gtk::PACK_SHRINK, 5 );
  icc_intent.add_entry("relative colorimetric", INTENT_RELATIVE_COLORIMETRIC);
  icc_intent.add_entry("perceptual", INTENT_PERCEPTUAL);
  icc_intent.add_entry("saturation", INTENT_SATURATION);
  icc_intent.add_entry("absolute colorimetric", INTENT_ABSOLUTE_COLORIMETRIC);
  icc_intent.set_active(INTENT_RELATIVE_COLORIMETRIC);
  icc_vbox.pack_start( icc_intent, Gtk::PACK_SHRINK, 5 );
  icc_bpc_label.set_text(_("black point compensation:"));
  icc_bpc_hbox.pack_start( icc_bpc_label, Gtk::PACK_SHRINK, 2 );
  icc_bpc_check.set_active(false);
  icc_bpc_hbox.pack_start( icc_bpc_check, Gtk::PACK_SHRINK, 2 );
  icc_vbox.pack_start( icc_bpc_hbox, Gtk::PACK_SHRINK, 5 );
  icc_file_hbox.pack_start( icc_file_button, Gtk::PACK_SHRINK, 4 );
  icc_file_hbox.pack_start( icc_file_entry, Gtk::PACK_EXPAND_WIDGET, 8 );
  //icc_vbox.pack_start( icc_file_hbox, Gtk::PACK_SHRINK, 5 );

  left_vbox.pack_start( icc_vbox, Gtk::PACK_SHRINK );

  right_vbox.set_size_request(600,500);
  right_vbox.pack_start( preview_frame, Gtk::PACK_EXPAND_WIDGET, 15 );

  middle_hbox.pack_start( left_vbox, Gtk::PACK_SHRINK, 15 );
  middle_hbox.pack_start( vertical_separator, Gtk::PACK_SHRINK, 15 );
  middle_hbox.pack_start( right_vbox, Gtk::PACK_SHRINK, 15 );

  get_vbox()->pack_start( middle_hbox, Gtk::PACK_SHRINK, 15 );

  add_button( _("Cancel"), 0 );
  add_button( _("OK"), 1 );

  export_format_selector.get_cbox().signal_changed().connect( sigc::mem_fun(*this,
      &ExportDialog::on_format_changed) );

  icc_profile.get_cbox().signal_changed().connect( sigc::mem_fun(*this,
      &ExportDialog::on_colorspace_changed) );

  signal_response().connect( sigc::mem_fun(*this,
      &ExportDialog::on_button_clicked) );

  file_button.signal_clicked().connect( sigc::mem_fun(*this,
      &ExportDialog::on_file_button_clicked) );



  show_all_children();

  set_deletable ( true );
}


PF::ExportDialog::~ExportDialog()
{
  std::cout<<"ExportDialog::~ExportDialog() called."<<std::endl;
}


void PF::ExportDialog::open()
{
  //op_tree.update_model();
  show_all();
}


void PF::ExportDialog::on_show()
{
  std::cout<<"ExportDialog::on_show() called."<<std::endl;
  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_image_folder();
  Glib::ustring last_saved = editor->get_last_exported_file();
  std::cout<<"  last_saved: "<<last_saved<<std::endl;
  std::string tmp_filename = last_saved;
  if( tmp_filename.empty() ) {
    if( editor->get_image() && !(editor->get_image()->get_filename().empty()) ) {
      tmp_filename = editor->get_image()->get_filename();
    } else {
      tmp_filename = "untitled.jpg";
    }
  }

  std::string image_filename;
  int id = export_format_selector.get_active_id();
  switch( id ) {
  case PF::EXPORT_FORMAT_JPEG: {
    image_filename = PF::replaceFileExtension(tmp_filename, "jpg");
    break;
  }
  case PF::EXPORT_FORMAT_TIFF_8:
  case PF::EXPORT_FORMAT_TIFF_16:
  case PF::EXPORT_FORMAT_TIFF_32f: {
    image_filename = PF::replaceFileExtension(tmp_filename, "tif");
    break;
  }
  }

  file_entry.set_text( image_filename );

  on_format_changed();

  Gtk::Dialog::on_show();
}


void PF::ExportDialog::on_hide()
{
  std::cout<<"ExportDialog::on_hide() called."<<std::endl;
  Gtk::Dialog::on_hide();
}


bool PF::ExportDialog::on_delete_event( GdkEventAny* any_event )
{
  std::cout<<"ExportDialog::on_delete_event() called."<<std::endl;
  on_hide();
  return true;
  //return Dialog::on_delete_event(any_event);
}


void PF::ExportDialog::on_format_changed()
{
  int id = export_format_selector.get_active_id();
  switch( id ) {
  case PF::EXPORT_FORMAT_JPEG: {
    jpeg_options_vbox.show_all();
    jpeg_options_vbox.show();
    tiff_options_vbox.hide();
    std::string old_filename = file_entry.get_text();
    std::string new_filename = PF::replaceFileExtension(old_filename, "jpg");
    file_entry.set_text( new_filename );
    break;
  }
  case PF::EXPORT_FORMAT_TIFF_8:
  case PF::EXPORT_FORMAT_TIFF_16:
  case PF::EXPORT_FORMAT_TIFF_32f: {
    jpeg_options_vbox.hide();
    tiff_options_vbox.show_all();
    tiff_options_vbox.show();
    std::string old_filename = file_entry.get_text();
    std::string new_filename = PF::replaceFileExtension(old_filename, "tif");
    file_entry.set_text( new_filename );
    break;
  }
  default: break;
  }
}


void PF::ExportDialog::on_colorspace_changed()
{
  int id = icc_profile.get_active_id();
  switch( id ) {
  case PF::PROF_TYPE_EMBEDDED: {
    icc_trc.hide();
    icc_intent.hide();
    icc_bpc_hbox.hide();
    break;
  }
  default: {
    icc_trc.show();
    icc_intent.show();
    icc_bpc_hbox.show();
    break;
  }
  }
}


void PF::ExportDialog::on_button_clicked(int id)
{
  switch(id) {
  case 1:
    std::cout << "File selected: " <<  file_entry.get_text() << std::endl;
    if( editor && editor->get_image() ) {
      image_export_opt_t options;
      options.jpeg_quality = jpeg_quality_scale.get_value();
      options.jpeg_chroma_subsampling = jpeg_chroma_subsampling_check.get_active();
      options.jpeg_quant_table = jpeg_quant_table_selector.get_active_id();
      options.tiff_format = export_format_selector.get_active_id();
      options.tiff_compress = tiff_compressed_check.get_active();
      options.size = (PF::export_size_t)size_selector.get_active_id();
      options.width = 0;
      options.height = 0;
      options.interpolator = PF::SCALE_INTERP_LANCZOS3;
      options.sharpen_enabled = resize_sharpening_check.get_active();
      options.sharpen_radius = resize_sh_radius_scale.get_value();
      options.sharpen_amount = 100;
      options.profile_type = (profile_type_t)icc_profile.get_active_id();
      options.trc_type = (TRC_type)icc_trc.get_active_id();
      options.intent = (cmsUInt32Number)icc_intent.get_active_id();
      options.bpc = icc_bpc_check.get_active();
      editor->get_image()->export_merged( file_entry.get_text(), &options );
      editor->set_last_exported_file( file_entry.get_text() );
    }
    hide();
    break;
  case 0:
    //hide_all();
    hide();
    break;
  }
}



void PF::ExportDialog::on_file_button_clicked()
{
  Gtk::FileChooserDialog dialog( _("Export image as..."),
      Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

#ifdef GTKMM_2
  Gtk::FileFilter filter_all;
  filter_all.set_name( _("All supported formats") );
  filter_all.add_mime_type("image/jpeg");
  filter_all.add_mime_type("image/tiff");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_all = Gtk::FileFilter::create();
  filter_all->set_name( _("All supported formats") );
  filter_all->add_mime_type("image/jpeg");
  filter_all->add_mime_type("image/tiff");
#endif
  dialog.add_filter(filter_all);

#ifdef GTKMM_2
  Gtk::FileFilter filter_jpeg;
  filter_jpeg.set_name( _("JPEG files") );
  filter_jpeg.add_mime_type("image/jpeg");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_jpeg = Gtk::FileFilter::create();
  filter_jpeg->set_name( _("JPEG files") );
  filter_jpeg->add_mime_type("image/jpeg");
#endif
  dialog.add_filter(filter_jpeg);

#ifdef GTKMM_2
  Gtk::FileFilter filter_tiff;
  filter_tiff.set_name( _("TIFF files") );
  filter_tiff.add_mime_type("image/tiff");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_tiff = Gtk::FileFilter::create();
  filter_tiff->set_name( _("TIFF files") );
  filter_tiff->add_mime_type("image/tiff");
#endif
  dialog.add_filter(filter_tiff);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_image_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  Glib::ustring last_saved = editor->get_last_exported_file();
  if( !last_saved.empty() ) {
    dialog.set_filename( last_saved );
  } else {
    if( editor->get_image() ) {
      std::string image_filename;
      PF::getFileName( "", editor->get_image()->get_filename(), image_filename );
      image_filename = image_filename + ".jpg";
      dialog.set_current_name( image_filename );
    } else {
      dialog.set_current_name( "untitled.jpg" );
    }
  }

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): {
    std::cout << "Export clicked." << std::endl;

    //Notice that this is a std::string, not a Glib::ustring.
    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_image_folder( last_dir );
    std::string filename = dialog.get_filename();
#ifdef GTKMM_2
    Gtk::FileFilter* filter_cur = dialog.get_filter();
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::FileFilter> filter_cur = dialog.get_filter();
#endif

    if( filter_cur->get_name() == _("TIFF files") ) {
      std::string ext;
      if( getFileExtension( "/", filename, ext ) ) {
        if( ext != "tiff" && ext != "tif" ) {
          filename = filename + ".tif";
        }
      }
    }

    if( filter_cur->get_name() == _("JPEG files") ) {
      std::string ext;
      if( getFileExtension( "/", filename, ext ) ) {
        if( ext != "jpeg" && ext != "jpg" ) {
          filename = filename + ".jpg";
        }
      }
    }

    file_entry.set_text( filename );

    //std::cout << "File selected: " <<  filename << std::endl;
    //if( editor && editor->get_image() ) {
    //  editor->get_image()->export_merged( filename );
    //  editor->set_last_exported_file( filename );
    //}

    break;
  }
  case(Gtk::RESPONSE_CANCEL): {
    std::cout << "Cancel clicked." << std::endl;
    break;
  }
  default: {
    std::cout << "Unexpected button clicked." << std::endl;
    break;
  }
  }
}



