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

#ifndef LAYER_WIDGET__HH
#define LAYER_WIDGET__HH

#include <gtkmm.h>

#include "../base/image.hh"

#include "layertree.hh"

#include "operation_config_dialog.hh"

namespace PF {

class LayerWidget : public Gtk::Notebook
{
  std::vector<Gtk::ScrolledWindow*> layer_frames;
  std::vector<LayerTree*> layer_views;

  Image* image;

public:
  LayerWidget(  );
  virtual ~LayerWidget( );

  Image* get_image() { return image; }
  void set_image( Image* img ) { 
    image = img; 
    layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
  }

  void on_cell_toggled(const Glib::ustring& path);

  void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
};

}


#endif
