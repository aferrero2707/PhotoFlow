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

#include "defringe_config.hh"


PF::DefringeConfigGUI::DefringeConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Defringe" ),

/* "method for color protection:\n - global average: fast, might show slightly wrong previews in high "
  "magnification; might sometimes protect saturation too much or too low in comparison to local "
  "average\n - local average: slower, might protect saturation better than global average by using "
  "near pixels as color reference, so it can still allow for more desaturation where required\n - "
  "static: fast, only uses the threshold as a static limit" */
op_modeSelector( this, "op_mode", _("operation mode"), 0 ),

/* "radius for detecting fringe" */
radius_slider( this, "radius", _("edge dection radius"), 4.0, 0.5, 20.0, 0.1, .01, 1), /* default=4.0, increment=0.1, decimals=1 */

/* "threshold for defringe, higher values mean less defringing" */
threshold_slider( this, "threshold", _("threshold"), 20, 0.5, 128.0, 0.1, 0.1, 1) /* default=20.0, increment=0.1, decimals=1 */
{
  controlsBox.pack_start( op_modeSelector, Gtk::PACK_SHRINK );
  controlsBox.pack_start( radius_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( threshold_slider, Gtk::PACK_SHRINK );

  add_widget( controlsBox );

  get_main_box().show_all_children();
}



