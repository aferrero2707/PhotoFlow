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

#ifndef OPERATION_CONFIG_DIALOG_HH
#define OPERATION_CONFIG_DIALOG_HH

#include <gtkmm.h>

#include "../base/image.hh"

#include "widgets/textbox.hh"
#include "widgets/slider.hh"
#include "widgets/selector.hh"

namespace PF {

class OperationConfigDialog: public OperationConfigUI, public Gtk::Dialog
{
  std::vector<PFWidget*> controls;

#ifdef GTKMM_2
  Gtk::VBox mainBox;
  Gtk::HBox mainHBox;
  Gtk::HBox chselBox;
  Gtk::VBox topBox;
  Gtk::HBox nameBox;
  Gtk::HBox controlsBox;
  Gtk::VBox controlsBoxLeft;
  Gtk::VBox controlsBoxRight;
#endif
#ifdef GTKMM_3
  Gtk::Box mainBox;
  Gtk::Box mainHBox;
  Gtk::Box chselBox;
  Gtk::Box topBox;
  Gtk::Box nameBox;
  Gtk::Box controlsBox;
  Gtk::Box controlsBoxLeft;
  Gtk::Box controlsBoxRight;
#endif

  Gtk::Frame topFrame;

  Gtk::Entry nameEntry;
  Gtk::Label lname, lblendmode;
  Gtk::Label lopacity, lintensity;
  Gtk::Label controlsLabel;
  //Gtk::Alignment lintensityAl, lopacityAl;
  Gtk::ComboBoxText blendmodeCombo;

  //Gtk::Adjustment intensityAdj, opacityAdj;
  //Gtk::HScale intensityScale, opacityScale;

  Slider intensitySlider, opacitySlider;
  Selector blendSelector;
  Selector greychSelector, rgbchSelector, labchSelector, cmykchSelector;

  std::list<std::string> values_save;

  //virtual OpParBase* get_par() = 0;
  //virtual ProcessorBase* get_processor() = 0;
public:
  OperationConfigDialog(Layer* layer, const Glib::ustring& title);
  virtual ~OperationConfigDialog();

#ifdef GTKMM_2
  Gtk::VBox& get_main_box() { return mainBox; }
#endif
#ifdef GTKMM_3
  Gtk::Box& get_main_box() { return mainBox; }
#endif

  void add_widget( Gtk::Widget& widget );

  void add_control( PFWidget* control ) { controls.push_back( control ); }

  void on_button_clicked(int id);

  bool focus_in_cb(GdkEventFocus *focus)
  {
    on_focus_in( focus );
    return true;
  }
  virtual void on_focus_in(GdkEventFocus *focus) 
  {
    std::cout<<"OperationConfigDialog: on_focus_in() called."<<std::endl;
  }


  bool focus_out_cb(GdkEventFocus *focus)
  {
    on_focus_out( focus );
    return true;
  }
  virtual void on_focus_out(GdkEventFocus *focus) 
  {
    std::cout<<"OperationConfigDialog: on_focus_out() called."<<std::endl;
  }


  virtual void pointer_press_event( int button, double x, double y, int mod_key ) {}
  virtual void pointer_release_event( int button, double x, double y, int mod_key ) {}
  virtual void pointer_motion_event( int button, double x, double y, int mod_key ) {}

  //void on_intensity_value_changed();
  //void on_opacity_value_changed();

  void open();

  virtual void init();

  void update();

  void update_properties();
};



  ProcessorBase* new_operation_with_gui( std::string op_type, Layer* current_layer );
}

#endif
