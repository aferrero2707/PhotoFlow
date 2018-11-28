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

#pragma once

#include <gtkmm.h>

#include "../operation_config_gui.hh"


namespace PF {

class NoiseGeneratorConfigGUI: public OperationConfigGUI
{
  Gtk::VBox controlsBox;

  CheckBox monochrome_checkbox;
  CheckBox perceptual_checkbox;
  Slider center_slider;
  Slider range_slider;

public:
  NoiseGeneratorConfigGUI( Layer* l ):
    OperationConfigGUI( l, "Noise" ),
    monochrome_checkbox( this, "monochrome", _("monochrome"), true),
    perceptual_checkbox( this, "perceptual", _("perceptual"), true),
    center_slider( this, "center", _("center"), 0, 0, 1, 0.01, 0.05, 1),
    range_slider( this, "range", _("range"), 0, 0, 1, 0.01, 0.05, 1)
{
    controlsBox.pack_start( monochrome_checkbox, Gtk::PACK_SHRINK );
    controlsBox.pack_start( perceptual_checkbox, Gtk::PACK_SHRINK );
    controlsBox.pack_start( center_slider, Gtk::PACK_SHRINK );
    controlsBox.pack_start( range_slider, Gtk::PACK_SHRINK );
    add_widget( controlsBox );
}


  bool has_preview() { return false; }
};

}
