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

#include "local_contrast_config_v2.hh"


PF::LocalContrastV2ConfigGUI::LocalContrastV2ConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Local Contrast" ),
  amount_slider( this, "amount", "amount", 100, -100, 100, 0.5, 5, 100),
  white_level_slider( this, "white_level", "white level", 100, 0, 100, 0.5, 5, 100),
  guidedRadiusSlider( this, "radius", "radius", 1, 0, 1000, 0.05, 0.1, 1),
  guidedThresholdSlider( this, "threshold", "edge threshold", 20, 0.5, 1000.0, 0.5, 5, 200),
  boost_slider( this, "boost", "boost", 100, 0, 100, 0.5, 5, 100)
{
  guidedControlsBox.set_spacing(4);
  guidedControlsBox.pack_start( guidedRadiusSlider, Gtk::PACK_SHRINK );
  guidedControlsBox.pack_start( guidedThresholdSlider, Gtk::PACK_SHRINK );

  controlsBox.set_spacing(8);
  controlsBox.pack_start( amount_slider, Gtk::PACK_SHRINK, 0 );
  //controlsBox.pack_start( white_level_slider, Gtk::PACK_SHRINK, 10 );
  controlsBox.pack_start( boost_slider, Gtk::PACK_SHRINK, 0 );
  controlsBox.pack_start( guidedControlsBox, Gtk::PACK_SHRINK, 0 );

  //globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( controlsBox );
}




void PF::LocalContrastV2ConfigGUI::do_update()
{
  OperationConfigGUI::do_update();
}

