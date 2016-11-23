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

#include "operation_config_gui.hh"
#include "operationstree.hh"
#include "widgets/toolbutton.hh"

namespace PF {

class ImageEditor;

class ControlsGroup: public Gtk::VBox
{
  ImageEditor* editor;
  std::vector<Gtk::Frame*> controls;
  std::vector<PF::OperationConfigGUI*> guis;

public:
  ControlsGroup( ImageEditor* editor );
  size_t size() { return controls.size(); }
  /**/
  void clear();
  void populate();
  void update();
  void add_control(PF::Layer* layer, PF::OperationConfigGUI* control);
  void remove_control(PF::OperationConfigGUI* control);
  bool remove_all_controls();
  void collapse_all();
  /**/
  //void set_controls( std::vector<Gtk::Frame*>& new_controls);
};


class LayerWidget : public Gtk::VBox
{
  Image* image;
  ImageEditor* editor;

  Gtk::VPaned layers_panel;
  Gtk::VBox top_box;
  Gtk::HBox main_box;
  Gtk::VBox vbox;
  Gtk::Notebook notebook;
  Gtk::ScrolledWindow controls_scrolled_window;
  ControlsGroup controls_group;
  //Gtk::HButtonBox buttonbox;
  Gtk::HBox buttonbox;
  Gtk::Button buttonAdd, buttonAddGroup, buttonDel;
  Gtk::Button buttonPresetLoad, buttonPresetSave;
  Gtk::Dialog layersDialog;
  OperationsTreeDialog operationsDialog;

  Gtk::VBox tool_buttons_box;
  ToolButton add_button, group_button, trash_button, insert_image_button, curves_button;
  ToolButton uniform_button, gradient_button, path_mask_button, desaturate_button, crop_button;
  ToolButton basic_edits_button, levels_button, draw_button, clone_button, perspective_button, scale_button;

  std::vector<Gtk::ScrolledWindow*> layer_frames;
  std::vector<LayerTree*> layer_views;

  int get_selected_layer_id();
  bool get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter);
  bool get_row(int id, Gtk::TreeModel::iterator& iter);
  void select_row(int id);

  void unset_sticky_and_editing( Layer* l );
  void unset_sticky_and_editing( std::list<Layer*>& layers );
  void detach_controls( Layer* l );
  void detach_controls( std::list<Layer*>& layers );
  int get_map_tab( std::list<Layer*>* map_layers );
  void close_map_tabs( Layer* l );

public:
  sigc::signal<void,int> signal_active_layer_changed;

  LayerWidget( Image* image, ImageEditor* editor );
  virtual ~LayerWidget( );

  Image* get_image() { return image; }
  /*
  void set_image( Image* img ) { 
    image = img; 
    layer_views[0]->set_layers( &(image->get_layer_manager().get_layers()) );
    image->get_layer_manager().signal_modified.connect(sigc::mem_fun(this, &LayerWidget::update) );
  }
  */
  ControlsGroup& get_controls_group() { return controls_group; }
  Gtk::VBox& get_tool_buttons_box() { return tool_buttons_box; }

  void add_layer( Layer* layer, bool do_update=true  );
  void insert_image( std::string filename );
  void insert_preset( std::string filename );
  void remove_layers();

  void update( bool force_rebuild=false ) {
#ifndef NDEBUG
    std::cout<<"LayerWidget::update() called."<<std::endl;
    if( layer_views.size() > 0 )
      std::cout<<"layer_views.size() > 0"<<std::endl;
#endif
    for(unsigned int i = 0; i < layer_views.size(); i++) {
      int id = layer_views[i]->get_selected_layer_id();
#ifndef NDEBUG
      std::cout<<"LayerWidget::update() view #"<<i<<"  selected layer id="<<id<<std::endl;
#endif
      if( force_rebuild )
        layer_views[i]->set_tree_modified();
      layer_views[i]->update_model();
      layer_views[i]->select_row( id );
    }
  }

  void update_controls();

  static gboolean update_cb(PF::LayerWidget* w)
  {
    //std::cout<<"LayerWidget::update_cb() called."<<std::endl;
    if( w ) w->update();
    return( FALSE );
  }

  void update_idle()
  {
    //std::cout<<"LayerWidget::update_idle() called."<<std::endl;
    gdk_threads_add_idle ((GSourceFunc) LayerWidget::update_cb, this);
  }


  //bool on_button_event( GdkEventButton* button );

  void on_button_add();
  void on_button_add_group();
  void on_button_add_image();
  void on_button_del();

  void save_preset(std::string filename);

  void delete_selected_layers();
  void cut_selected_layers();
  void copy_selected_layers();
  void paste_layers();

  void on_button_load();
  void on_button_save();

  void on_selection_changed();

  void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
  void on_row_expanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path );
  void on_row_collapsed( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path );

#ifdef GTKMM_3
  void on_switch_page(Widget* page, guint page_num);
#endif
#ifdef GTKMM_2
  void on_switch_page(_GtkNotebookPage* page, guint page_num);
#endif

  void remove_tab( Gtk::Widget* widget );

  void modified() { /*if(image) image->modified();*/ }
};

}


#endif
