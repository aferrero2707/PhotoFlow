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

#include "wavdec_config.hh"


PF::WavDecConfigGUI::WavDecConfigGUI( PF::Layer* layer ):
OperationConfigGUI( layer, "Wavelet Decompose" ),
numScales_slider( this, "numScales", _("Scales"), 50, 0, 15, 1, 1, 1),
currScale_slider( this, "currScale", _("Preview Scale"), 50, 0, 15, 1, 1, 1),
blendFactor_slider( this, "blendFactor", _("Blend Factor"), 50, 0., 1., 0.1, 0.05, 1)
{
  controlsBox.pack_start( numScales_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( currScale_slider, Gtk::PACK_SHRINK );
  controlsBox.pack_start( blendFactor_slider, Gtk::PACK_SHRINK );

  add_widget( controlsBox );

  get_main_box().show_all_children();
}


