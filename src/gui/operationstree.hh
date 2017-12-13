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

#ifndef OPERATIONS_TREE__HH
#define OPERATIONS_TREE__HH

#include <gtkmm.h>


namespace PF {

  class LayerWidget;

  // Definition of the calumns in the operations list
  class OperationsTreeColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    
    OperationsTreeColumns()
    { 
      add(col_name); 
      add(col_nickname); 
      add(col_help);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> col_name;
    Gtk::TreeModelColumn<std::string> col_nickname;
    Gtk::TreeModelColumn<Glib::ustring> col_help;
  };

  class OperationsTree : public Gtk::TreeView
  {

    OperationsTreeColumns columns;

    // Tree model to be filled with individial operationss informations
    Glib::RefPtr<Gtk::TreeStore> treeModel;

    Gtk::TreeStore::Row row_vips;

  public:
    OperationsTree( );
    virtual ~OperationsTree();

    // Updates the tree model with the layers from the associated image
    void add_op( Glib::ustring name, const std::string nik);

    Glib::RefPtr<Gtk::TreeStore> get_model() { return treeModel; }
    OperationsTreeColumns& get_columns() { return columns; }
  };


  class OperationsTreeWidget: public Gtk::HBox
  {
    Gtk::ScrolledWindow left_win;
    Gtk::Frame left_frame;
    OperationsTree tree;
    Gtk::ScrolledWindow right_win;
    Gtk::Frame right_frame;
    Gtk::TextView textview;
  public:
    OperationsTreeWidget();
    OperationsTree& get_tree() { return tree; }

    void on_selection_changed();
  };


  class OperationsTreeDialog: public Gtk::Dialog
  {
    Gtk::Notebook notebook;

    //Gtk::ScrolledWindow op_load_box;
    //OperationsTree op_load;
    OperationsTreeWidget op_load;

    //Gtk::ScrolledWindow op_raw_box;
    //OperationsTree op_raw;
    OperationsTreeWidget op_raw;

    //Gtk::ScrolledWindow op_conv_box;
    //OperationsTree op_conv;
    OperationsTreeWidget op_conv;

    //Gtk::ScrolledWindow op_color_box;
    //OperationsTree op_color;
    OperationsTreeWidget op_color;

    //Gtk::ScrolledWindow op_detail_box;
    //OperationsTree op_detail;
    OperationsTreeWidget op_detail;

    //Gtk::ScrolledWindow op_geom_box;
    //OperationsTree op_geom;
    OperationsTreeWidget op_geom;
    OperationsTreeWidget op_mask;

    //Gtk::ScrolledWindow op_gmic_box;
    OperationsTreeWidget op_gmic;

    //Gtk::ScrolledWindow op_misc_box;
    //OperationsTree op_misc;
    OperationsTreeWidget op_misc;

    Image* image;

    LayerWidget* layer_widget;

  public:
    OperationsTreeDialog(Image* image, LayerWidget* layer_widget);
    virtual ~OperationsTreeDialog();

    void on_button_clicked(int id);

    void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

    void add_layer();

    void open();
  };

}


#endif
