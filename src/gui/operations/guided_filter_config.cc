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


PF::GuidedFilterConfigGUI::GuidedFilterConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Guided filter" ),
radius_slider( this, "radius", _("radius"), 4.0, 1, 100.0, 5, 1, 1),
threshold_slider( this, "threshold", _("threshold"), 20, 0.5, 1000.0, 0.5, 1, 1000),
perceptual_cbox(this, "convert_to_perceptual", "log scale", true)
{
  controlsBox.pack_start( radius_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( threshold_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( perceptual_cbox, Gtk::PACK_SHRINK );

  add_widget( controlsBox );

  get_main_box().show_all_children();
}



