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

#ifndef LAYER_LIST_HH
#define LAYER_LIST_HH

#include <gtkmm.h>

#include "pfwidget.hh"

namespace PF {

  class LayerList: public Gtk::VBox
  {
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
      
      Gtk::TreeModelColumn<std::string> col_name;
      Gtk::TreeModelColumn<Layer*> col_layer;
      
      ModelColumns()
      { add(col_name); add(col_layer); }
    };

    ModelColumns columns;

    OperationConfigUI* dialog;

    Gtk::VBox vbox;
    Gtk::Label label;
    Gtk::ComboBox cbox;
    Glib::RefPtr<Gtk::ListStore> model;

    bool inhibit;

  public:
    LayerList(OperationConfigUI* dialog, std::string label);

    ~LayerList() {}

    void update_model();

    void changed();
  };


}

#endif
