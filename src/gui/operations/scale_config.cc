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

#include "../../operations/scale.hh"

#include "scale_config.hh"


PF::ScaleConfigDialog::ScaleConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Scale" ),
  scale_mode( this, "scale_mode", "Mode: ", 0 ),
  scale_unit( this, "scale_unit", "", 0 ),
  scale_width_pixels_slider( this, "scale_width_pixels", "width: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_height_pixels_slider( this, "scale_height_pixels", "height: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_width_percent_slider( this, "scale_width_percent", "width: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_height_percent_slider( this, "scale_height_percent", "height: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_width_mm_slider( this, "scale_width_mm", "width: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_height_mm_slider( this, "scale_height_mm", "height: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_width_cm_slider( this, "scale_width_cm", "width: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_height_cm_slider( this, "scale_height_cm", "height: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_width_inches_slider( this, "scale_width_inches", "width: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_height_inches_slider( this, "scale_height_inches", "height: ", 0, 0, 10000000, 1, 10, 1 ),
  scale_resolution_slider( this, "scale_resolution", "resolution: ", 0, 0, 10000000, 1, 10, 1 )
{
  controlsBox.pack_start( scale_mode );
  
  scale_controls_box.pack_end( scale_unit );
  controlsBox.pack_start( scale_controls_box );

  scale_pixels_box.pack_start( scale_width_pixels_slider );
  scale_pixels_box.pack_start( scale_height_pixels_slider );

  scale_percent_box.pack_start( scale_width_percent_slider );
  scale_percent_box.pack_start( scale_height_percent_slider );

  scale_mm_box.pack_start( scale_width_mm_slider );
  scale_mm_box.pack_start( scale_height_mm_slider );

  scale_cm_box.pack_start( scale_width_cm_slider );
  scale_cm_box.pack_start( scale_height_cm_slider );

  scale_inches_box.pack_start( scale_width_inches_slider );
  scale_inches_box.pack_start( scale_height_inches_slider );

  add_widget( controlsBox );
}



void PF::ScaleConfigDialog::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    if( scale_pixels_box.get_parent() == &scale_controls_box )
      scale_controls_box.remove( scale_pixels_box );

    if( scale_percent_box.get_parent() == &scale_controls_box )
      scale_controls_box.remove( scale_percent_box );

    if( scale_mm_box.get_parent() == &scale_controls_box )
      scale_controls_box.remove( scale_mm_box );

    if( scale_cm_box.get_parent() == &scale_controls_box )
      scale_controls_box.remove( scale_cm_box );

    if( scale_inches_box.get_parent() == &scale_controls_box )
      scale_controls_box.remove( scale_inches_box );

    if( scale_resolution_slider.get_parent() == &controlsBox )
      controlsBox.remove( scale_resolution_slider );

    PF::OpParBase* par = get_layer()->get_processor()->get_par();
    PF::PropertyBase* prop = par->get_property( "scale_unit" );
    PF::PropertyBase* prop2 = par->get_property( "scale_mode" );
    if( prop && prop2 && prop->is_enum() && prop2->is_enum() ) {
      std::cout<<"ScaleConfigDialog::do_update(): scale_unit="<<prop->get_enum_value().first<<std::endl;
      switch( prop->get_enum_value().first ) {
      case PF::SCALE_UNIT_PX:
        scale_controls_box.pack_start( scale_pixels_box );
        //scale_pixels_box.show_all_children();
        //scale_pixels_box.show();
        break;
      case PF::SCALE_UNIT_PERCENT:
        if( prop2->get_enum_value().first == PF::SCALE_MODE_FIT )
          scale_height_percent_slider.set_editable( false );
        else
          scale_height_percent_slider.set_editable( true );
        scale_controls_box.pack_start( scale_percent_box );
        //scale_percent_box.show_all_children();
        //scale_percent_box.show();
        break;
      case PF::SCALE_UNIT_MM:
        scale_controls_box.pack_start( scale_mm_box );
        controlsBox.pack_start( scale_resolution_slider );
        //scale_mm_box.show();
        break;
      case PF::SCALE_UNIT_CM:
        scale_controls_box.pack_start( scale_cm_box );
        controlsBox.pack_start( scale_resolution_slider );
        //scale_cm_box.show();
        break;
      case PF::SCALE_UNIT_INCHES:
        scale_controls_box.pack_start( scale_inches_box );
        controlsBox.pack_start( scale_resolution_slider );
        //scale_inches_box.show();
        break;
      }
    }
    controlsBox.show_all_children();
    //scale_controls_box.show();
  }
  OperationConfigDialog::do_update();
}
