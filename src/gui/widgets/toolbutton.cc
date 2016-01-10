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

#include <iostream>

#include "toolbutton.hh"
#include "../layerwidget.hh"


PF::ToolButton::ToolButton(Glib::ustring imgname, Glib::ustring name, Image* i, LayerWidget* lw):
toolname(name),
image(i),
layer_widget(lw)
{
  img.set( imgname );
#ifdef GTKMM_3
  img.set_opacity(0.7);
#endif
  button_box.pack_start( img, Gtk::PACK_SHRINK );
  event_box.add( button_box );
  event_box.add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );

  pack_start( event_box, Gtk::PACK_SHRINK );

  //event_box.signal_button_release_event().connect(sigc::mem_fun(*this,
  //    &ToolButton::on_button_release_event) );

  show_all_children();
}



bool PF::ToolButton::on_button_press_event( GdkEventButton* button )
{
//#ifndef NDEBUG
  std::cout<<"PF::ToolButton::on_button_press_event(): button "<<button->button<<" pressed."<<std::endl;
//#endif
  if( button->button != 1 ) return false;
#ifdef GTKMM_3
  img.set_opacity(1);
#endif
  return true;
}


bool PF::ToolButton::on_button_release_event( GdkEventButton* button )
{
//#ifndef NDEBUG
  std::cout<<"PF::ToolButton::on_button_release_event(): button "<<button->button<<" released."<<std::endl;
//#endif
  if( button->button != 1 ) return false;
#ifdef GTKMM_3
  img.set_opacity(0.7);
#endif
  signal_clicked.emit();
  add_layer();
  return true;
}



void PF::ToolButton::add_layer()
{
  if(toolname.empty()) return;
  if( !image ) return;

  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  if( !layer ) return;


  PF::ProcessorBase* processor =
      PF::PhotoFlow::Instance().new_operation( toolname.c_str(), layer );
  if( !processor || !processor->get_par() ) return;
  PF::OperationConfigUI* ui = dynamic_cast<PF::OperationConfigUI*>( processor->get_par()->get_config_ui() );
  if( processor->get_par()->get_default_name().empty() )
    layer->set_name( _("New Layer") );
  else
    layer->set_name( processor->get_par()->get_default_name() );

  if( processor ) {
    layer_widget->add_layer( layer );
    //layer_manager.get_layers().push_back( layer );
    //layer_manager.modified();
    if( ui ) {
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if(dialog) {
        if( dialog ) {
          //processor->get_par()->set_config_ui( dialog );
          //dialog->update();
          dialog->open();
          //dialog->enable_editing();
        }
      }
    }
  }
}


