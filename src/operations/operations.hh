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
  ProcessorBase* new_raw_developer_v1();
  ProcessorBase* new_raw_output_v1();
  ProcessorBase* new_buffer();
  ProcessorBase* new_blender();
  ProcessorBase* new_clip();
  ProcessorBase* new_brightness_contrast();
  ProcessorBase* new_white_balance();
  ProcessorBase* new_levels();
  ProcessorBase* new_basic_adjustments();
  ProcessorBase* new_color_correction();
  ProcessorBase* new_hue_saturation();
  ProcessorBase* new_hsl_mask();
  ProcessorBase* new_clone();
  ProcessorBase* new_crop();
  ProcessorBase* new_scale();
  ProcessorBase* new_perspective();
  ProcessorBase* new_convert_colorspace();
  ProcessorBase* new_icc_transform();
  ProcessorBase* new_curves();
  ProcessorBase* new_channel_mixer();
  ProcessorBase* new_gradient();
  ProcessorBase* new_gaussblur();
  ProcessorBase* new_blur_bilateral();
  ProcessorBase* new_denoise();
  ProcessorBase* new_unsharp_mask();
  ProcessorBase* new_sharpen();
  ProcessorBase* new_invert();
  ProcessorBase* new_threshold();
  ProcessorBase* new_uniform();
  ProcessorBase* new_desaturate();
  ProcessorBase* new_desaturate_luminance();
  ProcessorBase* new_desaturate_average();
  ProcessorBase* new_dynamic_range_compressor();
  ProcessorBase* new_dynamic_range_compressor_v2();
  ProcessorBase* new_draw();
  ProcessorBase* new_clone_stamp();
  ProcessorBase* new_lensfun();
  ProcessorBase* new_tone_mapping();
  ProcessorBase* new_tone_mapping_v2();
  ProcessorBase* new_local_contrast();
  ProcessorBase* new_clahe();
#ifdef HAVE_OCIO
  ProcessorBase* new_ocio_config();
  ProcessorBase* new_ocio_view();
  ProcessorBase* new_ocio_aces();
#endif
  ProcessorBase* new_volume();
  ProcessorBase* new_noise_generator();
  ProcessorBase* new_path_mask();
  ProcessorBase* new_shadows_highlights();
  ProcessorBase* new_shadows_highlights_v2();
  ProcessorBase* new_defringe();
  ProcessorBase* new_guided_filter();
  ProcessorBase* new_median_filter();
  ProcessorBase* new_split_details();
  ProcessorBase* new_subtrimg();
  //ProcessorBase* new_vips_operation( std::string op_type );
}

#endif
