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


PF::ScaleConfigGUI::ScaleConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Scale/Rotate" ),
  rotate_angle_slider( this, "rotate_angle", _("rotation angle"), 0, -360, 360, 0.01, 10, 1 ),
  autocrop( this, "autocrop", _("auto crop"), false ),
  scale_mode( this, "scale_mode", _("scale mode: "), 0 ),
  scale_unit( this, "scale_unit", _("unit: "), 0 ),
  scale_width_pixels_slider( this, "scale_width_pixels", _("width: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_height_pixels_slider( this, "scale_height_pixels", _("height: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_width_percent_slider( this, "scale_width_percent", _("width: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_height_percent_slider( this, "scale_height_percent", _("height: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_width_mm_slider( this, "scale_width_mm", _("W: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_height_mm_slider( this, "scale_height_mm", _("H: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_width_cm_slider( this, "scale_width_cm", _("W: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_height_cm_slider( this, "scale_height_cm", _("H: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_width_inches_slider( this, "scale_width_inches", _("W: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_height_inches_slider( this, "scale_height_inches", _("H: "), 0, 0, 10000000, 1, 10, 1 ),
  scale_resolution_slider( this, "scale_resolution", _("resolution: "), 0, 0, 10000000, 1, 10, 1 ),
  active_point_id( -1 )
{
  controlsBox.pack_start( rotate_angle_slider );
  controlsBox.pack_start( autocrop );

  controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 10 );

  controlsBox.pack_start( scale_mode, Gtk::PACK_SHRINK, 5 );
  controlsBox.pack_start( scale_unit, Gtk::PACK_SHRINK, 5 );
  
  //scale_controls_box.pack_end( scale_unit );
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



void PF::ScaleConfigGUI::do_update()
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
      //std::cout<<"ScaleConfigGUI::do_update(): scale_unit="<<prop->get_enum_value().first<<std::endl;
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
  OperationConfigGUI::do_update();
}



bool PF::ScaleConfigGUI::pointer_press_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  if( button != 1 ) return false;

  PF::ScalePar* par = dynamic_cast<PF::ScalePar*>(get_par());
  if( !par ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  // Find handle point
  std::vector< std::pair<int,int> >::iterator i;
  active_point_id = -1;
  for(unsigned int i = 0; i < par->get_rotation_points().size(); i++ ) {
    double dx = x - par->get_rotation_points()[i].first;
    double dy = y - par->get_rotation_points()[i].second;
    if( (fabs(dx) > 10) || (fabs(dy) > 10) ) continue;
    active_point_id = i;
    break;
  }
  if( active_point_id >= 0 ) {
    return true;
  }
  return false;
}


bool PF::ScaleConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  PF::ScalePar* par = dynamic_cast<PF::ScalePar*>(get_par());
  if( !par ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  bool result = false;

  if( button == 1 && active_point_id < 0) {
    if( par->get_rotation_points().size() != 2 ) {
      par->get_rotation_points().push_back( std::make_pair((int)x,(int)y) );
      result = true;
    } else {
      // if the points are already created, we dont do anything
    }
  }

  float pi = 3.1415926535;
  if( par->get_rotation_points().size() == 2 ) {
    int dx = par->get_rotation_points()[1].first - par->get_rotation_points()[0].first;
    int dy = par->get_rotation_points()[1].second - par->get_rotation_points()[0].second;
    if( abs(dx) > abs(dy) ) {
      // Horizontal line
      if( dx < 0 ) {
        dx = -dx; dy = -dy;
      }
      float angle = atan2( dy, dx )*180.0f/pi;
      //std::cout<<"angle: "<<-angle<<std::endl;
      rotate_angle_slider.get_adjustment()->set_value( -angle );
      do_update();
    } else {
      // Vertical line
      if( dy < 0 ) {
        dx = -dx; dy = -dy;
      }
      float angle = atan2( dx, dy )*180.0f/pi;
      //std::cout<<"angle: "<<angle<<std::endl;
      rotate_angle_slider.get_adjustment()->set_value( angle );
      do_update();
    }
  }

  return result;
}


bool PF::ScaleConfigGUI::pointer_motion_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  if( button != 1 ) return false;
  if( active_point_id < 0 ) return false;

  PF::ScalePar* par = dynamic_cast<PF::ScalePar*>(get_par());
  if( !par ) return false;
  if( par->get_rotation_points().size() <= active_point_id ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  int ix = x;
  int iy = y;

  par->get_rotation_points()[active_point_id].first = ix;
  par->get_rotation_points()[active_point_id].second = iy;

  return true;
}




bool PF::ScaleConfigGUI::modify_preview( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out,
                                                            float scale, int xoffset, int yoffset )
{
  PF::ScalePar* par = dynamic_cast<PF::ScalePar*>(get_par());
  if( !par ) return false;

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to outout
  buf_out.copy( buf_in );

  if( par->get_rotation_points().size() == 2 ) {
    double px1 = par->get_rotation_points()[0].first,
        py1 = par->get_rotation_points()[0].second,
        pw1 = 1, ph1 = 1;
    layer2screen( px1, py1, pw1, ph1 );
    double px2 = par->get_rotation_points()[1].first,
        py2 = par->get_rotation_points()[1].second,
        pw2 = 1, ph2 = 1;
    layer2screen( px2, py2, pw2, ph2 );
    buf_out.draw_line( px1, py1, px2, py2, buf_in );
  }

  int point_size = 2;

  for(unsigned int i = 0; i < par->get_rotation_points().size(); i++ ) {
    double px = par->get_rotation_points()[i].first,
        py = par->get_rotation_points()[i].second,
        pw = 1, ph = 1;
    layer2screen( px, py, pw, ph );
    VipsRect point = { (int)px-point_size-1,
                       (int)py-point_size-1,
                       point_size*2+3, point_size*2+3};
    VipsRect point2 = { (int)px-point_size,
                        (int)py-point_size,
                        point_size*2+1, point_size*2+1};
    buf_out.fill( point, 0, 0, 0 );
    if( i == active_point_id )
      buf_out.fill( point2, 255, 0, 0 );
    else
      buf_out.fill( point2, 255, 255, 255 );
  }

  return true;
}
