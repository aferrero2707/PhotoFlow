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
  offs_frame(_("offset")),
  slope_frame(_("slope")),
  pow_frame(_("power")),
  offs_slider( this, "offs", _("offset"), 0, -1, 1, 0.01, 0.05, 1),
  r_offs_slider( this, "r_offs", _("red"), 0, -1, 1, 0.01, 0.05, 1),
  g_offs_slider( this, "g_offs", _("green"), 0, -1, 1, 0.01, 0.05, 1),
  b_offs_slider( this, "b_offs", _("blue"), 0, -1, 1, 0.01, 0.05, 1),
  slope_slider( this, "slope", _("slope"), 0, -1, 1, 0.01, 0.05, 1),
  r_slope_slider( this, "r_slope", _("red"), 0, -1, 1, 0.01, 0.05, 1),
  g_slope_slider( this, "g_slope", _("green"), 0, -1, 1, 0.01, 0.05, 1),
  b_slope_slider( this, "b_slope", _("blue"), 0, -1, 1, 0.01, 0.05, 1),
  pow_slider( this, "pow", _("power"), 0, -1, 1, 0.01, 0.05, 1),
  r_pow_slider( this, "r_pow", _("red"), 0, -1, 1, 0.01, 0.05, 1),
  g_pow_slider( this, "g_pow", _("green"), 0, -1, 1, 0.01, 0.05, 1),
  b_pow_slider( this, "b_pow", _("blue"), 0, -1, 1, 0.01, 0.05, 1),
  saturation_slider( this, "saturation", _("saturation"), 0, 0, 2, 0.01, 0.1, 1),
  is_log( this, "log_encoding", _("log encoding"), false )
{
  controlsBox.pack_start( is_log, Gtk::PACK_SHRINK );
  controlsBox.pack_start( saturation_slider, Gtk::PACK_SHRINK );

  slope_box.pack_start( slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( r_slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( g_slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( b_slope_slider, Gtk::PACK_SHRINK );
  slope_frame.add(slope_box);
  controlsBox.pack_start( slope_frame, Gtk::PACK_SHRINK );

  offs_box.pack_start( offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( r_offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( g_offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( b_offs_slider, Gtk::PACK_SHRINK );
  offs_frame.add(offs_box);
  controlsBox.pack_start( offs_frame, Gtk::PACK_SHRINK );

  pow_box.pack_start( pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( r_pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( g_pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( b_pow_slider, Gtk::PACK_SHRINK );
  pow_frame.add(pow_box);
  controlsBox.pack_start( pow_frame, Gtk::PACK_SHRINK );

  add_widget( controlsBox );
}


