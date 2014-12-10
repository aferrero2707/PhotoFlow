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


#include "../../base/processor.hh"
#include "operations.hh"

PF::ProcessorBase* PF::new_gmic_operation( std::string op_type )
{
  PF::ProcessorBase* processor = NULL;

  if( op_type == "gmic" ) {
    processor = new_gmic();
  } else if( op_type == "gmic_blur_bilateral" ) {
    processor = new_gmic_blur_bilateral();
  } else if( op_type == "gmic_denoise" ) {
    processor = new_gmic_denoise();
  } else if( op_type == "gmic_smooth_anisotropic" ) {
    processor = new_gmic_smooth_anisotropic();
  } else if( op_type == "gmic_smooth_diffusion" ) {
    processor = new_gmic_smooth_diffusion();
  } else if( op_type == "gmic_smooth_mean_curvature" ) {
    processor = new_gmic_smooth_mean_curvature();
  } else if( op_type == "gmic_smooth_wavelets_haar" ) {
    processor = new_gmic_smooth_wavelets_haar();
  } else if( op_type == "gmic_smooth_median" ) {
    processor = new_gmic_smooth_median();
  } else if( op_type == "gmic_smooth_selective_gaussian" ) {
    processor = new_gmic_smooth_selective_gaussian();
  } else if( op_type == "gmic_smooth_total_variation" ) {
    processor = new_gmic_smooth_total_variation();
  } else if( op_type == "gmic_emulate_film_colorslide" ) {
    processor = new_gmic_emulate_film_colorslide();
  } else if( op_type == "gmic_emulate_film_bw" ) {
    processor = new_gmic_emulate_film_bw();
  } else if( op_type == "gmic_emulate_film_instant_consumer" ) {
    processor = new_gmic_emulate_film_instant_consumer();
  } else if( op_type == "gmic_emulate_film_instant_pro" ) {
    processor = new_gmic_emulate_film_instant_pro();
  } else if( op_type == "gmic_emulate_film_negative_color" ) {
    processor = new_gmic_emulate_film_negative_color();
  } else if( op_type == "gmic_emulate_film_negative_new" ) {
    processor = new_gmic_emulate_film_negative_new();
  } else if( op_type == "gmic_emulate_film_negative_old" ) {
    processor = new_gmic_emulate_film_negative_old();
  } else if( op_type == "gmic_emulate_film_print_films" ) {
    processor = new_gmic_emulate_film_print_films();
  } else if( op_type == "gmic_emulate_film_various" ) {
    processor = new_gmic_emulate_film_various();
  } else if( op_type == "gmic_gcd_despeckle" ) {
    processor = new_gmic_gcd_despeckle();
  } else if( op_type == "gmic_smooth_guided" ) {
    processor = new_gmic_smooth_guided();
  } else if( op_type == "gmic_iain_denoise" ) {
    processor = new_gmic_iain_denoise();
  } else if( op_type == "gmic_dream_smooth" ) {
    processor = new_gmic_dream_smooth();
  } else if( op_type == "gmic_extract_foreground" ) {
    processor = new_gmic_extract_foreground();
  } else if( op_type == "gmic_tone_mapping" ) {
    processor = new_gmic_tone_mapping();
    //insert new operations here
  }

  return processor;
}
