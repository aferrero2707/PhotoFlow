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
    //insert new operations here
  }

  return dialog;
}
