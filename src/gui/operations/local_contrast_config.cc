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

#include "local_contrast_config.hh"


PF::LocalContrastConfigGUI::LocalContrastConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Local Contrast" ),
  amount_slider( this, "amount", "amount", 100, 0, 1000, 2, 10, 100),
  enable_equalizer_box( this, "enable_equalizer", "enable equalizer", true),
  blacks_amount_slider( this, "blacks_amount", "blacks", 100, 0, 100, 5, 10, 100),
  shadows_amount_slider( this, "shadows_amount", "shadows", 100, 0, 100, 5, 10, 100),
  midtones_amount_slider( this, "midtones_amount", "midtones", 100, 0, 100, 5, 10, 100),
  highlights_amount_slider( this, "highlights_amount", "highlights", 100, 0, 100, 5, 10, 100),
  whites_amount_slider( this, "whites_amount", "whites", 100, 0, 100, 5, 10, 100),
  guidedRadiusSlider( this, "radius", "radius", 1, 0, 1000, 0.05, 0.1, 1),
  guidedThresholdSlider( this, "threshold", "threshold", 20, 0.5, 100.0, 5, 1, 1000)
{

  guidedControlsBox.pack_start( guidedRadiusSlider, Gtk::PACK_SHRINK );
  guidedControlsBox.pack_start( guidedThresholdSlider, Gtk::PACK_SHRINK );

  controlsBox.pack_start( amount_slider, Gtk::PACK_SHRINK, 10 );
  controlsBox.pack_start( guidedControlsBox, Gtk::PACK_SHRINK, 10 );
  //controlsBox.pack_start( separator, Gtk::PACK_SHRINK, 10 );

  //controlsBox.pack_start( amount_slider, Gtk::PACK_SHRINK );
  equalizerCheckboxBox.pack_start( enable_equalizer_box, Gtk::PACK_SHRINK );
  equalizerCheckboxBox.pack_start( equalizerCheckboxPadding, Gtk::PACK_EXPAND_WIDGET );
  //controlsBox.pack_start( equalizerCheckboxBox, Gtk::PACK_SHRINK );

  equalizerBox.pack_start( blacks_amount_slider, Gtk::PACK_SHRINK, 2 );
  equalizerBox.pack_start( shadows_amount_slider, Gtk::PACK_SHRINK, 2 );
  equalizerBox.pack_start( midtones_amount_slider, Gtk::PACK_SHRINK, 2 );
  equalizerBox.pack_start( highlights_amount_slider, Gtk::PACK_SHRINK, 2 );
  equalizerBox.pack_start( whites_amount_slider, Gtk::PACK_SHRINK, 2 );
  equalizerBox.pack_start( equalizerPadding, Gtk::PACK_EXPAND_WIDGET );

  //controlsBox.pack_start( equalizerBox, Gtk::PACK_SHRINK );

  globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( globalBox );
}




void PF::LocalContrastConfigGUI::do_update()
{
  OperationConfigGUI::do_update();
}

