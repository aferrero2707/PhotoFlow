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

#ifndef PF_TONE_MAPPING_V3_H
#define PF_TONE_MAPPING_V3_H

#include <string>

//#include <glibmm.h>

#include "../base/processor.hh"
#include "../base/splinecurve.hh"
#include "Filmic/FilmicCurve/FilmicToneCurve.h"


namespace PF 
{


#define GamutMapNYPoints 1000


enum tone_mapping_v3_blend_t
{
  TONE_MAPPING_V3_BLEND_NORMAL,
  TONE_MAPPING_V3_BLEND_HUE,
  TONE_MAPPING_V3_BLEND_LUMINANCE
};


enum tone_mapping_v3_preset_t
{
  TONE_MAPPING_V3_PRESET_CUSTOM = -1,
  TONE_MAPPING_V3_PRESET_BASE_CONTRAST = 0,
  TONE_MAPPING_V3_PRESET_MEDIUM_HIGH_CONTRAST = 1,
  TONE_MAPPING_V3_PRESET_HIGH_CONTRAST = 2,
  TONE_MAPPING_V3_PRESET_VERY_HIGH_CONTRAST = 3,
  TONE_MAPPING_V3_PRESET_LINEAR = 8,
  TONE_MAPPING_V3_PRESET_S_CURVE = 9,
  TONE_MAPPING_V3_PRESET_LAST = 10
};



class TM_V3
{
public:
  float slope, slope2, slope3;
  float whitePoint;
  float exposure;
  float latitude;
  // derived parameters
  float norm;
  float Yscale;
  float Xscale;
  float midGray;
  float midGrayInv;
  float logWhite;
  float logYmax;
  float logXmax;
  float logYmin;
  float logXmin;
  float logXblack;
  float logYblack;
  float logDelta;
  float logWP;
  float sa, sb, sc, sd;
  float sa2, sb2, sc2, sd2;

  float lin2log(float lin);
  float log2lin(float l);

  void set_norm(float n) {
    norm = n;
    Yscale = norm - logYmax;
    Xscale = 1.0f / Yscale;
  }
  void init(float s, float s2, float s3, float wp, float e, float lat);

  //float logistic(float x, bool verbose=false);
  float get(float x, bool verbose=false);
  float get_log(float x, bool verbose=false);
  float get_slope(float x);
};



class ToneMappingParV3: public OpParBase
{
  //PropertyBase preset;
  Property<bool> hue_protection;

  Property<float> exposure, latitude, slope, slope2, slope3, wp;
  Property<bool> lock_exposure_wp;
  Property<float> lc_amount, lc_radius, lc_threshold;
  Property<bool> lc_enable;

  Property<float> lumi_blend_frac;
  Property<float> sh_desaturation, hl_desaturation;

  Property<SplineCurve> lc_curve;

  TM_V3 tm;
  ProcessorBase* loglumi;
  ProcessorBase* guided[10];
  float threshold_scale[10];

  ICCProfile* icc_data;

public:

  float gamut_boundary[GamutMapNYPoints+1][360];

  ToneMappingParV3();

  bool has_intensity() { return false; }
  bool needs_caching();
  bool has_opacity() { return true; }
  bool needs_input() { return true; }

  bool get_hue_protection() { return hue_protection.get(); }
  float get_sh_desaturation() { return sh_desaturation.get(); }
  float get_hl_desaturation() { return hl_desaturation.get(); }

  float get_exposure() { return exposure.get(); }
  float get_latitude() { return latitude.get(); }
  float get_slope() { return slope.get(); }
  float get_slope2() { return slope2.get(); }
  float get_slope3() { return slope3.get(); }
  float get_white_point() { return wp.get(); }

  bool get_lc_enable() { return lc_enable.get(); }
  float get_lc_amount() { return lc_amount.get(); }

  float get_lumi_blend_frac() { return lumi_blend_frac.get(); }

  SplineCurve& get_lc_curve() { return lc_curve.get(); }

  ICCProfile* get_icc_data() { return icc_data; }

  TM_V3& get_tm() { return tm; }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  void propagate_settings();
  void pre_build( rendermode_t mode );
  VipsImage* build(std::vector<VipsImage*>& in, int first,
                   VipsImage* imap, VipsImage* omap, unsigned int& level);
};



ProcessorBase* new_tone_mapping_v3();
}

#endif 


