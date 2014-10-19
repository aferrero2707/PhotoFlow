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
    //insert new operations here
  }

  return processor;
}
