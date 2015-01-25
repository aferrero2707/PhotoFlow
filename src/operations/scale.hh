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

#ifndef PF_SCALE_H
#define PF_SCALE_H

#include <string>

#include "../base/property.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"

namespace PF 
{

enum scale_mode_t
{
  SCALE_MODE_FIT,
  SCALE_MODE_FILL,
  SCALE_MODE_RESIZE
};


enum scale_unit_t
{
  SCALE_UNIT_PX,
  SCALE_UNIT_PERCENT,
  SCALE_UNIT_MM,
  SCALE_UNIT_CM,
  SCALE_UNIT_INCHES
};


  class ScalePar: public OpParBase
  {
    Property<float> rotate_angle;
    PropertyBase scale_mode;
    PropertyBase scale_unit;
    Property<int> scale_width_pixels, scale_height_pixels;
    Property<float> scale_width_percent, scale_height_percent;
    Property<float> scale_width_mm, scale_height_mm;
    Property<float> scale_width_cm, scale_height_cm;
    Property<float> scale_width_inches, scale_height_inches;
    Property<float> scale_resolution;

  public:
    ScalePar();

    bool has_opacity() { return false; }
    bool has_intensity() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ScaleProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  ProcessorBase* new_scale();
}

#endif 


