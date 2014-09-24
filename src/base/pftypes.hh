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

#ifndef PF_TYPES_H
#define PF_TYPES_H

#include <lcms2.h>
#include <vips/vips.h>

#include "property.hh"

typedef cmsInt8Number int8_t;
typedef cmsUInt8Number uint8_t;

typedef cmsInt16Number int16_t;
typedef cmsUInt16Number uint16_t;

typedef cmsInt32Number int32_t;
typedef cmsUInt32Number uint32_t;

// 64-bit base types
#ifndef CMS_DONT_USE_INT64
typedef cmsInt64Number int64_t;
typedef cmsUInt64Number uint64_t;
#endif

typedef cmsFloat32Number float32_t;
typedef cmsFloat64Number float64_t;


namespace PF
{

  enum BandFormat {
    PF_BANDFMT_UCHAR = IM_BANDFMT_UCHAR,
    PF_BANDFMT_CHAR = IM_BANDFMT_CHAR,
    PF_BANDFMT_USHORT = IM_BANDFMT_USHORT,
    PF_BANDFMT_SHORT = IM_BANDFMT_SHORT,
    PF_BANDFMT_UINT = IM_BANDFMT_UINT,
    PF_BANDFMT_INT = IM_BANDFMT_INT,
    PF_BANDFMT_FLOAT = IM_BANDFMT_FLOAT,
    PF_BANDFMT_DOUBLE = IM_BANDFMT_DOUBLE,
    PF_BANDFMT_UNKNOWN
  };

  enum colorspace_t {
    PF_COLORSPACE_RAW,
    PF_COLORSPACE_GRAYSCALE,
    PF_COLORSPACE_RGB,
    PF_COLORSPACE_LAB,
    PF_COLORSPACE_CMYK,
    PF_COLORSPACE_UNKNOWN
  };

  template<colorspace_t CS>
  struct ColorspaceInfo
  {
    static int NCH;
  };

  enum blendmode_t {
    PF_BLEND_PASSTHROUGH,
    PF_BLEND_NORMAL,
    PF_BLEND_OVERLAY,
    PF_BLEND_MULTIPLY,
    PF_BLEND_SCREEN,
    PF_BLEND_LIGHTEN,
    PF_BLEND_DARKEN,
    PF_BLEND_LUMI,
    PF_BLEND_UNKNOWN
  };

  enum rendermode_t {
    PF_RENDER_NORMAL,
    PF_RENDER_PREVIEW,
    PF_RENDER_EDITING
  };


  colorspace_t convert_colorspace(VipsInterpretation interpretation);


  enum mod_key_t {
    MOD_KEY_NONE = 0,
    MOD_KEY_CTRL = 1,
    MOD_KEY_ALT = 2,
    MOD_KEY_SHIFT = 4
  };


  template<>
  class Property<blendmode_t>: public PropertyBase
  {
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par)
    {
      add_enum_value(PF_BLEND_PASSTHROUGH,"PF_BLEND_PASSTHROUGH","Passthrough");
      add_enum_value(PF_BLEND_NORMAL,"PF_BLEND_NORMAL","Normal");
      add_enum_value(PF_BLEND_OVERLAY,"PF_BLEND_OVERLAY","Overlay");
      add_enum_value(PF_BLEND_MULTIPLY,"PF_BLEND_MULTIPLY","Multiply");
      add_enum_value(PF_BLEND_SCREEN,"PF_BLEND_SCREEN","Screen");
      add_enum_value(PF_BLEND_LIGHTEN,"PF_BLEND_LIGHTEN","Lighten");
      add_enum_value(PF_BLEND_DARKEN,"PF_BLEND_DARKEN","Darken");
      //add_enum_value(PF_BLEND_LUMI,"PF_BLEND_LUMI","Luminosity");
      //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
      //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
      //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
    }
  };


}

#endif
