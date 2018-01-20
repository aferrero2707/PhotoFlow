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


}

#endif
