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
#include "exportdialog.hh"


PF::ExportDialog::ExportDialog():
      Gtk::Dialog( _("Export image"),false),
#ifdef GTKMM_2
      scale_width_pixels_adjustment( 1920, 1, 10000, 1, 10, 0 ),
      scale_height_pixels_adjustment( 1080, 1, 10000, 1, 10, 0 ),
#endif
      editor( NULL ),
      export_format(PF::EXPORT_FORMAT_JPEG)
{
  set_default_size(300,150);

  get_vbox()->pack_start( preview_frame, Gtk::PACK_SHRINK, 15 );

  units_label.set_text( _("units") );

  units_model = Gtk::ListStore::create(units_columns);
  units_cbox.set_model( units_model );
  units_cbox.pack_start(units_columns.col_name);

  Gtk::TreeModel::iterator ri;
  ri = units_model->append();
  (*(ri))[units_columns.col_name] = _("pixels");
  (*(ri))[units_columns.col_id] = 0;
  units_cbox.set_active( ri );
  ri = units_model->append();
  (*(ri))[units_columns.col_name] = _("percent");
  (*(ri))[units_columns.col_id] = 1;
  ri = units_model->append();
  (*(ri))[units_columns.col_name] = _("mm");
  (*(ri))[units_columns.col_id] = 2;
  ri = units_model->append();
  (*(ri))[units_columns.col_name] = _("cm");
  (*(ri))[units_columns.col_id] = 3;
  ri = units_model->append();
  (*(ri))[units_columns.col_name] = _("inches");
  (*(ri))[units_columns.col_id] = 4;

  resize_frame.add( resize_vbox );
  resize_vbox.pack_start( units_hbox, Gtk::PACK_SHRINK );
  resize_vbox.pack_start( scale_pixels_box, Gtk::PACK_SHRINK );

  units_hbox.pack_start( units_label, Gtk::PACK_SHRINK );
  units_hbox.pack_start( units_cbox, Gtk::PACK_SHRINK );

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

  get_vbox()->pack_start( resize_frame, Gtk::PACK_SHRINK, 15 );

  add_button( _("OK"), 1 );
  add_button( _("Cancel"), 0 );

  signal_response().connect( sigc::mem_fun(*this,
      &ExportDialog::on_button_clicked) );

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


void PF::ExportDialog::on_button_clicked(int id)
{
  switch(id) {
  case 1:
    std::cout << "File selected: " <<  file_name << std::endl;
    if( editor && editor->get_image() ) {
      editor->get_image()->export_merged( file_name );
      editor->set_last_exported_file( file_name );
    }
    hide();
    break;
  case 0:
    //hide_all();
    hide();
    break;
  }
}
