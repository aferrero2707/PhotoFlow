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

#include "relight_config.hh"


static double sh_slider_to_prop(double& val, PF::OperationConfigGUI*, void*)
{
  //return log(val)/log(10);
  return pow(10,val);
}

static double sh_prop_to_slider(double& val, PF::OperationConfigGUI*, void*)
{
  //return pow(10,val);
  float result = log(val)/log(10);
  return(roundf(result*100)/100);
}


PF::RelightConfigGUI::RelightConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "shadows/highlights" ),
  strength_s_slider( this, "strength", _("strength"), 0, 0, 100, 1, 5, 100, 250, 3),
  anchor_slider( this, "range", _("range"), 0, 0, 100, 0.5, 5, 100, 250, 3),
  contrast_slider( this, "contrast", _("contrast"), 0, 0, 100, 0.5, 5, 100, 250, 3)
{
  strength_s_slider.set_conversion_functions(sh_slider_to_prop, sh_prop_to_slider);

  controlsBox.pack_start( strength_s_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( anchor_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( contrast_slider, Gtk::PACK_SHRINK, 2 );

  //globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( controlsBox );
}




void PF::RelightConfigGUI::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
  }

  OperationConfigGUI::do_update();
}

