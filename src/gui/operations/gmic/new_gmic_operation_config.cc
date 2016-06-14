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

#include "../../operation_config_gui.hh"

#include "new_gmic_operation_config.hh"

#include "configs.hh"


PF::OperationConfigGUI* PF::new_gmic_operation_config( std::string op_type, PF::Layer* current_layer )
{
  PF::OperationConfigGUI* dialog = NULL;
  if( op_type == "gmic" ) {  
    dialog = new PF::GMicConfigGUI( current_layer );
  } else if( op_type == "gmic_blur_bilateral" ) {
    dialog = new PF::BlurBilateralConfigGUI( current_layer );
  } else if( op_type == "gmic_denoise" ) {
    dialog = new PF::GmicDenoiseConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_anisotropic" ) {
    dialog = new PF::GmicSmoothAnisotropicConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_diffusion" ) {
    dialog = new PF::GmicSmoothDiffusionConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_mean_curvature" ) {
    dialog = new PF::GmicSmoothMeanCurvatureConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_wavelets_haar" ) {
    dialog = new PF::GmicSmoothWaveletsHaarConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_median" ) {
    dialog = new PF::GmicSmoothMedianConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_selective_gaussian" ) {
    dialog = new PF::GmicSmoothSelectiveGaussianConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_total_variation" ) {
    dialog = new PF::GmicSmoothTotalVariationConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_colorslide" ) {
    dialog = new PF::GmicEmulateFilmColorslideConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_bw" ) {
    dialog = new PF::GmicEmulateFilmBEConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_instant_consumer" ) {
    dialog = new PF::GmicEmulateFilmInstantConsumerConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_instant_pro" ) {
    dialog = new PF::GmicEmulateFilmInstantProConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_color" ) {
    dialog = new PF::GmicEmulateFilmNegativeColorConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_new" ) {
    dialog = new PF::GmicEmulateFilmNegativeNewConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_old" ) {
    dialog = new PF::GmicEmulateFilmNegativeOldConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_print_films" ) {
    dialog = new PF::GmicEmulateFilmPrintFilmsConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_various" ) {
    dialog = new PF::GmicEmulateFilmVariousConfigGUI( current_layer );
  } else if( op_type == "gmic_emulate_film_user_defined" ) {
    dialog = new PF::GmicEmulateFilmUserDefinedConfigGUI( current_layer );
  } else if( op_type == "gmic_gcd_despeckle" ) {
    dialog = new PF::GmicGcdDespeckleConfigGUI( current_layer );
  } else if( op_type == "gmic_smooth_guided" ) {
    dialog = new PF::GmicSmoothGuidedConfigGUI( current_layer );
  } else if( op_type == "gmic_iain_denoise" ) {
    dialog = new PF::GmicIainDenoiseConfigGUI( current_layer );
  } else if( op_type == "gmic_dream_smooth" ) {
    dialog = new PF::GmicDreamSmoothConfigGUI( current_layer );
  } else if( op_type == "gmic_extract_foreground" ) {
    dialog = new PF::GmicExtractForegroundConfigGUI( current_layer );
  } else if( op_type == "gmic_tone_mapping" ) {
    dialog = new PF::GmicToneMappingConfigGUI( current_layer );
  } else if( op_type == "gmic_inpaint" ) {
    dialog = new PF::GmicInpaintConfigGUI( current_layer );
  } else if( op_type == "gmic_convolve" ) {
    dialog = new PF::GmicConvolveConfigGUI( current_layer );
  } else if( op_type == "gmic_gradient_norm" ) {
    dialog = new PF::GmicGradientNormConfigGUI( current_layer );
  } else if( op_type == "gmic_sharpen_rl" ) {
    dialog = new PF::GmicSharpenRLConfigGUI( current_layer );
  } else if( op_type == "gmic_split_details" ) {
    dialog = new PF::GmicSplitDetailsConfigGUI( current_layer );
  } else if( op_type == "gmic_transfer_colors" ) {
    dialog = new PF::GmicTransferColorsConfigGUI( current_layer );
  } else if( op_type == "gmic_watermark_fourier" ) {
    dialog = new PF::GmicWatermarkFourierConfigGUI( current_layer );
    //insert new operations here
  }

  return dialog;
}
