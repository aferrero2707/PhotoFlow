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

#include "shadows_highlights_config_v2.hh"


static double sh_slider_to_prop(double& val, PF::OperationConfigGUI*, void*)
{
  //return log(val)/log(10);
  return pow(10,val);
}

static double sh_prop_to_slider(double& val, PF::OperationConfigGUI*, void*)
{
  //return pow(10,val);
  float result = log(val)/log(10);
  return(roundf(result*100)/100);
}


static double hl_slider_to_prop(double& val, PF::OperationConfigGUI*, void*)
{
  //return log(val)/log(10);
  return pow(10,-1.0f*val);
}

static double hl_prop_to_slider(double& val, PF::OperationConfigGUI*, void*)
{
  //return pow(10,val);
  float result = -1.0f*log(val)/log(10);
  return(roundf(result*100)/100);
}


PF::ShadowsHighlightsConfigV2GUI::ShadowsHighlightsConfigV2GUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "shadows/highlights" ),
  shadows_frame(_("shadows")),
  highlights_frame(_("highlights")),
  blur_frame(_("mask blur")),
  amount_slider( this, "amount", "amount", 100, 0, 100, 0.5, 5, 100),
  strength_s_slider( this, "shadows", "shadows", 0, -100, 100, 1, 5, 100),
  range_s_slider( this, "shadows_range", "blacks", 0, 0, 100, 0.5, 5, 10),
  strength_h_slider( this, "highlights", "highlights", 0, -100, 100, 1, 5, 100),
  range_h_slider( this, "highlights_range", "hl. tonal width", 0, 0, 100, 0.5, 5, 100),
  constrast_slider( this, "constrast", "local constrast", 0, 0, 100, 0.5, 5, 100),
  constrast_threshold_slider( this, "constrast_threshold", "constrast threshold", 0, 0, 1000, 0.5, 5, 100),
  anchor_slider( this, "anchor", "anchor", 0, 0, 100, 0.5, 5, 100),
  median_smooth_gain_slider( this, "median_smooth_gain", "smooth gain", 0, 0, 10, 0.5, 5, 1),
  median_smooth_exponent_slider( this, "median_smooth_exponent", "smooth exponent", 0, 0, 10, 0.5, 5, 1),
  show_residual_box( this, "show_residual", "show mask", false),
  single_scale_blur_box( this, "single_scale_blur", "single scale blur", false),
  fast_approx_box( this, "fast_approx", "fast approx.", false),
  do_median_box( this, "do_median", "median filter", false),
  do_median_smoothing_box( this, "do_median_smoothing", "median smoothing", false),
  do_guided_box( this, "do_guided", "guided filter", false),
  guidedRadiusSlider( this, "sh_radius", "coarseness", 25, 0, 500, 0.5, 5, 1),
  guidedThresholdSlider( this, "sh_threshold", "edge threshold", 20, 0.5, 1000.0, 0.5, 5, 200)
{
  amount_slider.set_conversion_functions(sh_slider_to_prop, sh_prop_to_slider);
  strength_s_slider.set_conversion_functions(sh_slider_to_prop, sh_prop_to_slider);
  strength_h_slider.set_conversion_functions(hl_slider_to_prop, hl_prop_to_slider);

  controlsBox.pack_start( anchor_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( padding1, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( amount_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( range_s_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( strength_s_slider, Gtk::PACK_SHRINK, 2 );
  //shadows_frame.add(shadows_box);
  controlsBox.pack_start( strength_h_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( range_h_slider, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( constrast_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( constrast_threshold_slider, Gtk::PACK_SHRINK, 2 );
  //highlights_frame.add(highlights_box);
  //controlsBox.pack_start( padding2, Gtk::PACK_SHRINK, 2 );
  blur_box.pack_start( guidedRadiusSlider, Gtk::PACK_SHRINK, 2 );
  blur_box.pack_start( guidedThresholdSlider, Gtk::PACK_SHRINK, 2 );
  blur_frame.add(blur_box);

  //controlsBox.pack_start( shadows_frame, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( highlights_frame, Gtk::PACK_SHRINK, 2 );
  controlsBox.pack_start( blur_frame, Gtk::PACK_SHRINK, 2 );

  //controlsBox.pack_start( median_smooth_gain_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( median_smooth_exponent_slider, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( do_median_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( do_median_smoothing_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( do_guided_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( fast_approx_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( show_residual_box, Gtk::PACK_SHRINK, 2 );
  //controlsBox.pack_start( single_scale_blur_box, Gtk::PACK_SHRINK, 2 );

  globalBox.pack_start( controlsBox, Gtk::PACK_SHRINK );
  add_widget( globalBox );
}




void PF::ShadowsHighlightsConfigV2GUI::do_update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
  }

  OperationConfigGUI::do_update();
}

