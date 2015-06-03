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
#include "layerwidget.hh"
#include "tablabelwidget.hh"


namespace PF {

  class ImageEditor: public Gtk::Paned
  {
    std::string filename;
    Image* image;
    bool image_opened;

    Layer* active_layer;
    std::list<PF::Layer*> active_layer_children;

    Gtk::VBox imageBox;
    ImageArea* imageArea;
    Gtk::EventBox imageArea_eventBox;
    Gtk::ScrolledWindow imageArea_scrolledWindow;
    LayerWidget layersWidget;
    Gtk::HBox controlsBox;
    Gtk::Button buttonZoomIn, buttonZoomOut, buttonZoom100, buttonZoomFit;
    Gtk::VBox radioBox;
    Gtk::RadioButton buttonShowMerged, buttonShowActive;

    HTabLabelWidget* tab_label_widget;

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

    void set_active_layer( int id );

    void open_image();

    void on_image_modified();

    void on_map();
    void on_realize();

    // Handlers for the mouse events inside the image area
    bool on_button_press_event( GdkEventButton* button );
    bool on_button_release_event( GdkEventButton* button );
    bool on_motion_notify_event( GdkEventMotion* button );

		// Handler for the widget size change
		bool on_configure_event( GdkEventConfigure* event );

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

    void zoom_in();
    void zoom_out();
    void zoom_fit();
    void zoom_actual_size();
  };

}

#endif
