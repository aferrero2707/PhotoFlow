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

//#include "../../operations/hue_saturation.hh"

#include "../../base/color.hh"
#include "color_correction_config.hh"




PF::ColorCorrectionConfigGUI::ColorCorrectionConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Color Correction" ),
  r_frame(_("red channel")),
  g_frame(_("green channel")),
  b_frame(_("blue channel")),
  r_offs_slider( this, "r_offs", _("offset"), 0, -100, 100, 0.5, 5, 100),
  g_offs_slider( this, "g_offs", _("offset"), 0, -100, 100, 0.5, 5, 100),
  b_offs_slider( this, "b_offs", _("offset"), 0, -100, 100, 0,5, 5, 100),
  r_slope_slider( this, "r_slope", _("slope"), 0, 0, 10, 0.1, 0.5, 1),
  g_slope_slider( this, "g_slope", _("slope"), 0, 0, 10, 0.1, 0.5, 1),
  b_slope_slider( this, "b_slope", _("slope"), 0, 0, 10, 0.1, 0.5, 1),
  r_pow_slider( this, "r_pow", _("power"), 0, 0.01, 10, 0.01, 0.05, 1),
  g_pow_slider( this, "g_pow", _("power"), 0, 0.01, 10, 0.01, 0.05, 1),
  b_pow_slider( this, "b_pow", _("power"), 0, 0.01, 10, 0.01, 0.05, 1),
  saturation_slider( this, "saturation", _("saturation"), 0, 0, 2, 0.01, 0.1, 1)
{
  r_box.pack_start( r_slope_slider, Gtk::PACK_SHRINK );
  r_box.pack_start( r_offs_slider, Gtk::PACK_SHRINK );
  r_box.pack_start( r_pow_slider, Gtk::PACK_SHRINK );
  r_frame.add(r_box);
  controlsBox.pack_start( r_frame, Gtk::PACK_SHRINK );

  g_box.pack_start( g_slope_slider, Gtk::PACK_SHRINK );
  g_box.pack_start( g_offs_slider, Gtk::PACK_SHRINK );
  g_box.pack_start( g_pow_slider, Gtk::PACK_SHRINK );
  g_frame.add(g_box);
  controlsBox.pack_start( g_frame, Gtk::PACK_SHRINK );

  b_box.pack_start( b_slope_slider, Gtk::PACK_SHRINK );
  b_box.pack_start( b_offs_slider, Gtk::PACK_SHRINK );
  b_box.pack_start( b_pow_slider, Gtk::PACK_SHRINK );
  b_frame.add(b_box);
  controlsBox.pack_start( b_frame, Gtk::PACK_SHRINK );

  controlsBox.pack_start( saturation_slider, Gtk::PACK_SHRINK );

  add_widget( controlsBox );
}


