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
    virtual ~PFWidget() {}

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


}

#endif
