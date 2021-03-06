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
  std::vector<Gtk::Widget*> controls;
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

class AuxControlsGroup: public Gtk::VBox
{
  ImageEditor* editor;
  Gtk::Widget* controls;
  PF::OperationConfigGUI* gui;

public:
  AuxControlsGroup( ImageEditor* editor );
  void clear();
  void update();
  void set_control(PF::Layer* layer, PF::OperationConfigGUI* control);
};


class ControlsDialog: public Gtk::Dialog
{
  ImageEditor* editor;
  PF::OperationConfigGUI* gui;
  Gtk::Alignment dialog_width_align;
  Gtk::Alignment spacing1;
  Gtk::HBox dialog_width_hbox;
  Gtk::Alignment blend_box_padding_left, blend_box_padding_right;
  Gtk::HBox blend_box_hbox;
  Gtk::Alignment input_box_padding_left, input_box_padding_right;
  Gtk::HBox input_box_hbox;
  Gtk::Alignment controls_box_padding_left, controls_box_padding_right;
  Gtk::HBox controls_box_hbox;
  Gtk::EventBox controls_evbox;
  Gtk::HBox* top_box;
  Gtk::Button close_button;
  ToggleImageButton frame_close;
  Gtk::Notebook notebook;
  Gtk::HSeparator hline1, hline2, hline3;
  Gtk::EventBox top_eb, bottom_eb, controls_eb;
  Gtk::VBox top_vbox, bottom_vbox, controls_vbox;
  bool visible;
  int x, y;
public:
  ControlsDialog( ImageEditor* editor );
  void set_controls(PF::Layer* l);
  bool is_visible() { return visible; }
  void update();
  void on_realize();
  void on_hide();
  bool on_delete_event( GdkEventAny* any_event );
  void open();
  void close();

  bool on_key_press_event(GdkEventKey* event);
};


class LayerWidget : public Gtk::VBox
{
  Image* image;
  ImageEditor* editor;
  int selected_layer_id;

  LayerTree layers_view, mask_view;
  Gtk::VBox mask_view_box;
  Gtk::HBox mask_view_top_box;
  Gtk::Button mask_view_back_button;
  Gtk::CheckButton mask_view_show_button;
  Gtk::Label mask_view_show_label1;
  Gtk::Label mask_view_show_label2;
  Gtk::VBox mask_view_show_label_box;
  int active_view;

  Gtk::VPaned layers_panel;
  Gtk::VBox top_box;
  Gtk::HBox main_box;
  Gtk::VBox vbox;
  Gtk::HBox hbox;
  //Gtk::Notebook notebook;
  Gtk::ScrolledWindow controls_scrolled_window;
  ControlsGroup controls_group;
  AuxControlsGroup aux_controls_group;
  //Gtk::HButtonBox buttonbox;
  Gtk::HBox buttonbox;
  Gtk::Button buttonAdd, buttonAddGroup, buttonDel;
  Gtk::Button buttonPresetLoad, buttonPresetSave;
  Gtk::Dialog layersDialog;
  OperationsTreeDialog operationsDialog;

  Gtk::Menu tools_menu;

  std::map<PF::OperationConfigGUI*,PF::ControlsDialog*> controls_dialogs;
  bool controls_dialog_visible;
  int controls_dialog_x, controls_dialog_y;

  Gtk::VBox tool_buttons_box;
  ToolButton add_button, group_button, trash_button, insert_image_button, curves_button;
  ToolButton uniform_button, gradient_button, path_mask_button, desaturate_button, crop_button;
  ToolButton basic_edits_button, levels_button, draw_button, clone_button, perspective_button, scale_button;
  ToolButton relight_button;

  std::vector<Gtk::ScrolledWindow*> layer_frames;
  std::vector<LayerTree*> layer_views;

  bool floating_tool_dialogs;

  int get_selected_layer_id();
  bool get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter);
  bool get_row(int id, Gtk::TreeModel::iterator& iter);
  void select_row(int id);

  void unset_sticky_and_editing( Layer* l );
  void unset_sticky_and_editing( std::list<Layer*>& layers );
  void detach_controls( Layer* l );
  void detach_controls( std::list<Layer*>& layers );
  //int get_map_tab( std::list<Layer*>* map_layers );
  //void close_map_tabs( Layer* l );

  Glib::Dispatcher signal_update;

public:
  sigc::signal<void,int> signal_edited_layer_changed;

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

  void add_tool( std::string tool_name );
  void add_layer( Layer* layer, bool do_update=true  );
  void insert_image( std::string filename );
  void insert_preset( std::string filename );
  void remove_layers();

  void switch_to_layers_view();
  void switch_to_mask_view();
  void toggle_mask();

  void on_map();

  void update( bool force_rebuild=false );

  void update_controls();

  void update_async()
  {
    signal_update.emit();
  }


  void controls_dialog_open(PF::Layer* l);
  void controls_dialog_show();
  void controls_dialog_hide();
  void controls_dialog_delete(PF::Layer* l);
  void controls_dialog_delete( std::list<PF::Layer*>& layers );

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
  OperationConfigGUI* get_selected_layer_ui();


  void on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
  void on_row_expanded( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path );
  void on_row_collapsed( const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path );

#ifdef GTKMM_3
  void on_switch_page(Widget* page, guint page_num);
#endif
#ifdef GTKMM_2
  void on_switch_page(_GtkNotebookPage* page, guint page_num);
#endif

  bool on_key_press_event(GdkEventKey* event);


  //void remove_tab( Gtk::Widget* widget );

  void modified() { /*if(image) image->modified();*/ }
};

}


#endif
