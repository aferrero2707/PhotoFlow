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

#ifndef OPERATION_CONFIG_UIGTK_HH
#define OPERATION_CONFIG_UIGTK_HH

#include <gtkmm.h>

#include "../base/layer.hh"
#include "../base/image.hh"

#include "widgets/textbox.hh"
#include "widgets/checkbox.hh"
#include "widgets/slider.hh"
#include "widgets/selector.hh"
#include "widgets/exposure_slider.hh"
#include "widgets/imagebutton.hh"
#include "widgets/layerlist.hh"

#include "doublebuffer.hh"

namespace PF {

class ImageEditor;



class OperationConfigGUI: public OperationConfigUI
{
  ProcessorBase* op;
  ProcessorBase* blender;

  ImageEditor* editor;

  std::vector<PFWidget*> controls;
  std::list<std::string> values_save;
  std::list< std::list<std::string> > undo_history;
  std::list< std::list<std::string> > redo_history;

  Gtk::Label lname, lblendmode;
  Gtk::Label lopacity, lintensity;
  Gtk::Label controlsLabel;
  Gtk::ComboBoxText blendmodeCombo;

  Selector blendSelector, blendSelector2, blendSelectorMask, blendSelectorMask2;
  Gtk::HBox intensity_box, opacity_box;
  Slider intensitySlider, intensitySlider2, opacitySlider, opacitySlider2;
  CheckBox imap_enabled_box, omap_enabled_box, test_padding_enable_box;
  Slider shift_x, shift_y;
  bool has_ch_sel;
  Selector greychSelector, rgbchSelector, labchSelector, cmykchSelector;

  Gtk::Expander input_source_expander;
  CheckBox input_source_checkbox;
  Gtk::VBox layer_selector_box;
  LayerList layer_list;
  Selector sourceSelector;


  Gtk::CheckButton previewButton;

  Gtk::Dialog* dialog;

  // Embedded controls
  Gtk::Frame* frame;
  Gtk::Alignment frame_box_1_padding;
  Gtk::Alignment frame_top_buttons_alignment;
  Gtk::Alignment frame_box_2_padding;
  Gtk::Alignment frame_box_3_padding;
  Gtk::Alignment frame_box_4_padding;
  Gtk::Alignment middle_padding;
  Gtk::HBox frame_hbox;
  Gtk::VBox frame_vbox;
  Gtk::VBox frame_shift_box;
  Gtk::VBox frame_chsel_box;
  Gtk::HBox frame_top_buttons_hbox;
  Gtk::HBox frame_top_buttons_box2;
  Gtk::HBox frame_top_box_1_1;
  Gtk::HBox frame_top_box_1_2;
  Gtk::VBox frame_top_vbox_1;
  Gtk::HBox frame_top_box_1;
  Gtk::HBox frame_top_box_2;
  Gtk::VBox frame_top_box_3;
  Gtk::HBox frame_top_box_4;
  ToggleImageButton frame_visible;
  //ToggleImageButton frame_preview;
  ToggleImageButton frame_mask;
  ToggleImageButton frame_mask2;
  ToggleImageButton frame_edit;
  ToggleImageButton frame_edit2;
  ToggleImageButton frame_sticky;
  ToggleImageButton frame_sticky2;
  ToggleImageButton frame_undo;
  ToggleImageButton frame_redo;
  ToggleImageButton frame_reset;
  ImageButton frame_help;
  ImageButton frame_help2;
  ToggleImageButton frame_close;
  ToggleImageButton frame_expander;
  Gtk::Entry nameEntry, nameEntry2;
  Gtk::Frame controls_frame;
  Gtk::EventBox controls_evbox;
  Gtk::VBox controls_box;
  Gtk::HSeparator hline, hline2;

  Gtk::Expander expert_ctrls_expander;

  //ToggleImageButtonsBox map_buttons;

  Gtk::HBox aux_top_buttons_hbox;
  Gtk::HBox aux_opacity_box;
  Gtk::VBox aux_controls_box;
  Gtk::HBox aux_controls_hbox_1, aux_controls_hbox_2;
  Gtk::Alignment aux_top_buttons_alignment;

  VipsSemaphore update_done_sem;


  OpParBase* get_blender();

  void show_layer_cb() { show_layer(); }
  void hide_layer_cb() { hide_layer(); }
  void enable_masks_cb() { enable_masks(); }
  void disable_masks_cb() { disable_masks(); }
  void enable_editing_cb() { enable_editing(); }
  void disable_editing_cb() { disable_editing(); }
  void set_sticky_cb() { set_sticky(); }
  void unset_sticky_cb() { unset_sticky(); }
  void parameters_undo_cb() { parameters_undo(); }
  void parameters_redo_cb() { parameters_redo(); }
  void parameters_reset_cb() { parameters_reset(); }
  void show_help_cb() { show_help(); }
  void close_config_cb() { close_config(); }

  void reset_ch_selector();

public:
  OperationConfigGUI( Layer* layer, const Glib::ustring& title, bool has_ch_sel=true );
  virtual ~OperationConfigGUI();

  virtual bool has_preview() { return false; }

  OpParBase* get_par();

  Gtk::VBox& get_main_box() { return controls_box; }
  Gtk::Frame* get_frame() { return frame; }

  void set_editor( ImageEditor* e) { editor = e; }
  void add_widget( Gtk::Widget& widget );

  void add_control( PFWidget* control ) { controls.push_back( control ); }

  Gtk::VBox& get_aux_controls() { return aux_controls_box; }

  void on_map();
  void on_unmap();

  void expand();
  void collapse();
  bool is_expanded();

  void enable_preview();
  void disable_preview();

  bool get_editing_flag();
  virtual void show_layer();
  virtual void hide_layer();
  virtual void enable_masks();
  virtual void disable_masks();
  virtual void enable_editing();
  void reset_edit_button();
  virtual void disable_editing();
  bool is_editing() { return frame_edit.is_active(); }
  virtual void set_sticky();
  void reset_sticky_button();
  virtual void unset_sticky();
  bool is_sticky() { return frame_sticky.is_active(); }
  virtual void parameters_undo();
  virtual void parameters_redo();
  virtual void parameters_reset();
  virtual void show_help();
  virtual void close_config();

  virtual bool has_editing_mode() { return false; }

  void on_layer_name_changed();
  void on_layer_name2_changed();


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

  void update_buttons();
  void open();
  void init();
  void update();
  virtual void do_update();
  void update_notify() {
    vips_semaphore_up( &update_done_sem );
  }

  void update_properties();
};



ProcessorBase* new_operation_with_gui( std::string op_type, Layer* current_layer );
}

#endif
