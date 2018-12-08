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

#ifndef PF_WIDGET_HH
#define PF_WIDGET_HH

#include "../../base/image.hh"

namespace PF {

  class OperationConfigGUI;

  class PFWidget
  {
    bool inhibit;
    bool passive;
    OperationConfigGUI* dialog;
    ProcessorBase* processor;
    std::string pname;
    PropertyBase* property;

  public:
    PFWidget(OperationConfigGUI* d, std::string n);
    PFWidget(OperationConfigGUI* d, ProcessorBase* p, std::string n);

    sigc::signal<void> value_changed;

    bool get_inhibit() { return inhibit; }
    void set_inhibit( bool val ) { inhibit = val; }
    void set_passive( bool val ) { passive = val; }

    virtual void reset() { set_inhibit(true); if(property) {property->reset(); get_value();} set_inhibit(false); }

    OperationConfigGUI* get_dialog() { return dialog; }

    void set_processor( ProcessorBase* p) { processor = p; }
    ProcessorBase* get_processor() { return processor; }

    std::string get_prop_name() { return pname; }
    PropertyBase* get_prop() { return property; }

    virtual void init();

    virtual void get_value() = 0;
    virtual void set_value() = 0;

    void changed();
 };




  template <typename T>
  class SelectorWidget: public Gtk::HBox
  {
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
      Gtk::TreeModelColumn<T> col_value;
      Gtk::TreeModelColumn<Glib::ustring> col_text;

      ModelColumns()
      { add(col_value); add(col_text); }
    };
    ModelColumns columns;
    Glib::RefPtr<Gtk::ListStore> model;
    Gtk::Label label;
    Gtk::ComboBox cbox;

    public:
    SelectorWidget(Glib::ustring l): label(l)
    {
      model = Gtk::ListStore::create(columns);
      cbox.set_model( model );
      cbox.pack_start(columns.col_text);
      pack_start( label, Gtk::PACK_SHRINK, 2 );
      pack_start( cbox, Gtk::PACK_SHRINK, 2 );
    }

    Gtk::ComboBox& get_cbox() {return cbox;}

    void reset()
    {
      model = Gtk::ListStore::create(columns);
      cbox.set_model( model );
    }

    void add_entry(Glib::ustring t, T v)
    {
      Gtk::TreeModel::iterator ri = model->append();
      Gtk::TreeModel::Row row = *(ri);
      row[columns.col_value] = v;
      row[columns.col_text] = t;
    }

    void set_active(T value)
    {
      Glib::RefPtr<Gtk::TreeModel> model = cbox.get_model();
      Gtk::TreeModel::Children rows = model->children();
      for(unsigned int i = 0; i < rows.size(); i++) {
        Gtk::TreeModel::Row row = rows[i];
        T lvalue = row[columns.col_value];
        if( lvalue == value ) {
          cbox.set_active(i);
          break;
        }
      }
    }

    void set_active_text(Glib::ustring value)
    {
      Glib::RefPtr<Gtk::TreeModel> model = cbox.get_model();
      Gtk::TreeModel::Children rows = model->children();
      for(unsigned int i = 0; i < rows.size(); i++) {
        Gtk::TreeModel::Row row = rows[i];
        Glib::ustring lvalue = row[columns.col_text];
        if( lvalue == value ) {
          cbox.set_active(i);
          break;
        }
      }
    }

    void set_active_id(unsigned int id)
    {
      Glib::RefPtr<Gtk::TreeModel> model = cbox.get_model();
      Gtk::TreeModel::Children rows = model->children();
      if(id < rows.size()) cbox.set_active(id);
    }

    T get_active()
    {
      Gtk::TreeModel::iterator iter = cbox.get_active();
      if( iter ) {
        Gtk::TreeModel::Row row = *iter;
        if( row ) {
          return( row[columns.col_value] );
        }
      }
      return T();
    }

    Glib::ustring get_active_text()
    {
      Glib::ustring result;
      Gtk::TreeModel::iterator iter = cbox.get_active();
      if( iter ) {
        Gtk::TreeModel::Row row = *iter;
        if( row ) {
          result = row[columns.col_text];
        }
      }
      return(result);
    }

  };


  typedef SelectorWidget<std::string> TextSelector;
  typedef SelectorWidget<int> IntSelector;
}

#endif
