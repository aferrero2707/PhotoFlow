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

#ifndef CHECKBOX_HH
#define CHECKBOX_HH

#include <gtkmm.h>

#include "pfwidget.hh"

namespace PF {

  class CheckBox: public Gtk::HBox, public PFWidget
  {
    Gtk::Label label;
    Gtk::CheckButton check;

  public:
    CheckBox(OperationConfigGUI* dialog, std::string pname, std::string l, int val, int orientation=0);
    CheckBox(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l, int val, int orientation = 0);

    ~CheckBox() {}

    void get_value();
    void set_value();

    bool get_active() { return check.get_active(); }
  };


}

#endif
