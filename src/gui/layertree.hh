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

#include "../base/layermanager.hh"

namespace PF {

  // Definition of the calumns in the layer list
  class LayerTreeColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    
    LayerTreeColumns()
    { add(col_visible); add(col_name); add(col_layer); }
    
    Gtk::TreeModelColumn<bool> col_visible;
    Gtk::TreeModelColumn<Glib::ustring> col_name;
    Gtk::TreeModelColumn<Layer*> col_layer;
  };

class LayerTree : public Gtk::TreeView
{

  LayerTreeColumns columns;

  // Tree model to be filled with individial layers informations
  Glib::RefPtr<Gtk::TreeStore> treeModel;

  //Image* image;
  //LayerManager* layer_manager;
  std::list<Layer*>* layers;

  // Updates the tree model with the layers from the associated image
  void update_model();

public:
  LayerTree( );
  virtual ~LayerTree();

  Glib::RefPtr<Gtk::TreeStore> get_model() { return treeModel; }
  LayerTreeColumns& get_columns() { return columns; }

  //Image* get_image() { return image; }
  //void set_image(Image* img) { image = img; update_model(); }

  std::list<Layer*>* get_layers() { return layers; }
  void set_layers( std::list<Layer*>* l ) {
    layers = l;
    update_model();
  }

};

}


#endif
