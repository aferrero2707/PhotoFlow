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

#include "../../operations/denoise.hh"

#include "denoise_config.hh"


PF::DenoiseConfigGUI::DenoiseConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Noise Reduction" ),
  impnr_label(_("impulse NR")),
  nlmeans_label(_("non-local means")),
  impnr_frame(_("impulse NR")),
  nlmeans_frame(_("inon-local means")),
  //modeSelector( this, "nr_mode", "N.R. mode: ", PF_NR_ANIBLUR ),
  impulse_nr_enable( this, "impulse_nr_enable", "", false ),
  impulse_nr_threshold( this, "impulse_nr_threshold", _("threshold"), 50, 0, 100, 5, 20, 1),
  nlmeans_enable( this, "nlmeans_enable", "", false ),
  nlmeans_radius( this, "nlmeans_radius", _("radius"), 2, 1, 10, 1, 2, 1),
  nlmeans_strength( this, "nlmeans_strength", _("strength"), 50, 0, 100, 5, 20, 1),
  nlmeans_luma_frac( this, "nlmeans_luma_frac", _("luma"), 50, 0, 100, 5, 20, 100),
  nlmeans_chroma_frac( this, "nlmeans_chroma_frac", _("chroma"), 100, 0, 100, 5, 20, 100),
  iterationsSlider( this, "iterations", "Iterations", 1, 1, 10, 1, 1, 1),
  amplitudeSlider( this, "amplitude", "Amplitude", 1, 0, 100, 1, 1, 1),
  sharpnessSlider( this, "sharpness", "Sharpness", 1, 0, 10, 0.1, 1, 1),
  anisotropySlider( this, "anisotropy", "Anisotropy", 1, 0, 10, 0.1, 1, 1),
  alphaSlider( this, "alpha", "Alpha", 1, 0, 10, 0.1, 1, 1),
  sigmaSlider( this, "sigma", "Sigma", 1, 0, 10, 0.1, 1, 1)
{
  controlsBox.set_spacing(8);
  //impnr_vbox.pack_start( impulse_nr_enable, Gtk::PACK_SHRINK, 2 );
  impnr_vbox.pack_start( impulse_nr_threshold, Gtk::PACK_SHRINK, 2 );
  impnr_frame.add(impnr_vbox);
  impnr_label_hbox.pack_start(impulse_nr_enable);
  impnr_label_hbox.pack_start(impnr_label);
  impnr_label_hbox.set_spacing(2);
  impnr_frame.set_label_widget(impnr_label_hbox);
  controlsBox.pack_start( impnr_frame );

  //nlmeans_vbox.pack_start( nlmeans_enable, Gtk::PACK_SHRINK, 2 );
  nlmeans_vbox.pack_start( nlmeans_radius, Gtk::PACK_SHRINK, 2 );
  nlmeans_vbox.pack_start( nlmeans_strength, Gtk::PACK_SHRINK, 2 );
  nlmeans_vbox.pack_start( nlmeans_luma_frac, Gtk::PACK_SHRINK, 2 );
  nlmeans_vbox.pack_start( nlmeans_chroma_frac, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( amplitudeSlider );
  //controlsBox.pack_start( sharpnessSlider );
  //controlsBox.pack_start( anisotropySlider );
  //controlsBox.pack_start( alphaSlider );
  //controlsBox.pack_start( sigmaSlider );
  nlmeans_frame.add(nlmeans_vbox);
  nlmeans_label_hbox.pack_start(nlmeans_enable);
  nlmeans_label_hbox.pack_start(nlmeans_label);
  nlmeans_label_hbox.set_spacing(2);
  nlmeans_frame.set_label_widget(nlmeans_label_hbox);
  controlsBox.pack_start( nlmeans_frame );
  
  add_widget( controlsBox );
}



void PF::DenoiseConfigGUI::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    //radiusSlider.init();
  }
  OperationConfigGUI::open();
}
