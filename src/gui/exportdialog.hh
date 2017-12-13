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

#ifndef EXPORT_DIALOG__HH
#define EXPORT_DIALOG__HH

#include <gtkmm.h>


namespace PF {

enum export_format_t
{
  EXPORT_FORMAT_JPEG,
  EXPORT_FORMAT_TIFF
};

  class ImageEditor;

  class ExportDialog: public Gtk::Dialog
  {
    //Tree model columns:
    class UnitsModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

      UnitsModelColumns()
      { add(col_name); add(col_id); add(col_value); }

      Gtk::TreeModelColumn<Glib::ustring> col_name;
      Gtk::TreeModelColumn<int> col_id;
      Gtk::TreeModelColumn<Glib::ustring> col_value;
    };

    UnitsModelColumns units_columns;

    Gtk::HBox units_hbox;
    Gtk::Label units_label;
    Gtk::ComboBox units_cbox;
    Glib::RefPtr<Gtk::ListStore> units_model;

    Gtk::HBox scale_pixels_box;
#ifdef GTKMM_2
    Gtk::Adjustment scale_width_pixels_adjustment;
    Gtk::Adjustment scale_height_pixels_adjustment;
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::Adjustment> scale_width_pixels_adjustment;
    Glib::RefPtr<Gtk::Adjustment> scale_height_pixels_adjustment;
#endif
    NumEntry scale_width_pixels_entry;
    NumEntry scale_height_pixels_entry;

    Gtk::VBox resize_vbox;
    Gtk::Frame resize_frame;

    Gtk::Frame preview_frame;
    Gtk::VBox preview_box;
    ImageEditor* editor;
    export_format_t export_format;
    std::string file_name;
  public:
    ExportDialog();
    virtual ~ExportDialog();

    void set_editor(ImageEditor* e) { editor = e; }
    void set_export_format(export_format_t fmt) { export_format = fmt; }
    void set_file_name( std::string name ) { file_name = name; }

    void on_button_clicked(int id);

    void open();

    void on_show();
    void on_hide();

    bool on_delete_event( GdkEventAny *   any_event );
  };

}


#endif
