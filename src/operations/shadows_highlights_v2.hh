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

#ifndef PF_SHAHI_V2_H
#define PF_SHAHI_V2_H

//#ifdef __SSE2__
//#include "../rt/rtengine/sleefsseavx.c"
//#else
#include "../rt/rtengine/sleef.c"
//#endif

#include "../base/processor.hh"

#define SH_LOG_SCALE_MIN (-2.0f)
#define SH_LOG_SCALE_MAX (1.0f)

namespace PF
{


class ShadowsHighlightsV2Par: public OpParBase
{
  Property<float> shadows, highlights;
  Property<float> radius, threshold;

  Property<bool> show_residual;
  bool show_residual_;

  ProcessorBase* loglumi;
  ProcessorBase* guided[10];

  PF::ICCProfile* in_profile;
  float radius_scale[10];
  float threshold_scale[10];


public:
  ShadowsHighlightsV2Par();

  bool has_intensity() { return false; }
  bool needs_caching();

  PF::ICCProfile* get_profile() { return in_profile; }

  float get_shadows() { return shadows.get(); }
  float get_highlights() { return highlights.get(); }
  bool get_show_residual() { return show_residual_; }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  void propagate_settings();

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class ShadowsHighlightsV2Proc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class ShadowsHighlightsV2Proc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 2 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;
    //if( ireg[2] == NULL ) return;

    ShadowsHighlightsV2Par* opar = dynamic_cast<ShadowsHighlightsV2Par*>(par);
    if( !opar ) return;

    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    float* pin0;
    float* pin1;
    float* pin2;
    float* pout;
    //typename FormatInfo<T>::SIGNED diff;
    float R, out;
    int x, x0, y, pos;
    //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;
    float bias = profile->perceptual2linear(0.5);

    const float SH_LOG_SCALE_RANGE = SH_LOG_SCALE_MAX - SH_LOG_SCALE_MIN;

    float sh_scale = 1.0f / opar->get_shadows();
    float hl_scale = 1.0f / opar->get_highlights();

    //std::cout<<"sh_scale="<<sh_scale<<"  hl_scale="<<hl_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      // original image
      pin0 = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      // log-lumi image
      pin1 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      // blurred log-lumi image
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      // output image
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x0 = 0, x = 0; x < line_size; x+=3, x0++ ) {
        float l2 = pin2[x0];
        //l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
        //l2 = xexp10( l2 );
        //out = (l2 > 1) ? pow(l2, hl_scale) : pow(l2, sh_scale);
        out = (l2 > 0) ? l2 * hl_scale : l2 * sh_scale;
        l2 = xexp10( l2 );
        out = xexp10( out );
        R = out / l2;

        pout[x] = pin0[x] * R;
        pout[x+1] = pin0[x+1] * R;
        pout[x+2] = pin0[x+2] * R;

        if( opar->get_show_residual() ) pout[x] = pout[x+1] = pout[x+2] = l2 * bias;

        /*
        //float l1 = profile->get_lightness(pin0[x], pin0[x+1], pin0[x+2]);
        float l1 = pin1[x0];
        l1 = ( l1*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
        l1 = xexp10( l1 );
        //l1 *= bias;
        float l2 = pin2[x0];
        l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
        l2 = xexp10( l2 );
        //l2 *= bias;
        //l2 = floorf(l2 * 255) / 255.0f;
        float diff = l1 - l2;
        float diff_gain = 1.5;
        diff *= diff_gain;
        if( std::fabs(diff) < 1.0f ) {
          float exp_max = 4;
          float exponent = (exp_max - 1) * (1.0f - std::fabs(diff)) + 1.0f;
          if(diff >= 0) {
            diff = pow(diff, exponent);
          } else {
            diff = -1.0f * pow(-1.0f * diff, exponent);
          }
        }
        diff /= diff_gain;
        l2 += diff;

        //l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;

        //out = (l2 > 0) ? l2 * hl_scale : l2 * sh_scale;
        //out = xexp10( out );
        out = (l2 > 1) ? pow(l2, hl_scale) : pow(l2, sh_scale);
        //out *= bias;

        //l2 = xexp10( l2 ) * bias;
        R = out / l2;
        //std::cout<<"render: l1="<<l1<<std::endl;

        pout[x] = pin0[x] * R;
        pout[x+1] = pin0[x+1] * R;
        pout[x+2] = pin0[x+2] * R;

        if( opar->get_show_residual() ) pout[x] = pout[x+1] = pout[x+2] = l2 * bias;
        */
      }
    }
  }
};


ProcessorBase* new_shadows_highlights_v2();

}

#endif 


