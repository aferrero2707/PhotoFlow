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

#include "../../base/imageprocessor.hh"
#include "../../operations/uniform.hh"

#include "uniform_config.hh"


PF::UniformConfigGUI::UniformConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Uniform" ),
color_label(_("Color:              ")),
#ifdef GTKMM_2
color_button( Gdk::Color("white") )
#endif
#ifdef GTKMM_3
color_button( Gdk::RGBA("white") )
#endif
{
  colorButtonsBox1.pack_start( color_label, Gtk::PACK_SHRINK );
  colorButtonsBox1.pack_start( color_button, Gtk::PACK_SHRINK );
  controlsBox.pack_start( colorButtonsBox1 );

  color_button.signal_color_set().
      connect( sigc::mem_fun(this, &PF::UniformConfigGUI::on_color_changed) );

  add_widget( controlsBox );
}



void PF::UniformConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    // Pointer to the associated Layer object
    PF::Layer* layer = get_layer();
    PF::ProcessorBase* processor = layer->get_processor();
    PF::UniformPar* par = dynamic_cast<PF::UniformPar*>( processor->get_par() );
    if( par ) {
#ifdef GTKMM_2
      Gdk::Color color;
      color.set_rgb( par->get_R().get()*65535, par->get_G().get()*65535, par->get_B().get()*65535 );
      color_button.set_color( color );
#endif
#ifdef GTKMM_3
      Gdk::RGBA color;
      color.set_rgba( par->get_R().get(), par->get_G().get(), par->get_B().get() );
      color_button.set_rgba( color );
#endif
    }
  }
  OperationConfigGUI::open();
}


void PF::UniformConfigGUI::on_color_changed()
{
  // Pointer to the associated Layer object
  PF::Layer* layer = get_layer();
  if( !layer ) return;

  // First of all, the new stroke is recorded by the "master" operation
  PF::ProcessorBase* processor = layer->get_processor();
  if( !processor || !(processor->get_par()) ) return;

  PF::UniformPar* par = dynamic_cast<PF::UniformPar*>( processor->get_par() );
  if( !par ) return;

#ifdef GTKMM_2
  float value = color_button.get_color().get_red();
  par->get_R().set( value/65535 );
  value = color_button.get_color().get_green();
  par->get_G().set( value/65535 );
  value = color_button.get_color().get_blue();
  par->get_B().set( value/65535 );
#endif

#ifdef GTKMM_3
  par->get_R().set( color_button.get_rgba().get_red() );
  par->get_G().set( color_button.get_rgba().get_green() );
  par->get_B().set( color_button.get_rgba().get_blue() );
#endif

  if( layer->get_image() )
    layer->get_image()->update();
}


