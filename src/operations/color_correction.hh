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

#ifndef COLOR_CORRECTION_HH
#define COLOR_CORRECTION_HH

#include <string>

#include "../base/color.hh"
#include "../base/processor.hh"

namespace PF 
{

  class ColorCorrectionPar: public OpParBase
  {
    Property<float> offs, r_offs, g_offs, b_offs;
    Property<float> slope, r_slope, g_slope, b_slope;
    Property<float> pow, r_pow, g_pow, b_pow;
    Property<float> saturation;
    Property<bool> is_log;

    ICCProfile* icc_data;

    cmsHPROFILE lab_profile;
    cmsHTRANSFORM transform, transform_inv;

  public:

    ColorCorrectionPar();
    ~ColorCorrectionPar();

    cmsHTRANSFORM get_transform() { return transform; }
    cmsHTRANSFORM get_transform_inv() { return transform_inv; }

    ICCProfile* get_icc_data() { return icc_data; }

    void set_offset(float r, float g, float b) {offs.set(0); r_offs.set(r); g_offs.set(g); b_offs.set(b);}
    float get_r_offset() { return (offs.get() + r_offs.get()); }
    float get_g_offset() { return (offs.get() + g_offs.get()); }
    float get_b_offset() { return (offs.get() + b_offs.get()); }
    //float get_r_slope() { return ((slope.get() + 1.0f) * (r_slope.get() + 1.0f)); }
    //float get_g_slope() { return ((slope.get() + 1.0f) * (g_slope.get() + 1.0f)); }
    //float get_b_slope() { return ((slope.get() + 1.0f) * (b_slope.get() + 1.0f)); }
    //float get_r_power() { return ((1.0f - pow.get()) * (1.0f - r_pow.get())); }
    //float get_g_power() { return ((1.0f - pow.get()) * (1.0f - g_pow.get())); }
    //float get_b_power() { return ((1.0f - pow.get()) * (1.0f - b_pow.get())); }
    void set_slope(float r, float g, float b) {slope.set(1); r_slope.set(r); g_slope.set(g); b_slope.set(b);}
    float get_r_slope() { return ((slope.get()) * (r_slope.get())); }
    float get_g_slope() { return ((slope.get()) * (g_slope.get())); }
    float get_b_slope() { return ((slope.get()) * (b_slope.get())); }
    void set_power(float r, float g, float b) {pow.set(1); r_pow.set(r); g_pow.set(g); b_pow.set(b);}
    float get_r_power() { return ((pow.get()) * (r_pow.get())); }
    float get_g_power() { return ((pow.get()) * (g_pow.get())); }
    float get_b_power() { return ((pow.get()) * (b_pow.get())); }

    void set_saturation(float s) { saturation.set(s); }
    float get_saturation() { return saturation.get(); }

    bool get_log_encoding() { return is_log.get(); }

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_input() { return true; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class ColorCorrection
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class ColorCorrection< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      ColorCorrectionPar* opar = dynamic_cast<ColorCorrectionPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      float* pin;
      float* pout;
      float* p;
      float* q;
      float RGB[3], L;
      int x, y, k, i;

      const float minus = -1.f;

      /* Do the actual processing
       */

      float r_offs = opar->get_r_offset();
      float g_offs = opar->get_g_offset();
      float b_offs = opar->get_b_offset();
      float r_slope = opar->get_r_slope();
      float g_slope = opar->get_g_slope();
      float b_slope = opar->get_b_slope();
      float r_pow = opar->get_r_power();
      float g_pow = opar->get_g_power();
      float b_pow = opar->get_b_power();
      float saturation = opar->get_saturation();
      bool is_log = opar->get_log_encoding();

      ICCProfile* profile = opar->get_icc_data();

      //for(int ti=0;ti<100;ti++) {
      //if((ti%10)==0) std::cout<<"basic_adjustments: ti="<<ti<<" corner="<<r->left<<","<<r->top<<std::endl;
      for( y = 0; y < height; y++ ) {
        pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x+=3 ) {

          RGB[0] = pin[x];
          RGB[1] = pin[x+1];
          RGB[2] = pin[x+2];

          if( is_log ) {
            RGB[0] = lin_to_ACEScc(RGB[0]);
            RGB[1] = lin_to_ACEScc(RGB[1]);
            RGB[2] = lin_to_ACEScc(RGB[2]);
          }

          RGB[0] *= r_slope;
          RGB[1] *= g_slope;
          RGB[2] *= b_slope;

          RGB[0] += r_offs;
          RGB[1] += g_offs;
          RGB[2] += b_offs;

          pout[x] = (RGB[0]<0) ? RGB[0] : powf(RGB[0], r_pow);
          pout[x+1] = (RGB[1]<0) ? RGB[1] : powf(RGB[1], g_pow);
          pout[x+2] = (RGB[2]<0) ? RGB[2] : powf(RGB[2], b_pow);

          if( is_log ) {
            pout[x] = ACEScc_to_lin(pout[x]);
            pout[x+1] = ACEScc_to_lin(pout[x+1]);
            pout[x+2] = ACEScc_to_lin(pout[x+2]);
          }

          if(profile && saturation != 1) {
            L = profile->get_lightness(pout[x], pout[x+1], pout[x+2]);
            pout[x] = L + saturation * (pout[x] - L);
            pout[x+1] = L + saturation * (pout[x+1] - L);
            pout[x+2] = L + saturation * (pout[x+2] - L);
          }
        }
      }
    }
  };




  ProcessorBase* new_color_correction();
}

#endif 


