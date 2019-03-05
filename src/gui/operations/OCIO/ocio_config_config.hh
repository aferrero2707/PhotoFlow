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

#ifndef OCIO_CONFIG_CONFIG_DIALOG_HH
#define OCIO_CONFIG_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../../operation_config_gui.hh"

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;




namespace PF {

class OCIOCSSelector: public Gtk::HBox, public PFWidget
{
  OCIO::ConstConfigRcPtr config;
  Glib::ustring csname;
  std::string pname;
  PF::PropertyBase* property;
  Gtk::VBox vbox;
  Gtk::Label label;
  Gtk::Frame frame;
  Gtk::EventBox ebox;
  Gtk::Menu* menu;

  bool enabled;

  void set_csname(Glib::ustring csname_);
  void update( Glib::ustring csname_new );

public:
  sigc::signal<void> signal_cs_changed;

  OCIOCSSelector(OperationConfigGUI* dialog, std::string pname);

  ~OCIOCSSelector() {}

  void set_config(OCIO::ConstConfigRcPtr config);

  void init();
  void enable() { enabled = true; }
  void disable() { enabled = false; }

  std::string get_colorspace() { return label.get_text(); }

  void fill_menu();

  bool my_button_release_event( GdkEventButton* button );

  void on_item_clicked(Glib::ustring csname);

  virtual bool check_value( int id, const std::string& name, const std::string& val )
  {
    return true;
  }

  void get_value();
  void set_value();
};




class OCIOLookSelector: public Gtk::HBox, public PFWidget
{
  OCIO::ConstConfigRcPtr config;
  std::string colorspace;
  Glib::ustring lookname;
  std::string pname;
  PF::PropertyBase* property;
  Gtk::VBox vbox;
  Gtk::Label label;
  Gtk::Frame frame;
  Gtk::EventBox ebox;
  Gtk::Menu* menu;

  bool enabled;

  void set_lookname(Glib::ustring lookname_);
  void update( Glib::ustring lookname_new );

public:
  sigc::signal<void> signal_look_changed;

  OCIOLookSelector(OperationConfigGUI* dialog, std::string pname);

  ~OCIOLookSelector() {}

  void set_config(OCIO::ConstConfigRcPtr config);
  void set_colorspace(std::string cs);

  void init();
  void enable() { enabled = true; }
  void disable() { enabled = false; }

  void fill_menu();

  bool my_button_release_event( GdkEventButton* button );

  void on_item_clicked(Glib::ustring lookname);

  virtual bool check_value( int id, const std::string& name, const std::string& val )
  {
    return true;
  }

  void get_value();
  void set_value();
};




class OCIOConfigConfigGUI: public OperationConfigGUI
{
  Gtk::VBox controlsBox;

  Gtk::HBox file_hbox;
  Gtk::Image img_open;
  Gtk::Label label;
  Gtk::Entry fileEntry;
  Gtk::Button openButton;

  Gtk::Frame csin_frame;
  Gtk::VBox csin_vbox;
  Gtk::Label csin_label;
  OCIOCSSelector csin_selector;
  Selector in_prof_mode_selector;
  Gtk::HBox in_prof_mode_selector_box;
  Selector in_trc_type_selector;
  Gtk::HBox in_trc_type_selector_box;


  Gtk::Frame csout_frame;
  Gtk::VBox csout_vbox;
  Gtk::Label csout_label;
  OCIOCSSelector csout_selector;
  Selector out_prof_mode_selector;
  Gtk::HBox out_prof_mode_selector_box;
  Selector out_trc_type_selector;
  Gtk::HBox out_trc_type_selector_box;

  Gtk::Frame look_frame;
  Gtk::VBox look_vbox;
  OCIOLookSelector look_selector;

public:
  OCIOConfigConfigGUI( Layer* l );
  void open();
  void on_button_open_clicked();
  void on_filename_changed();
  void on_colorspace_changed();
};

}

#endif
