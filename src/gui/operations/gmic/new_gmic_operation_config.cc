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

#include "../../operation_config_dialog.hh"

#include "new_gmic_operation_config.hh"

#include "configs.hh"


PF::OperationConfigDialog* PF::new_gmic_operation_config( std::string op_type, PF::Layer* current_layer )
{
  PF::OperationConfigDialog* dialog = NULL;
  if( op_type == "gmic" ) {  
    dialog = new PF::GMicConfigDialog( current_layer );
  } else if( op_type == "gmic_blur_bilateral" ) {
    dialog = new PF::BlurBilateralConfigDialog( current_layer );
  } else if( op_type == "gmic_denoise" ) {
    dialog = new PF::GmicDenoiseConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_anisotropic" ) {
    dialog = new PF::GmicSmoothAnisotropicConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_diffusion" ) {
    dialog = new PF::GmicSmoothDiffusionConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_mean_curvature" ) {
    dialog = new PF::GmicSmoothMeanCurvatureConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_wavelets_haar" ) {
    dialog = new PF::GmicSmoothWaveletsHaarConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_median" ) {
    dialog = new PF::GmicSmoothMedianConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_selective_gaussian" ) {
    dialog = new PF::GmicSmoothSelectiveGaussianConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_total_variation" ) {
    dialog = new PF::GmicSmoothTotalVariationConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_colorslide" ) {
    dialog = new PF::GmicEmulateFilmColorslideConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_bw" ) {
    dialog = new PF::GmicEmulateFilmBEConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_instant_consumer" ) {
    dialog = new PF::GmicEmulateFilmInstantConsumerConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_instant_pro" ) {
    dialog = new PF::GmicEmulateFilmInstantProConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_color" ) {
    dialog = new PF::GmicEmulateFilmNegativeColorConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_new" ) {
    dialog = new PF::GmicEmulateFilmNegativeNewConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_negative_old" ) {
    dialog = new PF::GmicEmulateFilmNegativeOldConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_print_films" ) {
    dialog = new PF::GmicEmulateFilmPrintFilmsConfigDialog( current_layer );
  } else if( op_type == "gmic_emulate_film_various" ) {
    dialog = new PF::GmicEmulateFilmVariousConfigDialog( current_layer );
  } else if( op_type == "gmic_gcd_despeckle" ) {
    dialog = new PF::GmicGcdDespeckleConfigDialog( current_layer );
  } else if( op_type == "gmic_smooth_guided" ) {
    dialog = new PF::GmicSmoothGuidedConfigDialog( current_layer );
  } else if( op_type == "gmic_iain_denoise" ) {
    dialog = new PF::GmicIainDenoiseConfigDialog( current_layer );
  } else if( op_type == "gmic_dream_smooth" ) {
    dialog = new PF::GmicDreamSmoothConfigDialog( current_layer );
  } else if( op_type == "gmic_extract_foreground" ) {
    dialog = new PF::GmicExtractForegroundConfigDialog( current_layer );
  } else if( op_type == "gmic_tone_mapping" ) {
    dialog = new PF::GmicToneMappingConfigDialog( current_layer );
  } else if( op_type == "gmic_inpaint" ) {
    dialog = new PF::GmicInpaintConfigDialog( current_layer );
  } else if( op_type == "gmic_convolve" ) {
    dialog = new PF::GmicConvolveConfigDialog( current_layer );
  } else if( op_type == "gmic_gradient_norm" ) {
    dialog = new PF::GmicGradientNormConfigDialog( current_layer );
  } else if( op_type == "gmic_sharpen_rl" ) {
    dialog = new PF::GmicSharpenRLConfigDialog( current_layer );
  } else if( op_type == "gmic_split_details" ) {
    dialog = new PF::GmicSplitDetailsConfigDialog( current_layer );
  } else if( op_type == "gmic_transfer_colors" ) {
    dialog = new PF::GmicTransferColorsConfigDialog( current_layer );
  } else if( op_type == "gmic_watermark_fourier" ) {
    dialog = new PF::GmicWatermarkFourierConfigDialog( current_layer );
    //insert new operations here
  }

  return dialog;
}
