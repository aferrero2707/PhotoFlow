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

#ifndef SLIDER_HH
#define SLIDER_HH

#include <gtkmm.h>

#include "numentry.hh"

namespace PF {

  class Slider: public Gtk::HBox, public PFWidget
  {
    Gtk::HBox hbox, hbox2;
    Gtk::VBox vbox, vbox2;
    Gtk::Label label;
    Gtk::Alignment align;
    Gtk::Alignment label_align;
    Gtk::Alignment scale_align;
    Gtk::Alignment numentry_align;
    Gtk::Alignment reset_align;
#ifdef GTKMM_2
    Gtk::Adjustment adjustment;
#endif
#ifdef GTKMM_3
    Glib::RefPtr<Gtk::Adjustment> adjustment;
#endif
    Gtk::HScale scale;
    Gtk::SpinButton spinButton;
    NumEntry numentry;

    Gtk::Alignment reset_button_align;
    ImageButton reset_button;

    double multiplier;
    double (*fun_slider_to_prop)(double&, OperationConfigGUI*, void*);
    double (*fun_prop_to_slider)(double&, OperationConfigGUI*, void*);
    void* user_data;
    
    void create_widgets( std::string l, double val,
        double min, double max,
        double sincr, double pincr, int size, int layout );

  public:
    Slider(OperationConfigGUI* dialog, std::string pname, std::string l,
	   double val, double min, double max, double sincr, double pincr, double mult, int size=120, int layout=2);
    Slider(OperationConfigGUI* dialog, ProcessorBase* processor, std::string pname, std::string l,
	   double val, double min, double max, double sincr, double pincr, double mult, int size=120, int layout=2);

    ~Slider();

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

    void set_width( int w )
    {
      scale.set_size_request( w, -1 );
    }

    void set_conversion_functions(double (*f1)(double&, OperationConfigGUI*, void*),
        double (*f2)(double&, OperationConfigGUI*, void*), void* ud=NULL)
    {
      fun_slider_to_prop = f1;
      fun_prop_to_slider = f2;
      user_data = ud;
    }

    void get_value();
    void set_value();

    void update_gui()
    {
      while (gtk_events_pending())
        gtk_main_iteration();
    }
  };


}

#endif
