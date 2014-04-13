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
#include "operationstree.hh"

#include "operation_config_dialog.hh"
#include "operationstree.hh"

namespace PF {

class LayerWidget : public Gtk::VBox
{
  Image* image;

  Gtk::Notebook notebook;
  Gtk::HButtonBox buttonbox;
  Gtk::Button buttonAdd, buttonAddGroup, buttonDel;
  Gtk::Dialog layersDialog;
  OperationsTreeDialog operationsDialog;

  std::vector<Gtk::ScrolledWindow*> layer_frames;
  std::vector<LayerTree*> layer_views;

public:
  LayerWidget( Image* image );
  virtual ~LayerWidget( );

  Image* get_image() { return image; }
  /*
  void set_image( Image* img ) { 
    image = img; 
    layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
    image->get_layer_manager().signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
  }
  */

  void add_layer( Layer* layer );
  void remove_layers();

  void update() {
    if( layer_views.size() > 0 )
      std::cout<<"layer_views.size() > 0"<<std::endl;
    for(unsigned int i = 0; i < layer_views.size(); i++) 
      layer_views[i]->update_model();
  }

  void on_button_add();
  void on_button_add_group();
  void on_button_del();

  void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

  void remove_tab( Gtk::Widget* widget );
};

}


#endif
