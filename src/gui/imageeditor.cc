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

#include <gdk/gdk.h>

#include "../base/imageprocessor.hh"
#include "imageeditor.hh"


#define PIPELINE_ID 1


PF::ImageEditor::ImageEditor( std::string fname ):
  filename( fname ),
  image( new PF::Image() ),
  image_opened( false ),
  active_layer( NULL ),
  //imageArea( image->get_pipeline(PIPELINE_ID) ),
  layersWidget( image ),
  buttonZoomIn( "Zoom +" ),
  buttonZoomOut( "Zoom -" ),
  buttonZoom100( "1:1" ),
  buttonZoomFit( "Fit" ),
  buttonShowMerged( "show merged layers" ),
  buttonShowActive( "show active layer" )
{
	imageArea = new PF::ImageArea( image->get_pipeline(PIPELINE_ID) );

  imageArea->set_adjustments( imageArea_scrolledWindow.get_hadjustment(),
			     imageArea_scrolledWindow.get_vadjustment() );

  imageArea_scrolledWindow.add( *imageArea );
  imageArea_eventBox.add( imageArea_scrolledWindow );

  radioBox.pack_start( buttonShowMerged );
  radioBox.pack_start( buttonShowActive );

  Gtk::RadioButton::Group group = buttonShowMerged.get_group();
  buttonShowActive.set_group(group);

  controlsBox.pack_end( radioBox, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoom100, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomFit, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomOut, Gtk::PACK_SHRINK );
  controlsBox.pack_end( buttonZoomIn, Gtk::PACK_SHRINK );

  imageBox.pack_start( imageArea_eventBox );
  imageBox.pack_start( controlsBox, Gtk::PACK_SHRINK );

  pack1( imageBox, true, false );

  pack2( layersWidget, false, false );

  buttonZoomIn.signal_clicked().connect( sigc::mem_fun(*this,
						       &PF::ImageEditor::zoom_in) );
  buttonZoomOut.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_out) );
  buttonZoom100.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_actual_size) );
  buttonZoomFit.signal_clicked().connect( sigc::mem_fun(*this,
							&PF::ImageEditor::zoom_fit) );

  buttonShowMerged.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged),
							 true) );
  buttonShowActive.signal_clicked().connect( sigc::bind( sigc::mem_fun(imageArea,
								       &PF::ImageArea::set_display_merged),
							 false) );
  //set_position( get_allocation().get_width()-200 );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(imageArea,
								  &PF::ImageArea::set_active_layer) );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(this,
								  &PF::ImageEditor::set_active_layer) );

  /*
  imageArea_eventBox.signal_button_press_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_button_press_event) ); 
  imageArea_eventBox.signal_button_release_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_button_release_event) ); 
  imageArea_eventBox.signal_motion_notify_event().
    connect( sigc::mem_fun(*this, &PF::ImageEditor::on_motion_notify_event) ); 
  */

  imageArea->add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK  | Gdk::POINTER_MOTION_HINT_MASK | Gdk::STRUCTURE_MASK );
  imageArea_scrolledWindow.add_events( Gdk::STRUCTURE_MASK );
	imageArea_scrolledWindow.signal_configure_event().
		connect( sigc::mem_fun(*this, &PF::ImageEditor::on_configure_event) ); 
  //add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
	//add_events( Gdk::STRUCTURE_MASK );

  //open_image();

  show_all_children();
}


PF::ImageEditor::~ImageEditor()
{
	/*
  if( image )
    delete image;
	*/
  ProcessRequestInfo request;
  request.image = image;
  request.request = PF::IMAGE_DESTROY;
  PF::ImageProcessor::Instance().submit_request( request );	
}



void PF::ImageEditor::open_image()
{
  if( image_opened ) return;
  std::cout<<"ImageEditor::open_image(): opening image..."<<std::endl;
  image->open( filename );
  std::cout<<"ImageEditor::open_image(): ... done."<<std::endl;
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
  int level = 0;
  pipeline->set_level( level );
	imageArea->set_shrink_factor( 1 );
  layersWidget.update();
  std::cout<<"ImageEditor::open_image(): updating image"<<std::endl;
  image->set_loaded( false );
  image->update();
  //getchar();
  //PF::ImageProcessor::Instance().wait_for_caching();
  image->set_loaded( true );
  image_opened = true;
  //Gtk::Paned::on_map();
}


void PF::ImageEditor::on_map()
{
  std::cout<<"ImageEditor::on_map() called."<<std::endl;
  //open_image();
  Gtk::Paned::on_map();
}

void PF::ImageEditor::on_realize()
{
  std::cout<<"ImageEditor::on_realize() called."<<std::endl;
  open_image();
  Gtk::Paned::on_realize();
}

void PF::ImageEditor::zoom_out()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  pipeline->set_level( level + 1 );
	imageArea->set_shrink_factor( 1 );
  image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_out(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
}


void PF::ImageEditor::zoom_in()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
  int level = pipeline->get_level();
  if( level > 0 ) {
    pipeline->set_level( level - 1 );
		imageArea->set_shrink_factor( 1 );
    image->update();
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


void PF::ImageEditor::zoom_fit()
{
  if( !image ) return;
  image->lock();
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  PF::Pipeline* pipeline2 = image->get_pipeline( PIPELINE_ID );
  if( !pipeline || !pipeline2) {
    image->unlock();
    return;
  }
  VipsImage* out = pipeline->get_output();
  if( !out ) {
    image->unlock();
    return;
  }

	float shrink_h = ((float)imageArea_scrolledWindow.get_hadjustment()->get_page_size())/out->Xsize;
	float shrink_v = ((float)imageArea_scrolledWindow.get_vadjustment()->get_page_size())/out->Ysize;
	float shrink_min = (shrink_h<shrink_v) ? shrink_h : shrink_v;
	int target_level = 0;
	while( shrink_min < 0.5 ) {
		target_level++;
		shrink_min *= 2;
	}
  /*
  if( shrink_min < 0.75 ) {
    target_level++;
    shrink_min *= 2;
  }
  */

  std::cout<<"ImageEditor::zoom_fit(): image area size="
           <<imageArea_scrolledWindow.get_hadjustment()->get_page_size()<<","
           <<imageArea_scrolledWindow.get_vadjustment()->get_page_size()
           <<"  image size="<<out->Xsize<<","<<out->Ysize
           <<"  level="<<target_level<<"  shrink="<<shrink_min<<std::endl;

	imageArea->set_shrink_factor( shrink_min );
	pipeline2->set_level( target_level );
	image->update();
  image->unlock();

  /*
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
	pipeline->set_level( 0 );
	imageArea->set_shrink_factor( 1 );
	image->update(pipeline,true);
	if( !imageArea->get_display_image() ) return;
	float shrink_h = ((float)imageArea_scrolledWindow.get_hadjustment()->get_page_size())/imageArea->get_display_image()->Xsize;
	float shrink_v = ((float)imageArea_scrolledWindow.get_vadjustment()->get_page_size())/imageArea->get_display_image()->Ysize;
	float shrink_min = (shrink_h<shrink_v) ? shrink_h : shrink_v;
	int target_level = 0;
	while( shrink_min < 0.5 ) {
		target_level++;
		shrink_min *= 2;
	}

	imageArea->set_shrink_factor( shrink_min );
	pipeline->set_level( target_level );
	image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
  */
}


void PF::ImageEditor::zoom_actual_size()
{
  PF::Pipeline* pipeline = image->get_pipeline( PIPELINE_ID );
  if( !pipeline ) return;
	pipeline->set_level( 0 );
	imageArea->set_shrink_factor( 1 );
	image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


void PF::ImageEditor::set_active_layer( int id ) 
{
  PF::Layer* old_active = active_layer;
  active_layer = NULL;
  if( image )
    active_layer = image->get_layer_manager().get_layer( id );
  //std::cout<<"ImageEditor::set_active_layer("<<id<<"): old_active="<<old_active<<"  active_layer="<<active_layer<<std::endl;
  if( old_active != active_layer ) {
    if( old_active &&
        old_active->get_processor() &&
        old_active->get_processor()->get_par() &&
        old_active->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigUI* ui = old_active->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if( dialog && dialog->get_visible() ) {
        dialog->disable_editing();
      }
    }
    
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() &&
        active_layer->get_processor()->get_par()->get_config_ui() ) {
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if( dialog && dialog->get_visible() ) {
        dialog->enable_editing();
      }
    }
  }
}


bool PF::ImageEditor::screen2image( gdouble& x, gdouble& y )
{
#ifndef NDEBUG
  /**/
  std::cout<<"PF::ImageEditor::screen2image(): x="<<x<<"  y="<<y<<"  adjustments:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_value()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_value()<<std::endl;
  /**/
#endif
  //x += imageArea_scrolledWindow.get_hadjustment()->get_value();
  //y += imageArea_scrolledWindow.get_vadjustment()->get_value();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): offsets: "
           <<imageArea->get_xoffset()<<" "
           <<imageArea->get_yoffset()<<std::endl;
#endif
  x -= imageArea->get_xoffset();
  y -= imageArea->get_yoffset();
  if( (x<0) || (y<0) ) return false;
  if( imageArea->get_display_image() ) {
    if( x >= imageArea->get_display_image()->Xsize ) 
      return false;
    if( y >= imageArea->get_display_image()->Ysize ) 
      return false;
  }

  float zoom_fact = get_zoom_factor();
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): zoom_factor="<<zoom_fact<<std::endl;
  std::cout<<"PF::ImageEditor::screen2image(): shrink_factor="<<imageArea->get_shrink_factor()<<std::endl;
#endif
  zoom_fact *= imageArea->get_shrink_factor();
  x /= zoom_fact;
  y /= zoom_fact;
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::screen2image(): x'="<<x<<"  y'="<<y<<std::endl;
#endif
  return true;
}


bool PF::ImageEditor::on_button_press_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_press_event(): button "<<button->button<<" pressed."<<std::endl;
#endif
  gdouble x = button->x;
  gdouble y = button->y;
  screen2image( x, y );
#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
  std::cout<<"  active_layer: "<<active_layer<<std::endl;
#endif
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
    if( dialog && dialog->get_visible() ) {
      //#ifndef NDEBUG
      std::cout<<"  sending button press event to dialog"<<std::endl;
      //#endif
      int mod_key = PF::MOD_KEY_NONE;
      if( button->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
      if( button->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
      if( dialog->pointer_press_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
}


bool PF::ImageEditor::on_button_release_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_release_event(): button "<<button->button<<" released."<<std::endl;
#endif
  gdouble x = button->x;
  gdouble y = button->y;
  screen2image( x, y );
#ifndef NDEBUG
  std::cout<<"  pointer @ "<<x<<","<<y<<std::endl;
#endif
  if( active_layer &&
      active_layer->get_processor() &&
      active_layer->get_processor()->get_par() ) {
    PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
    PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
    if( dialog && dialog->get_visible() ) {
#ifndef NDEBUG
      std::cout<<"  sending button release event to dialog"<<std::endl;
#endif
      int mod_key = PF::MOD_KEY_NONE;
      if( button->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
      if( button->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
      if( dialog->pointer_release_event( button->button, x, y, mod_key ) ) {
        // The dialog requires to draw on top of the preview image, so we call draw_area() 
        // to refresh the preview
        imageArea->draw_area();
      }
    }
  }
}


bool PF::ImageEditor::on_motion_notify_event( GdkEventMotion* event )
{
  /*
  GDK_SHIFT_MASK    = 1 << 0,
  GDK_LOCK_MASK     = 1 << 1,
  GDK_CONTROL_MASK  = 1 << 2,
  GDK_MOD1_MASK     = 1 << 3,
  GDK_MOD2_MASK     = 1 << 4,
  GDK_MOD3_MASK     = 1 << 5,
  GDK_MOD4_MASK     = 1 << 6,
  GDK_MOD5_MASK     = 1 << 7,
  GDK_BUTTON1_MASK  = 1 << 8,
  GDK_BUTTON2_MASK  = 1 << 9,
  GDK_BUTTON3_MASK  = 1 << 10,
  GDK_BUTTON4_MASK  = 1 << 11,
  GDK_BUTTON5_MASK  = 1 << 12,
  */

	int ix, iy;
	gdouble x, y;
	guint state;
	if (event->is_hint) {
		//event->window->get_pointer(&ix, &iy, &state);
		/*
      x = ix;
      y = iy;
      return true;
		*/
		x = event->x;
		y = event->y;
		state = event->state;	
	} else {
		x = event->x;
		y = event->y;
		state = event->state;	
	}
  if( state & GDK_BUTTON1_MASK ) {
    //gdouble x = event->x;
    //gdouble y = event->y;
    if( !screen2image( x, y ) )
      return true;
#ifndef NDEBUG
    std::cout<<"PF::ImageEditor::on_motion_notify_event(): pointer @ "<<x<<","<<y
             <<"  hint: "<<event->is_hint<<"  state: "<<event->state
             <<std::endl;
#endif
    if( active_layer &&
        active_layer->get_processor() &&
        active_layer->get_processor()->get_par() ) {
      PF::OperationConfigUI* ui = active_layer->get_processor()->get_par()->get_config_ui();
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if( dialog && dialog->get_visible() ) {
#ifndef NDEBUG
        std::cout<<"  sending motion event to dialog"<<std::endl;
#endif
        int mod_key = PF::MOD_KEY_NONE;
        if( event->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
        if( event->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
        if( dialog->pointer_motion_event( 1, x, y, mod_key ) ) {
          // The dialog requires to draw on top of the preview image, so we call draw_area() 
          // to refresh the preview
          imageArea->draw_area();
        }
      }
    }
  }
	return true;
}


bool PF::ImageEditor::on_configure_event( GdkEventConfigure* event )
{
	std::cout<<"ImageEditor::on_configure_event() called"<<std::endl;
	return false;
}
