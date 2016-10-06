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

#define OMAP_COL_NUM 2
#define IMAP_COL_NUM 3

namespace PF {

class ImageEditor;
class LayerWidget;

  class LayerTreeModel: public Gtk::TreeStore
  {
  protected:
    LayerTreeModel();

  public:
    // Definition of the calumns in the layer list
    class LayerTreeColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
      
      LayerTreeColumns()
      { add(col_visible); add(col_name); add(col_omap); add(col_imap); add(col_layer); }
      
      Gtk::TreeModelColumn<bool> col_visible;
      Gtk::TreeModelColumn<Glib::ustring> col_name;
      Gtk::TreeModelColumn<Layer*> col_layer;
      Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > col_imap;
      Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > col_omap;
    };


    LayerTreeColumns columns;

    sigc::signal<void> signal_dnd_done;

    //PF::Layer* src_layer;
    //PF::Layer* dest_layer;
    //PF::Layer* group_layer;

    static Glib::RefPtr<LayerTreeModel> create();

  protected:
    Layer* get_dest_layer(const Gtk::TreeModel::Path& dest,
                          bool& drop_into) const;
    Layer* get_parent_layer(const Gtk::TreeModel::Path& dest) const;
    virtual bool row_draggable_vfunc( const Gtk::TreeModel::Path& path ) const;
    virtual bool row_drop_possible_vfunc( const Gtk::TreeModel::Path& dest,
                                          const Gtk::SelectionData& selection_data) const;
    virtual bool drag_data_received_vfunc( const Gtk::TreeModel::Path& dest,
                                           const Gtk::SelectionData& selection_data);
  };



  class LayersTreeView: public Gtk::TreeView
  {
    Gtk::Menu popupMenu;
    LayerWidget* layer_widget;
  public:
    LayersTreeView(LayerWidget* layer_widget);
    bool on_button_press_event(GdkEventButton* button_event) override;
    void on_menu_cut();
    void on_menu_copy();
    void on_menu_paste();
  };



  class LayerTree : public Gtk::ScrolledWindow
  {
    // Tree model to be filled with individial layers informations
    Glib::RefPtr<PF::LayerTreeModel> treeModel;

    LayersTreeView treeView;

    ImageEditor* editor;

    //Image* image;
    //LayerManager* layer_manager;
    std::list<Layer*>* layers;

    bool map_flag;

    void update_mask_icons( Gtk::TreeModel::Row row,  PF::Layer* l );
    void update_model(Gtk::TreeModel::Row parent_row);

    bool get_row(int id, const Gtk::TreeModel::Children& rows, Gtk::TreeModel::iterator& iter);
    bool get_row(int id, Gtk::TreeModel::iterator& iter);

  public:
    sigc::signal<void> signal_updated;

    LayerTree( ImageEditor* editor, bool is_map=false );
    virtual ~LayerTree();

    Glib::RefPtr<Gtk::TreeStore> get_model() { return treeModel; }
    LayerTreeModel::LayerTreeColumns& get_columns() { return treeModel->columns; }

    Gtk::TreeView& get_tree() { return treeView; }

    PF::Layer* get_selected_layer();
    int get_selected_layer_id();
    void unselect_all()
    {
      get_tree().get_selection()->unselect_all();
    }
    void select_row(int id);

    bool is_map() { return map_flag; }

    //Image* get_image() { return image; }
    //void set_image(Image* img) { image = img; update_model(); }

    void on_cell_toggled(const Glib::ustring& path);

    // Updates the tree model with the layers from the associated image
    void update_model();
    void update_model_cb() { update_model(); }
    void update_model_idle_cb();

    std::list<Layer*>* get_layers() { return layers; }
    void set_layers( std::list<Layer*>* l ) {
      layers = l;
      update_model();
    }

  };

}


#endif
