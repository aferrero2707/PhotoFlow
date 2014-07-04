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

#ifndef LAYER_TREE__HH
#define LAYER_TREE__HH

#include <gtkmm.h>

#include "../base/layer.hh"

namespace PF {

  // Definition of the calumns in the layer list
  class LayerTreeColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    
    LayerTreeColumns()
    { add(col_visible); add(col_name); add(col_imap); add(col_omap); add(col_layer); }
    
    Gtk::TreeModelColumn<bool> col_visible;
    Gtk::TreeModelColumn<Glib::ustring> col_name;
    Gtk::TreeModelColumn<Layer*> col_layer;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > col_imap;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > col_omap;
  };

  class LayerTree : public Gtk::ScrolledWindow
  {

    LayerTreeColumns columns;

    // Tree model to be filled with individial layers informations
    Glib::RefPtr<Gtk::TreeStore> treeModel;

    Gtk::TreeView treeView;

    //Image* image;
    //LayerManager* layer_manager;
    std::list<Layer*>* layers;

    bool map_flag;

    void update_model(Gtk::TreeModel::Row parent_row);

    bool select_layer( int id, Gtk::TreeModel::Row& parent_row );

  public:
    LayerTree( bool is_map=false );
    virtual ~LayerTree();

    Glib::RefPtr<Gtk::TreeStore> get_model() { return treeModel; }
    LayerTreeColumns& get_columns() { return columns; }

    Gtk::TreeView& get_tree() { return treeView; }

    PF::Layer* get_selected_layer();

    bool select_layer( int id );

    bool is_map() { return map_flag; }

    //Image* get_image() { return image; }
    //void set_image(Image* img) { image = img; update_model(); }

    void on_cell_toggled(const Glib::ustring& path);

    // Updates the tree model with the layers from the associated image
    void update_model();

    std::list<Layer*>* get_layers() { return layers; }
    void set_layers( std::list<Layer*>* l ) {
      layers = l;
      update_model();
    }

  };

}


#endif
