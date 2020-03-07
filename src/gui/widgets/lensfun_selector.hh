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

#ifndef LF_SELECTOR_HH
#define LF_SELECTOR_HH

#include <gtkmm.h>

#include "pfwidget.hh"

namespace PF {

class LFDbHelper {
public:
    class LFModelCam: public Gtk::TreeModel::ColumnRecord {
    public:
        LFModelCam() { add(make); add(model); }
        Gtk::TreeModelColumn<Glib::ustring> make;
        Gtk::TreeModelColumn<Glib::ustring> model;
    };

    class LFModelLens: public Gtk::TreeModel::ColumnRecord {
    public:
        LFModelLens() { add(lens); add(prettylens); }
        Gtk::TreeModelColumn<Glib::ustring> lens;
        Gtk::TreeModelColumn<Glib::ustring> prettylens;
    };

    LFModelCam lensfunModelCam;
    LFModelLens lensfunModelLens;

    Glib::RefPtr<Gtk::TreeStore> lensfunCameraModel;
    Glib::RefPtr<Gtk::TreeStore> lensfunLensModel;

    LFDbHelper();
    void fillLensfunCameras();
    void fillLensfunLenses();
};


/**
 * @brief subclass of Gtk::ComboBox with cutom width assignment
 */
class LFComboBox : public Gtk::ComboBox
{
    int naturalWidth, minimumWidth;

    void get_preferred_width_vfunc (int &minimum_width, int &natural_width) const;
    void get_preferred_width_for_height_vfunc (int height, int &minimum_width, int &natural_width) const;

public:
    LFComboBox ();

    void setPreferredWidth (int minimum_width, int natural_width);
    void set_width(int width, int child_width);
    void set_font_size(int size);
};


class LFCamSelector: public Gtk::HBox, public PFWidget
{

  Gtk::VBox vbox;
  Gtk::Label label;
  LFComboBox cbox;
  Glib::RefPtr<Gtk::ListStore> model;

public:
  LFCamSelector(OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width=100);
  LFCamSelector(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val, int width=100);

  ~LFCamSelector() {}

  virtual bool check_value( int id, const std::string& name, const std::string& val )
  {
    return true;
  }

  void get_value();
  void set_value();
};


class LFLensSelector: public Gtk::HBox, public PFWidget
{

  Gtk::VBox vbox;
  Gtk::Label label;
  LFComboBox cbox;
  Glib::RefPtr<Gtk::ListStore> model;

public:
  LFLensSelector(OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width=100);
  LFLensSelector(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val, int width=100);

  ~LFLensSelector() {}

  virtual bool check_value( int id, const std::string& name, const std::string& val )
  {
    return true;
  }

  void get_value();
  void set_value();
};


class LFSelector: public Gtk::HBox, public PFWidget
{
  std::string pname2;
  PF::PropertyBase* property2;
  std::string pname3;
  PF::PropertyBase* property3;
  Gtk::VBox vbox;
  Gtk::Label cam_label, lens_label;
  Gtk::Frame cam_frame, lens_frame;
  Gtk::EventBox cam_ebox, lens_ebox;
  Gtk::Menu cam_menu, lens_menu, lens_menu_full;
  Glib::ustring cam_maker_name, cam_model_name;
  Glib::ustring lens_maker_name, lens_name;

  Gtk::HBox cb_hbox;
  Gtk::Label cb_label;
  Gtk::CheckButton cb;

  bool enabled;

  void update_cam( Glib::ustring maker_new, Glib::ustring model_new );
  void update_lens( Glib::ustring lens_new );

public:
  LFSelector(OperationConfigGUI* dialog, std::string pname, std::string pname2, std::string pname3);

  ~LFSelector() {}

  void init();
  void enable() { enabled = true; }
  void disable() { enabled = false; }

  void fill_cam_menu();
  void fill_lens_menu();
  void fill_lens_menu_full();

  bool my_cam_button_release_event( GdkEventButton* button );
  bool my_lens_button_release_event( GdkEventButton* button );

  void on_cam_item_clicked(Glib::ustring make, Glib::ustring model);
  void on_lens_item_clicked(Glib::ustring make, Glib::ustring model);
  void set_cam(Glib::ustring cam_make, Glib::ustring cam_model);
  void set_lens(Glib::ustring lens_model);

  virtual bool check_value( int id, const std::string& name, const std::string& val )
  {
    return true;
  }

  void get_value();
  void set_value();
};


}

#endif
