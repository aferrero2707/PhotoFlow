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

#ifndef VSLIDER_HH
#define VSLIDER_HH

#include <gtkmm.h>

#include "pfwidget.hh"

namespace PF {

  class VSlider: public Gtk::VBox, public PFWidget
  {
    Gtk::HBox hbox;
    Gtk::Label label;
    Gtk::Alignment align;
#ifdef GTKMM_2
    Gtk::Adjustment adjustment;
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::Adjustment> adjustment;
#endif
    Gtk::VScale scale;
    Gtk::SpinButton spinButton;

    double multiplier;
    
  public:
    VSlider(OperationConfigDialog* dialog, std::string pname, std::string l,
	   double val, double min, double max, double sincr, double pincr, double mult);
    VSlider(OperationConfigDialog* dialog, ProcessorBase* processor, std::string pname, std::string l,
	   double val, double min, double max, double sincr, double pincr, double mult);

    ~VSlider() {}

#ifdef GTKMM_2
    Gtk::Adjustment* get_adjustment() { return( &adjustment ); }
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::Adjustment> get_adjustment() { return adjustment; }
#endif

    void set_editable( bool flag )
    {
      spinButton.set_editable( flag );
    }

    void get_value();
    void set_value();
  };


}

#endif
