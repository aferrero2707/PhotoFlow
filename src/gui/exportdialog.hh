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
#include "widgets/pfwidget.hh"


namespace PF {

  class ImageEditor;


  class TextSelector_: public Gtk::HBox
  {
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

      ModelColumns() { add(col_name); add(col_id); }

      Gtk::TreeModelColumn<Glib::ustring> col_name;
      Gtk::TreeModelColumn<int> col_id;
    };

    ModelColumns columns;
    Gtk::Label label;
    Gtk::ComboBox cbox;
    Glib::RefPtr<Gtk::ListStore> model;

    public:
    TextSelector_(Glib::ustring label);
    void add_entry(Glib::ustring text, int id);

    void set_active(int id);

    Gtk::ComboBox& get_cbox() {return cbox;}
    int get_active_id();
    Glib::ustring get_active_text();
  };

  class ExportDialog: public Gtk::Dialog
  {
    //Gtk::FileChooserWidget file_chooser;
    Gtk::HBox file_hbox;
    Gtk::Button file_button;
    Gtk::Entry file_entry;
    IntSelector export_format_selector;

    Gtk::HSeparator top_separator;
    Gtk::HSeparator format_type_separator;
    Gtk::HSeparator format_options_separator;
    Gtk::HSeparator size_options_separator;
    Gtk::VSeparator vertical_separator;


    Gtk::VBox left_vbox, right_vbox;
    Gtk::HBox middle_hbox;

    Gtk::VBox jpeg_options_vbox;
    Gtk::Label jpeg_options_label;
    Gtk::HBox jpeg_quality_hbox;
    Gtk::Label jpeg_quality_label;
    Gtk::HScale jpeg_quality_scale;
    Gtk::HBox jpeg_chroma_subsampling_hbox;
    Gtk::Label jpeg_chroma_subsampling_label;
    Gtk::CheckButton jpeg_chroma_subsampling_check;
    IntSelector jpeg_quant_table_selector;

    Gtk::VBox tiff_options_vbox;
    Gtk::Label tiff_options_label;
    IntSelector tiff_format_selector;
    Gtk::HBox tiff_compressed_hbox;
    Gtk::Label tiff_compressed_label;
    Gtk::CheckButton tiff_compressed_check;

    Gtk::VBox resize_vbox;
    Gtk::Label resize_label;
    IntSelector size_selector;
    IntSelector units_selector;

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
    Gtk::HBox resize_sharpening_label_hbox;
    Gtk::CheckButton resize_sharpening_check;
    Gtk::Label resize_sharpening_label;
    Gtk::HBox resize_sh_radius_hbox;
    Gtk::Label resize_sh_radius_label;
    Gtk::HScale resize_sh_radius_scale;
    Gtk::HBox resize_sh_amount_hbox;
    Gtk::Label resize_sh_amount_label;
    Gtk::HScale resize_sh_amount_scale;


    Gtk::VBox icc_vbox;
    IntSelector icc_profile;
    IntSelector icc_trc;
    IntSelector icc_intent;
    Gtk::HBox icc_bpc_hbox;
    Gtk::Label icc_bpc_label;
    Gtk::CheckButton icc_bpc_check;
    Gtk::HBox icc_file_hbox;
    Gtk::Button icc_file_button;
    Gtk::Entry icc_file_entry;

    Gtk::Frame preview_frame;
    Gtk::VBox preview_box;
    ImageEditor* editor;
    export_format_t export_format;
    //std::string file_name;
  public:
    ExportDialog();
    virtual ~ExportDialog();

    void set_editor(ImageEditor* e) { editor = e; }
    void set_export_format(export_format_t fmt) { export_format = fmt; }
    //void set_file_name( std::string name ) { file_name = name; }

    void on_format_changed();
    void on_colorspace_changed();
    void on_file_button_clicked();
    void on_button_clicked(int id);

    void open();

    void on_show();
    void on_hide();

    bool on_delete_event( GdkEventAny *   any_event );
  };

}


#endif
