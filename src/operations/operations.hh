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

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "gmic/operations.hh"

namespace PF
{

  ProcessorBase* new_convert_format();
  ProcessorBase* new_image_reader();
  ProcessorBase* new_raw_loader();
  ProcessorBase* new_raw_developer();
  ProcessorBase* new_raw_output();
  ProcessorBase* new_buffer();
  ProcessorBase* new_blender();
  ProcessorBase* new_brightness_contrast();
  ProcessorBase* new_hue_saturation();
  ProcessorBase* new_hsl_mask();
  ProcessorBase* new_clone();
  ProcessorBase* new_crop();
  ProcessorBase* new_scale();
  ProcessorBase* new_convert2rgb();
  ProcessorBase* new_convert2srgb();
  ProcessorBase* new_convert2lab();
  ProcessorBase* new_convert_colorspace();
  ProcessorBase* new_curves();
  ProcessorBase* new_channel_mixer();
  ProcessorBase* new_gradient();
  ProcessorBase* new_gaussblur();
  ProcessorBase* new_denoise();
  ProcessorBase* new_unsharp_mask();
  ProcessorBase* new_sharpen();
  ProcessorBase* new_invert();
  ProcessorBase* new_threshold();
  ProcessorBase* new_uniform();
  ProcessorBase* new_desaturate();
  ProcessorBase* new_draw();
  ProcessorBase* new_clone_stamp();
  ProcessorBase* new_lensfun();
  ProcessorBase* new_volume();
  //ProcessorBase* new_vips_operation( std::string op_type );
}

#endif
