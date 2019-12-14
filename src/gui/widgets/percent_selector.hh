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

#ifndef PERCENT_SELECTOR_HH
#define PERCENT_SELECTOR_HH

#include <gtkmm.h>

#include "pfwidget.hh"

namespace PF {

  class PercentSelector: public Gtk::HBox, public PFWidget
  {
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
      
      ModelColumns()
      { add(col_name); add(col_id); add(col_value); }
      
      Gtk::TreeModelColumn<Glib::ustring> col_name;
      Gtk::TreeModelColumn<int> col_id;
      Gtk::TreeModelColumn<float> col_value;
    };

    ModelColumns columns;

    Gtk::VBox vbox;
    Gtk::Label label;
    Gtk::ComboBox cbox;
    Glib::RefPtr<Gtk::ListStore> model;
    int current_active_id;

  public:
    PercentSelector(OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width=100);
    PercentSelector(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val, int width=100);

    ~PercentSelector() {}

    void set_sensitive(bool s) { cbox.set_sensitive(s); }

    void set_row_separator_func( const Gtk::ComboBox::SlotRowSeparator& slot )
    {
      cbox.set_row_separator_func( slot );
    }

    virtual bool check_value( int id, const std::string& name, const std::string& val )
    {
      return true;
    }

    void on_value_changed();
    void on_entry_activated();
    bool on_entry_focus_out_event( GdkEventFocus* event );

    void get_value();
    void set_value();
  };


}

#endif
