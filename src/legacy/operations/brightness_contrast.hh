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

#ifndef VIPS_BRIGHTNESS_CONTRAST_H
#define VIPS_BRIGHTNESS_CONTRAST_H

#include "../base/format_info.hh"
#include "../base/pixel_processor.hh"

namespace PF 
{

#include "brightness_contrast_par.hh"
  
#include "brightness_contrast_proc.hh"


  /*
  template < OP_TEMPLATE_DEF > 
  class BrightnessContrast: public OperationPTP< OP_TEMPLATE_IMP, 
						 BrightnessContrastProc<T,CS,PREVIEW,BrightnessContrastPar>, 
						 BrightnessContrastPar >
  {
  };
  */

  template < OP_TEMPLATE_DEF > 
  class BrightnessContrast: public PixelProcessor< OP_TEMPLATE_IMP, BrightnessContrastPar, BrightnessContrastProc >
  {
  };


  ProcessorBase* new_brightness_contrast();

}

#endif 


