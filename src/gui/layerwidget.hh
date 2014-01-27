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

#include "layertree.hh"

namespace PF {

class LayerWidget : public Gtk::Notebook
{
  std::vector<Gtk::ScrolledWindow*> layer_frames;
  std::vector<LayerTree*> layer_views;
  std::vector<LayerTree*> layer_trees;

  LayerManager* layer_manager;

public:
  LayerWidget(  );
  virtual ~LayerWidget( );

  sigc::signal<void> signal_redraw;

  LayerManager* get_layer_manager() { return layer_manager; }
  void set_layer_manager( LayerManager* lm ) { 
    layer_manager = lm; 
    layer_views[0]->set_layers( &(layer_manager->get_layers()) );
  }

  void on_cell_toggled(const Glib::ustring& path);
};

}


#endif
