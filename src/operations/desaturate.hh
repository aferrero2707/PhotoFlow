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

#ifndef VIPS_DESATURATE_H
#define VIPS_DESATURATE_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/pixel_processor.hh"

namespace PF 
{


  enum desaturate_method_t {
    DESAT_LAB,
    DESAT_LUMINOSITY,
    DESAT_LIGHTNESS,
    DESAT_AVERAGE
  };


  class DesaturatePar: public OpParBase
  {
    PropertyBase method;

    ProcessorBase* proc_luminance;
    ProcessorBase* proc_luminosity;
    ProcessorBase* proc_lightness;
    ProcessorBase* proc_average;
    ProcessorBase* proc_average2;
    ProcessorBase* convert2lab;
    ProcessorBase* convert_cs;

  public:
    DesaturatePar();

    bool has_intensity() { return false; }

    desaturate_method_t get_method() 
    { 
      return( static_cast<desaturate_method_t>(method.get_enum_value().first) );
    }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };

  
#include "desaturate_luminosity.hh"
#include "desaturate_lightness.hh"
#include "desaturate_average.hh"

  template < OP_TEMPLATE_DEF > 
  class DesaturateProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, DesaturatePar* par)
    {
    }
  };


  ProcessorBase* new_desaturate();
}

#endif 


