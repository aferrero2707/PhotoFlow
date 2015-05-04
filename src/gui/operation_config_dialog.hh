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
#include "widgets/checkbox.hh"
#include "widgets/slider.hh"
#include "widgets/selector.hh"
#include "widgets/exposure_slider.hh"

#include "doublebuffer.hh"

namespace PF {

class ImageEditor;

class OperationConfigDialog: public OperationConfigUI, public Gtk::Dialog
{
  ProcessorBase* op;
  ProcessorBase* blender;

  ImageEditor* editor;

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

  Gtk::HBox shiftBox;

  Gtk::Frame topFrame;

  Gtk::Entry nameEntry;
  Gtk::Label lname, lblendmode;
  Gtk::Label lopacity, lintensity;
  Gtk::Label controlsLabel;
  //Gtk::Alignment lintensityAl, lopacityAl;
  Gtk::ComboBoxText blendmodeCombo;

  //Gtk::Adjustment intensityAdj, opacityAdj;
  //Gtk::HScale intensityScale, opacityScale;

  Selector blendSelector;
  Gtk::HBox intensity_box, opacity_box;
  Slider intensitySlider, opacitySlider;
  CheckBox imap_enabled_box, omap_enabled_box;
  Slider shift_x, shift_y;
  bool has_ch_sel;
  Selector greychSelector, rgbchSelector, labchSelector, cmykchSelector;

  Gtk::VBox previewBox;
  Gtk::CheckButton previewButton;

  std::list<std::string> values_save;

  void reset_ch_selector();

  //virtual OpParBase* get_par() = 0;
  //virtual ProcessorBase* get_processor() = 0;
public:
  OperationConfigDialog(Layer* layer, const Glib::ustring& title, bool has_ch_sel=true);
  virtual ~OperationConfigDialog();

  virtual bool has_preview() { return false; }

#ifdef GTKMM_2
  Gtk::VBox& get_main_box() { return mainBox; }
#endif
#ifdef GTKMM_3
  Gtk::Box& get_main_box() { return mainBox; }
#endif

  void set_editor( ImageEditor* e) { editor = e; }
  void add_widget( Gtk::Widget& widget );

  void add_control( PFWidget* control ) { controls.push_back( control ); }

  void on_button_clicked(int id);

  void on_preview_clicked();

  bool focus_in_cb(GdkEventFocus *focus)
  {
    on_focus_in( focus );
    return true;
  }
  virtual void on_focus_in(GdkEventFocus *focus) 
  {
    //std::cout<<"OperationConfigDialog: on_focus_in() called."<<std::endl;
  }


  bool focus_out_cb(GdkEventFocus *focus)
  {
    on_focus_out( focus );
    return true;
  }
  virtual void on_focus_out(GdkEventFocus *focus) 
  {
    //std::cout<<"OperationConfigDialog: on_focus_out() called."<<std::endl;
  }


  void on_map();
  void on_unmap();


  virtual void enable_editing();
  virtual void disable_editing();


  void screen2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void screen2image( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    screen2image( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }
  void image2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void image2layer( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    image2layer( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }
  void screen2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void screen2layer( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    screen2layer( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }
  void image2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void image2screen( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    image2screen( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }
  void layer2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void layer2image( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    layer2image( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }
  void layer2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void layer2screen( VipsRect& r )
  {
    double x = r.left; double y = r.top; double w = r.width; double h = r.height;
    layer2screen( x, y, w, h );
    r.left = x; r.top = y; r.width = w; r.height = h;
  }

  virtual bool pointer_press_event( int button, double x, double y, int mod_key ) { return false; }
  virtual bool pointer_release_event( int button, double x, double y, int mod_key ) { return false; }
  virtual bool pointer_motion_event( int button, double x, double y, int mod_key ) { return false; }

  //void on_intensity_value_changed();
  //void on_opacity_value_changed();

  virtual bool modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                               float scale, int xoffset, int yoffset ) { return false; }

  void open();

  void init();

  void update();

  void do_update();

  void update_properties();
};



  ProcessorBase* new_operation_with_gui( std::string op_type, Layer* current_layer );
}

#endif
