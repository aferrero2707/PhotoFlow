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
  Property<float> amount, shadows, highlights, anchor, shadows_range, highlights_range, contrast, contrast_threshold;
  Property<float> radius, threshold;

  Property<bool> show_residual, single_scale_blur;
  int median_radius;
  bool show_residual_;

  ProcessorBase* loglumi;
  ProcessorBase* guided[10];

  PF::ICCProfile* in_profile;
  float threshold_scale[10];


public:
  ShadowsHighlightsV2Par();

  bool has_intensity() { return false; }
  bool needs_caching();

  PF::ICCProfile* get_profile() { return in_profile; }

  float get_amount() { return amount.get(); }
  float get_shadows() { return shadows.get(); }
  float get_shadows_range() { return shadows_range.get(); }
  float get_highlights() { return highlights.get(); }
  float get_highlights_range() { return highlights_range.get(); }
  float get_contrast() { return contrast.get(); }
  float get_contrast_threshold() { return contrast_threshold.get(); }
  float get_anchor() { return anchor.get(); }
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
    if( n != 3 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;
    if( ireg[2] == NULL ) return;

    ShadowsHighlightsV2Par* opar = dynamic_cast<ShadowsHighlightsV2Par*>(par);
    if( !opar ) return;

    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    VipsRect *r = &oreg->valid;
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
    float bias = profile->perceptual2linear( opar->get_anchor() );
    float lwhite = xlog10(0.9f/bias);

    const float SH_LOG_SCALE_RANGE = SH_LOG_SCALE_MAX - SH_LOG_SCALE_MIN;

    //float amount = 1.0f / opar->get_amount();
    float sh_scale = 1.0f / opar->get_shadows();
    float hl_scale = 1.0f / opar->get_highlights();
    float scale_delta = hl_scale - sh_scale;
    float sh_range = opar->get_shadows_range();
    float hl_range = opar->get_highlights_range();

    //std::cout<<"sh_scale="<<sh_scale<<"  hl_scale="<<hl_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      // original image
      pin0 = (float*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
      // log-lumi image
      pin1 = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      // blurred log-lumi image
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      // output image
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x0 = 0, x = 0; x < line_size; x+=3, x0++ ) {
        float l1 = pin1[x0];
        float l2 = pin2[x0];
        float delta = l1 - l2;
        float scale = opar->get_contrast();
        //l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
        //l2 = xexp10( l2 );
        //out = (l2 > 1) ? pow(l2, hl_scale) : pow(l2, sh_scale);
        if( l2 < 0 ) {
          float sh_slope = (1.0f - sh_scale) / (1.0f + l2*l2/sh_range);
          l2 *= (1.0f - sh_slope);
          //out = l2 * sh_scale;
        } else {
          //float hl_slope = (1.0f - hl_scale) / (1.0f + l2*l2/hl_range);
          //out = l2 * (1.0f - hl_slope);
          //out = l2 * hl_scale;
          //float hl_slope = (1.0f - 1.0f/(l2*hl_range+1)) * scale_delta + sh_scale;
          //float M_PI2 = M_PI/2;
          //float hl_frac = (l2<hl_range) ? sin(l2*M_PI2/hl_range) : 1;
          float l22 = 1.0f - (l2/hl_range);
          float hl_frac = (l2<hl_range) ? (1.0f - l22*l22) : 1;
          float hl_slope = hl_frac * scale_delta + sh_scale;
          l2 *= hl_slope;
        }

        float delta2 = delta;
        if( delta < 0 ) {
          float boost = 2.0f - (lwhite - l2)/lwhite;
          if(boost < 0) boost = 0;
          if(boost > 2) boost = 2;
          //boost *= 2;
          scale *= boost;
        }
        if( delta > 0 ) {
          float boost = (lwhite - l2)/lwhite;
          if(boost < 0) boost = 0;
          if(boost > 1.5) boost = 1.5;
          //boost *= 2;
          scale *= boost;
        }
        delta2 *= scale;
        out = delta2 + delta + l2;

        //out = (l2 > 0) ? l2 * hl_scale : l2 * sh_scale;
        l1 = xexp10( l1 );
        out = xexp10( out );
        R = out / l1;

        pout[x] = pin0[x] * R;
        pout[x+1] = pin0[x+1] * R;
        pout[x+2] = pin0[x+2] * R;

        //std::cout<<"[x,y]="<<x0+r->left<<","<<y+r->top<<"  pin0: "<<pin0[x]<<"  pin1: "<<pin1[x]<<"  pin2: "<<pin2[x]<<"  delta: "<<delta<<"  scale: "<<scale<<"  lwhite: "<<lwhite<<"  delta2: "<<delta2<<"  R="<<R<<"  pout="<<pout[x]<<std::endl;

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


  void render2(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n <= 2 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;
    if( ireg[2] == NULL ) return;

    ShadowsHighlightsV2Par* opar = dynamic_cast<ShadowsHighlightsV2Par*>(par);
    if( !opar ) return;

    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    int width = r->width;
    int height = r->height;

    float* pin0;
    float* pin1;
    float* pin2;
    float* pin_mm;
    float* pout;
    //typename FormatInfo<T>::SIGNED diff;
    float R, out;
    int x, x0, x_mm, y, pos;
    //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;
    float bias = profile->perceptual2linear( opar->get_anchor() );
    float lwhite = xlog10(0.9f/bias);

    const float SH_LOG_SCALE_RANGE = SH_LOG_SCALE_MAX - SH_LOG_SCALE_MIN;

    //float amount = 1.0f / opar->get_amount();
    float sh_scale = 1.0f / opar->get_shadows();
    float hl_scale = 1.0f / opar->get_highlights();
    float scale_delta = hl_scale - sh_scale;
    float sh_range = opar->get_shadows_range();
    float hl_range = opar->get_highlights_range();

    float lmean = xlog10(opar->get_contrast_threshold() / bias);

    int winsize = 5;

    //std::cout<<"sh_scale="<<sh_scale<<"  hl_scale="<<hl_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      // original image
      pin0 = (float*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
      // log-lumi image
      pin1 = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      // min/max/var image
      pin_mm = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      // blurred log-lumi image
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      // output image
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x0 = 0, x = 0, x_mm = 0; x < line_size; x+=3, x0++, x_mm += 3 ) {
        float l0 = pin0[x0];
        float l1 = pin1[x0];
        float l2 = pin2[x0];
        //l2 = lmean;
        float l1threshold = 0.1f*lwhite;
        //if(l2>0) l2 *= opar->get_contrast_threshold();
        if(false && l2>0 && l1>l2) {
          if(l1>=lwhite) {
            l2 = l1;
          } else {
            //float mix = (lwhite-l1)*2/(lwhite-l2);
            float mix = 1.0f - l2/lwhite;
            if(mix>1) mix=1;
            if(mix<0) mix=0;
          mix *= mix;
          //mix = sqrt(mix);
          //std::cout<<"l1="<<l1<<"  l2="<<l2<<"  lwhite="<<lwhite<<"  mix="<<mix<<"  l2="<<mix * l2 + (1.0f - mix) * l1<<std::endl;
          l2 = mix * l2 + (1.0f - mix) * l1;
          }
        }
        //if(l2>0) l2 *= opar->get_contrast_threshold();

        // local contrast
        //l1 = (l1-l2)*2 + l2;

        float min = pin_mm[x_mm];
        float max = pin_mm[x_mm+1];
        float var = pin_mm[x_mm+2];
        /*
        int xmin = (x0 <= winsize) ? 0 : x0-winsize;
        int xmax = (x0 >= (width-winsize)) ? width : x0+winsize;
        for(int x1 = xmin; x1 < xmax; x1++) {
          if(pin1[x1] < min) min = pin1[x1];
          if(pin1[x1] > max) max = pin1[x1];
        }
        */
        float delta = var*10; //max - min;
        float scale = opar->get_contrast_threshold() / delta;
        float scale_max = opar->get_contrast();
        if( scale > scale_max ) scale = scale_max;
        //if( scale < 1 ) scale = 1;
        scale = scale_max;
        if( l1 > l2 && l2 > 0 ) {
          float damp_factor = (lwhite-l2)/lwhite;
          //damp_factor = sqrt(damp_factor);
          damp_factor = damp_factor*damp_factor;
          //scale = (scale-1)*damp_factor + 1;
          //scale = 1;
          //if( out > out_max) out = out_max;
          //std::cout<<"l1="<<l1<<"  l2="<<l2<<"  lwhite="<<lwhite<<"  min="<<min<<"  max="<<max<<"  delta="<<delta<<"  damp_factor="<<damp_factor<<"  scale="<<scale<<std::endl;
        }
        //l2 = xlog10(0.5/bias);
        delta = (l1 - l2)*1;
        float delta2 = delta;
        l2 *= 0.9;
        if( delta < 0 ) {
          float boost = 2.0f - (lwhite - l2)/lwhite;
          if(boost < 0) boost = 0;
          if(boost > 1.5) boost = 1.5;
          //boost *= 2;
          scale *= boost;
        }
        if( delta > 0 ) {
          float boost = (lwhite - l2)/lwhite;
          if(boost < 0) boost = 0;
          if(boost > 1.5) boost = 1.5;
          //boost *= 2;
          scale *= boost;
        }
        //if(delta > 0) delta2 = delta / (delta*delta + 1);
        //else delta2 = delta / (delta*delta + 1);
        delta2 /= 1;
        //out = l1 - l2;
        //if( out>0 ) out = log1p(out)*scale;
        //else out *= scale;
        delta2 *= scale;
        out = delta2 + delta + l2;

        //std::cout<<"l1="<<l1<<"  min="<<min<<"  max="<<max<<"  l2="<<l2<<"  delta="<<delta<<"  delta2="<<delta2<<"  scale="<<scale<<"  out="<<out<<std::endl;
        //out = (l2 > 0) ? l2 * hl_scale : l2 * sh_scale;
        l2 = xexp10( l2 );
        l1 = xexp10( l1 );
        out = xexp10( out );
        R = out / l1;

        //std::cout<<"var="<<var<<"  l1="<<l1<<"  out="<<out<<"  R="<<R<<std::endl;

        pout[x] = pin0[x] * R;
        pout[x+1] = pin0[x+1] * R;
        pout[x+2] = pin0[x+2] * R;

        if( opar->get_show_residual() ) pout[x] = pout[x+1] = pout[x+2] = l2 * bias;
      }
    }
  }
};


ProcessorBase* new_shadows_highlights_v2();

}

#endif 


