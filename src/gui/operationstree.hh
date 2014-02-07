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

  // Definition of the calumns in the operations list
  class OperationsTreeColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    
    OperationsTreeColumns()
    { 
      add(col_name); 
      add(col_nickname); 
    }
    
    Gtk::TreeModelColumn<Glib::ustring> col_name;
    Gtk::TreeModelColumn<Glib::ustring> col_nickname;
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
    void update_model();

    Glib::RefPtr<Gtk::TreeStore> get_model() { return treeModel; }
    OperationsTreeColumns& get_columns() { return columns; }
  };


  class OperationsTreeDialog: public Gtk::Dialog
  {
    Gtk::ScrolledWindow op_tree_box;
    OperationsTree op_tree;

  public:
    OperationsTreeDialog();
    virtual ~OperationsTreeDialog();

    void on_button_clicked(int id);

    void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

    void add_layer();

    void open();
  };

}


#endif
