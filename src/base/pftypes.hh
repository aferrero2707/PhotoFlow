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
#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef CMS_DONT_USE_INT64
typedef cmsInt64Number int64_t;
typedef cmsUInt64Number uint64_t;
#endif
#endif

typedef cmsFloat32Number float32_t;
typedef cmsFloat64Number float64_t;

#if defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)
// Work arround to fix build issues that may occur with Mingw:
// error: 'DBL_EPSILON' was not declared in this scope
// error: 'FLT_EPSILON' was not declared in this scope

#  ifndef LDBL_EPSILON
#    define LDBL_EPSILON __LDBL_EPSILON__
#  endif
#  ifndef DBL_EPSILON
#    define DBL_EPSILON __DBL_EPSILON__
#  endif
#  ifndef FLT_EPSILON
#    define FLT_EPSILON __FLT_EPSILON__
#  endif
#endif



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
    PF_COLORSPACE_MULTIBAND,
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
    PF_BLEND_ADD,
    PF_BLEND_SUBTRACT,
    PF_BLEND_GRAIN_EXTRACT,
    PF_BLEND_GRAIN_MERGE,
    PF_BLEND_OVERLAY,
    PF_BLEND_OVERLAY_GIMP,
    PF_BLEND_SOFT_LIGHT,
    PF_BLEND_HARD_LIGHT,
    PF_BLEND_VIVID_LIGHT,
    PF_BLEND_MULTIPLY,
    PF_BLEND_SCREEN,
    PF_BLEND_LIGHTEN,
    PF_BLEND_DARKEN,
    PF_BLEND_LUMI,
    PF_BLEND_LUMINANCE,
    PF_BLEND_COLOR,
    PF_BLEND_LCH_L,
    PF_BLEND_LCH_C,
    PF_BLEND_LCH_H,
    PF_BLEND_SEP1=1001,
    PF_BLEND_SEP2=1002,
    PF_BLEND_SEP3=1003,
    PF_BLEND_SEP4=1004,
    PF_BLEND_SEP5=1005,
    PF_BLEND_SEP6=1006,
    PF_BLEND_SEP7=1007,
    PF_BLEND_SEP8=1008,
    PF_BLEND_SEP9=1009,
    PF_BLEND_LAST
  };

  enum mask_blendmode_t {
    PF_MASK_BLEND_NORMAL = PF_BLEND_NORMAL,
    PF_MASK_BLEND_MULTIPLY = PF_BLEND_MULTIPLY,
    PF_MASK_BLEND_INTERSECTION = PF_BLEND_LAST+1,
    PF_MASK_BLEND_UNION,
    PF_MASK_BLEND_EXCLUSION,
    PF_MASK_BLEND_UNKNOWN
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


  enum wb_mode_t {
    WB_CAMERA=0,
    WB_SPOT=1,
    WB_COLOR_SPOT=2,
    WB_AREA_SPOT=3,
    WB_UNIWB=4,
    WB_DAYLIGHT,
    WB_DIRECT_SUNLIGHT,
    WB_CLOUDY,
    WB_SHADE,
    WB_INCANDESCENT,
    WB_INCANDESCENT_WARM,
    WB_TUNGSTEN,
    WB_FLUORESCENT,
    WB_FLUORESCENT_HIGH,
    WB_COOL_WHITE_FLUORESCENT,
    WB_WARM_WHITE_FLUORESCENT,
    WB_DAYLIGHT_FLUORESCENT,
    WB_NEUTRAL_FLUORESCENT,
    WB_WHITE_FLUORESCENT,
    WB_SODIUM_VAPOR_FLUORESCENT,
    WB_DAY_WHITE_FLUORESCENT,
    WB_HIGH_TEMP_MERCURY_VAPOR_FLUORESCENT,
    WB_FLASH,
    WB_FLASH_AUTO,
    WB_EVENING_SUN,
    WB_UNDERWATER,
    WB_BACK_AND_WHITE,
    WB_LAST
  };


  enum hlreco_mode_t {
    HLRECO_NONE,
    HLRECO_CLIP,
    HLRECO_BLEND
  };


  enum jpeg_quant_table_t {
    JPEG_QUANT_TABLE_DEFAULT = 0,
    JPEG_QUANT_TABLE_MEDIUM = 2,
    JPEG_QUANT_TABLE_BEST = 4
  };


  enum export_size_t {
    SIZE_ORIGINAL,
    SIZE_400_300,
    SIZE_800_600,
    SIZE_1280_720,
    SIZE_1280_800,
    SIZE_1280_1024,
    SIZE_1440_900,
    SIZE_1600_1200,
    SIZE_1920_1080,
    SIZE_1920_1200,
    SIZE_2048_1400,
    SIZE_2048_2048,
    SIZE_2K,
    SIZE_4K,
    SIZE_5K,
    SIZE_8K,
    SIZE_A4_300DPI,
    SIZE_A4P_300DPI,
    SIZE_CUSTOM
  };



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


  enum scale_interpolation_t
  {
    SCALE_INTERP_NEAREST,
    SCALE_INTERP_BILINEAR,
    SCALE_INTERP_BICUBIC,
    SCALE_INTERP_LANCZOS2,
    SCALE_INTERP_LANCZOS3,
    SCALE_INTERP_NOHALO
  };



  template<>
  class Property<blendmode_t>: public PropertyBase
  {
  public:
    Property(std::string name, OpParBase* par);
  };


  template<>
  class Property<mask_blendmode_t>: public PropertyBase
  {
  public:
    Property(std::string name, OpParBase* par);
  };


}

#endif
