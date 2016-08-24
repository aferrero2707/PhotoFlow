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

#ifndef IMAGE_EDITOR_H
#define IMAGE_EDITOR_H

#include <string>

#include <gtkmm.h>

#include "../base/photoflow.hh"

#include "imagearea.hh"
#include "histogram.hh"
#include "layerwidget.hh"
#include "tablabelwidget.hh"
#include "widgets/statusindicator.hh"


namespace PF {


class PreviewScrolledWindow: public Gtk::ScrolledWindow
{
public:
  void on_map();
  bool on_configure_event(GdkEventConfigure*event);
};



// The image_size_updater is a special pipeline sink that stores
// the dimensions of the full-res image which corresponds to
// the output being displayed in the preview area.
// It is used to compute the correct scaling factor when
// the preview image is shown in zoom-to-fit mode.
class ImageSizeUpdater: public PipelineSink
{
  int displayed_layer_id;

  VipsImage* image;
  int image_width, image_height;
public:
  ImageSizeUpdater( Pipeline* p );

  int get_displayed_layer() { return displayed_layer_id; }
  void set_displayed_layer( int id ) { displayed_layer_id = id; }

  VipsImage* get_image() { return image; }
  int get_image_width() { return image_width; }
  int get_image_height() { return image_height; }

  void update( VipsRect* area );
};



class ImageEditor: public Gtk::HBox
{
  std::string filename;
  Image* image;
  bool image_opened;

  Layer* displayed_layer;
  Layer* active_layer;
  std::list<PF::Layer*> active_layer_children;

  ImageSizeUpdater* image_size_updater;

  Gtk::VBox imageBox;
  ImageArea* imageArea;
  Gtk::EventBox imageArea_eventBox;

  Gtk::Expander hist_expander;
  Histogram* histogram;

  // Boxes for aligning the image area inside the scrolled window.
  // The image area gets inserted into an HBox which in turn
  // is inserted into a VBox, in both cases with PACK_EXPAND_PADDING.
  // The VBox is the inserted into an EventBox which provides
  // the black background.
  Gtk::VBox imageArea_vbox;
  Gtk::HBox imageArea_hbox;
  Gtk::EventBox imageArea_eventBox2;
  PreviewScrolledWindow imageArea_scrolledWindow;

  Gtk::VBox imageArea_scrolledWindow_box;
  //Gtk::HPaned main_panel;
  Gtk::HBox main_panel;
  LayerWidget layersWidget;
  Gtk::VBox layersWidget_box;
  Gtk::Widget* aux_controls;
  Gtk::VBox aux_controlsBox;
  Gtk::HBox controlsBox;

  Gtk::Frame soft_proof_frame;
  Gtk::VBox soft_proof_box;
  Gtk::CheckButton sim_black_ink_button;
  Gtk::CheckButton sim_paper_color_button;

  StatusIndicatorWidget status_indicator;
  Gtk::Image  img_zoom_in, img_zoom_out, img_zoom_fit;
  Gtk::Button buttonZoomIn, buttonZoomOut, buttonZoom100, buttonZoomFit;
  Gtk::Image img_shadows_warning, img_highlights_warning;
  Gtk::ToggleButton button_shadows_warning, button_highlights_warning;
  Gtk::VBox radioBox;
  Gtk::RadioButton buttonShowMerged, buttonShowActive;
  Gtk::ScrolledWindow controls_group_scrolled_window;

  HTabLabelWidget* tab_label_widget;


  bool fit_image;
  bool fit_image_needed;

  bool hide_background_layer;

  int preview_drag_start_x, preview_drag_start_y, adjustment_drag_start_x, adjustment_drag_start_y;

  void expand_layer( PF::Layer* layer, std::list<PF::Layer*>& list );
  void get_child_layers( Layer* layer, std::list<PF::Layer*>& container,
      std::list<Layer*>& children );
  void get_child_layers();

public:
  ImageEditor( std::string filename );
  ~ImageEditor();

  Image* get_image() { return image; }

  LayerWidget& get_layer_widget() { return layersWidget; }

  void set_tab_label_widget( HTabLabelWidget* l ) { tab_label_widget = l; }

  void update_controls();
  void set_aux_controls( Gtk::Widget* aux );
  Gtk::Widget* get_aux_controls() { return aux_controls; }

  int get_active_layer() {
    //std::cout<<"ImageEditor::get_active_layer(): active_layer="<<active_layer;
    //if(active_layer) std::cout<<"(\""<<active_layer->get_name()<<"\", "<<active_layer->get_id()<<")"<<std::endl;
    return( (active_layer) ? active_layer->get_id() : -1 );
  }
  void set_active_layer( int id );
  int get_displayed_layer() { (displayed_layer) ? displayed_layer->get_id() : -1; }
  void set_displayed_layer( int id );

  void set_hide_background_layer( bool flag ) { hide_background_layer = flag; }
  bool get_hide_background_layer() { return hide_background_layer; }

  void open_image();
  void build_image();

  void on_image_modified();

  void on_map();
  void on_realize();

  void set_status( std::string label, int status )
  {
    status_indicator.set_status( label, status );
  }
  void set_status_ready() { set_status(_("ready"), 0); }
  void set_status_caching() { set_status(_("caching"), 1); }
  void set_status_processing() { set_status(_("processing"), 2); }
  void set_status_exporting() { set_status(_("exporting"), 2); }

  // Handlers for the mouse events inside the image area
  bool my_button_press_event( GdkEventButton* button );
  bool my_button_release_event( GdkEventButton* button );
  bool my_motion_notify_event( GdkEventMotion* button );

  // Handler for the widget size change
  //bool on_preview_configure_event( GdkEventConfigure* event );
  void on_my_size_allocate(Gtk::Allocation& allocation);

  float get_zoom_factor()
  {
    PF::Pipeline* pipeline = image->get_pipeline(1);
    if( !pipeline ) return 1.0f;
    int level = pipeline->get_level();
    float fact = 1.0f;
    for( unsigned int i = 0; i < level; i++ )
      fact /= 2.0f;
#ifndef NDEBUG
    std::cout<<"get_zoom_factor(): level="<<level<<"  fact="<<fact<<std::endl;
#endif
    return fact;
  }

  void screen2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void image2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void screen2layer( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
  {
    screen2image( x, y, w, h );
    image2layer( x, y, w, h );
  }

  void image2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void layer2image( gdouble& x, gdouble& y, gdouble& w, gdouble& h );
  void layer2screen( gdouble& x, gdouble& y, gdouble& w, gdouble& h )
  {
    layer2image( x, y, w, h );
    image2screen( x, y, w, h );
  }

  void toggle_highlights_warning();
  void toggle_shadows_warning();

  void on_sim_black_ink_toggled();
  void on_sim_paper_color_toggled();

  void zoom_in();
  void zoom_out();
  bool zoom_fit();
  void zoom_actual_size();
};

}

#endif
