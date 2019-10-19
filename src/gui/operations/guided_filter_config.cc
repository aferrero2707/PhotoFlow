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

#include "guided_filter_config.hh"




static double threshold_slider_to_prop(double& val, PF::OperationConfigGUI* gui, void*)
{
  PF::GuidedFilterConfigGUI* pgui = dynamic_cast<PF::GuidedFilterConfigGUI*>(gui);
  double ret = val;
  if( pgui && pgui->is_perceptual() )
    ret = val / 100;
  std::cout<<"[threshold_slider_to_prop] is_perceptual(): "<<(pgui && pgui->is_perceptual())
      <<"  val="<<val<<"  ret="<<ret<<std::endl;
  return ret;
}

static double threshold_prop_to_slider(double& val, PF::OperationConfigGUI* gui, void*)
{
  PF::GuidedFilterConfigGUI* pgui = dynamic_cast<PF::GuidedFilterConfigGUI*>(gui);
  double ret = val;
  if( pgui && pgui->is_perceptual() )
    ret = val * 100;
  std::cout<<"[threshold_prop_to_slider] is_perceptual(): "<<(pgui && pgui->is_perceptual())
      <<"  val="<<val<<"  ret="<<ret<<std::endl;
  return ret;
}



PF::GuidedFilterConfigGUI::GuidedFilterConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Guided filter" ),
radius_slider( this, "radius", _("radius"), 4.0, 1, 500.0, 0.5, 1, 1),
threshold_slider( this, "threshold", _("threshold"), 20, 0.5, 1000.0, 0.5, 1, 1000),
subsampling_slider( this, "subsampling", _("subsampling"), 20, 1, 64.0, 1, 4, 1),
perceptual_cbox(this, "convert_to_perceptual", "log scale", true)
{
  threshold_slider.set_conversion_functions(threshold_slider_to_prop, threshold_prop_to_slider);

  controlsBox.pack_start( radius_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( threshold_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( subsampling_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( perceptual_cbox, Gtk::PACK_SHRINK );

  add_widget( controlsBox );

  get_main_box().show_all_children();
}



