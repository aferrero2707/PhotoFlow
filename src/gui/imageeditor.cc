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

#include "imageeditor.hh"


#define PIPELINE_ID 1


PF::ImageEditor::ImageEditor( Image* img ):
  image( img ),
  active_layer( NULL ),
  imageArea( image->get_view(PIPELINE_ID) ),
  layersWidget( image ),
  buttonZoomIn( "Zoom +" ),
  buttonZoomOut( "Zoom -" ),
  buttonShowMerged( "show merged layers" ),
  buttonShowActive( "show active layer" )
{
  imageArea.set_adjustments( imageArea_scrolledWindow.get_hadjustment(),
			     imageArea_scrolledWindow.get_vadjustment() );

  imageArea_scrolledWindow.add( imageArea );
  imageArea_eventBox.add( imageArea_scrolledWindow );

  radioBox.pack_start( buttonShowMerged );
  radioBox.pack_start( buttonShowActive );

  Gtk::RadioButton::Group group = buttonShowMerged.get_group();
  buttonShowActive.set_group(group);

  controlsBox.pack_end( radioBox, Gtk::PACK_SHRINK );
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

  buttonShowMerged.signal_clicked().connect( sigc::bind( sigc::mem_fun(&imageArea,
								       &PF::ImageArea::set_display_merged),
							 true) );
  buttonShowActive.signal_clicked().connect( sigc::bind( sigc::mem_fun(&imageArea,
								       &PF::ImageArea::set_display_merged),
							 false) );
  //set_position( get_allocation().get_width()-200 );

  layersWidget.signal_active_layer_changed.connect( sigc::mem_fun(&imageArea,
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

  imageArea.add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK );
  //imageArea_scrolledWindow.add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
  //add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );

  show_all_children();
}


PF::ImageEditor::~ImageEditor()
{
  if( image )
    delete image;
}



void PF::ImageEditor::zoom_out()
{
  PF::View* view = image->get_view( PIPELINE_ID );
  if( !view ) return;
  int level = view->get_level();
  view->set_level( level + 1 );
  image->update();

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_out(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif
}


void PF::ImageEditor::zoom_in()
{
  PF::View* view = image->get_view( PIPELINE_ID );
  if( !view ) return;
  int level = view->get_level();
  if( level > 0 ) {
    view->set_level( level - 1 );
    image->update();
  }

#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::zoom_in(): area size:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_page_size()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_page_size()<<std::endl;
#endif

}


void PF::ImageEditor::screen2image( gdouble& x, gdouble& y )
{
#ifndef NDEBUG
  /**/
  std::cout<<"PF::ImageEditor::screen2image(): x+"<<x<<"  y="<<y<<"  adjustments:"
	   <<"  h="<<imageArea_scrolledWindow.get_hadjustment()->get_value()
	   <<"  v="<<imageArea_scrolledWindow.get_vadjustment()->get_value()<<std::endl;
  /**/
#endif
  //x += imageArea_scrolledWindow.get_hadjustment()->get_value();
  //y += imageArea_scrolledWindow.get_vadjustment()->get_value();
  x -= imageArea.get_xoffset();
  y -= imageArea.get_yoffset();
  float zoom_fact = get_zoom_factor();
  x /= zoom_fact;
  y /= zoom_fact;
}


bool PF::ImageEditor::on_button_press_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_press_event(): button "<<button->button<<" pressed."<<std::endl;
#endif
  if( button->button == 1 ) {
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
	std::cout<<"  sending button press event to dialog"<<std::endl;
#endif
	int mod_key = PF::MOD_KEY_NONE;
	if( button->state & GDK_CONTROL_MASK ) mod_key += PF::MOD_KEY_CTRL;
	if( button->state & GDK_SHIFT_MASK ) mod_key += PF::MOD_KEY_SHIFT;
	dialog->pointer_press_event( button->button, x, y, mod_key );
      }
    }
  }
}


bool PF::ImageEditor::on_button_release_event( GdkEventButton* button )
{
#ifndef NDEBUG
  std::cout<<"PF::ImageEditor::on_button_release_event(): button "<<button->button<<" released."<<std::endl;
#endif
  if( button->button == 1 ) {
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
	dialog->pointer_release_event( button->button, x, y, mod_key );
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
  if( event->state & GDK_BUTTON1_MASK ) {
    gdouble x = event->x;
    gdouble y = event->y;
    screen2image( x, y );
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
	dialog->pointer_motion_event( 1, x, y, mod_key );
      }
    }
  }
}


