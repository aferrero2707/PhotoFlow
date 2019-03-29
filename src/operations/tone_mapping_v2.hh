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

#ifndef PF_TONE_MAPPING_V2_H
#define PF_TONE_MAPPING_V2_H

#include <string>

//#include <glibmm.h>

#include "../base/processor.hh"
#include "Filmic/FilmicCurve/FilmicToneCurve.h"


namespace PF 
{


#define GamutMapNYPoints 1000


enum tone_mapping_method2_t
{
  TONE_MAPPING_LIN_EXP,
};


enum tone_mapping_blend_t
{
  TONE_MAPPING_BLEND_NORMAL,
  TONE_MAPPING_BLEND_HUE,
  TONE_MAPPING_BLEND_LUMINANCE
};


enum tone_mapping_preset_t
{
  TONE_MAPPING_PRESET_CUSTOM = -1,
  TONE_MAPPING_PRESET_BASE_CONTRAST = 0,
  TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST = 1,
  TONE_MAPPING_PRESET_HIGH_CONTRAST = 2,
  TONE_MAPPING_PRESET_VERY_HIGH_CONTRAST = 3,
  TONE_MAPPING_PRESET_LINEAR = 8,
  TONE_MAPPING_PRESET_S_CURVE = 9,
  TONE_MAPPING_PRESET_LAST = 10
};


struct LE_params_t
{
  float LE_gain;
  float LE_Ymidgray;
  float LE_slope;
  float LE_slope2;
  float LE_linmax;
  float LE_linmax2;
  float LE_Ylinmax;
  float LE_Srange;
  float LE_compr;
  float LE_Sslope;
  float LE_Kstrength;
  float LE_Kmax;
  float LE_Kymax;
  float LE_Kexp;
};


//void init_LE_params(LE_params_t& par, )



inline float SH_HL_mapping_pow( const float& in, const float& sh_compr, const float& hl_compr, const float& pivot, bool verbose=false )
{
  float d_compr = hl_compr - sh_compr;
  float nRGB = in/pivot;
  float ex = (nRGB > 1) ? nRGB - 1 : log(nRGB + 1.0e-15);
  float ex2 = atan(ex);
  float exponent = ex2 * d_compr / M_PI + sh_compr + d_compr/2;
  //float norm = (nRGB > 1) ? exp( (1-nRGB)*1 ) * (log_scale_sh2-log_scale_hl2)+log_scale_hl2 : log_scale_sh2;
  float norm2 = pow(pivot,1.0f/(exponent+1));
  return pow(in,1.0f/(exponent+1)) * pivot / norm2;
}


inline float SH_HL_mapping_log( const float& in, const float& sh_compr, const float& hl_compr, const float& pivot, bool verbose=false )
{
  if( sh_compr == 0 && hl_compr == 0 ) return in;
  //if(in>1.5) return in;
  float d_compr = hl_compr - sh_compr;
  float nRGB = in/pivot;
  float ex = d_compr * atan((nRGB-1)*4) / M_PI + (hl_compr + sh_compr)/2;
  float base = ex + 1.0e-10;// * ex;
  //float base = 1.0f - ex/(ex+1) + 1.0e-15;
  //float norm = (nRGB > 1) ? exp( (1-nRGB)*1 ) * (log_scale_sh2-log_scale_hl2)+log_scale_hl2 : log_scale_sh2;
  float norm = log(pivot*base+1);
  float result = log(in*base+1)*pivot/norm;
  if(verbose)
    std::cout<<"in="<<in<<" pivot="<<pivot<<" nRGB="<<nRGB<<" (nRGB-1)*4="<<(nRGB-1)*4<<" ex="<<ex<<" base="<<base<<" norm="<<norm<<" result="<<result<<std::endl;
  return result;
}

float SH_HL_mapping( const float& in, const float& sh_compr, const float& hl_compr, const float& pivot, bool verbose=false );



struct TM_lin_exp_params_t
{
  float LE_midgray;
  float LE_gain;
  float LE_Ymidgray;
  float LE_slope;
  float LE_slope2;
  float LE_linmax;
  float LE_linmax2;
  float LE_Ylinmax;
  float LE_Srange;
  float LE_compr;
  float LE_compr2;
  float LE_Sslope;
  float LE_Sslope2;
  float LE_Kstrength;
  float LE_Kmax;
  float LE_Kymax;
  float LE_Kexp;
  void init(float gain, float slope, float lin_max, float knee_strength,
      float compression, float shoulder_slope);
};


void TM_lin_exp(const PF::TM_lin_exp_params_t& par, const float& val, float& result);



class ToneMappingParV2: public OpParBase
{
  PropertyBase preset;
  Property<bool> hue_protection;

  Property<float> LE_gain, LE_compression, LE_slope, LE_lin_max, LE_knee_strength, LE_shoulder_slope, LE_shoulder_slope2, LE_shoulder_max;
  TM_lin_exp_params_t LE_params;
  float LE_gain_presets[10];
  float LE_slope_presets[10];
  float LE_lin_max_presets[10];
  float LE_knee_strength_presets[10];
  float LE_shoulder_slope_presets[10];
  float LE_shoulder_max_presets[10];

  Property<float> lumi_blend_frac;

  Property<float> saturation_scaling, hl_desaturation;

  Property<float> local_contrast_amount, local_contrast_radius, local_contrast_threshold;
  ProcessorBase* guided_blur;

  ICCProfile* icc_data;

public:

  float gamut_boundary[GamutMapNYPoints+1][360];

  ToneMappingParV2();

  tone_mapping_preset_t get_preset() { return (tone_mapping_preset_t)preset.get_enum_value().first; }
  void set_preset( tone_mapping_preset_t p ) {
    preset.set_enum_value( p );
  }

  bool get_hue_protection() { return hue_protection.get(); }
  float get_saturation_scaling() { return saturation_scaling.get(); }
  float get_hl_desaturation() { return hl_desaturation.get(); }

  float get_LE_gain() { return LE_gain.get(); }
  float get_LE_compression() { return LE_compression.get(); }
  float get_LE_slope() { return LE_slope.get(); }
  float get_LE_lin_max() { return LE_lin_max.get(); }
  float get_LE_knee_strength() { return LE_knee_strength.get(); }
  float get_LE_shoulder_slope() { return LE_shoulder_slope.get(); }
  float get_LE_shoulder_slope2() { return LE_shoulder_slope2.get(); }

  float get_lumi_blend_frac() { return lumi_blend_frac.get(); }

  float get_local_contrast_amount() { return local_contrast_amount.get(); }

  ICCProfile* get_icc_data() { return icc_data; }

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_input() { return true; }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  void propagate_settings();

  void pre_build( rendermode_t mode );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
                   VipsImage* imap, VipsImage* omap, unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class ToneMappingV2
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};



template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class ToneMappingV2< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingParV2* opar = dynamic_cast<ToneMappingParV2*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    float lumi_blend_frac = opar->get_lumi_blend_frac();
    ICCProfile* prof = opar->get_icc_data();

    float* pin;
    float* psmooth;
    float* pout;
    float RGB[4];
    float RGBin[4];
    float dRGB[4];
    float rRGB[4];
    float Lab[3];
    int x, y, k;

    const float minus = -1.f;

    TM_lin_exp_params_t LE_params;
    LE_params.init(opar->get_LE_gain(), opar->get_LE_slope(), opar->get_LE_lin_max(),
        opar->get_LE_knee_strength(), opar->get_LE_compression(), opar->get_LE_shoulder_slope());

/*
    float LE_midgray = pow(0.5,2.45);
    float LE_gain = opar->get_LE_gain();
    float LE_Ymidgray = LE_midgray * LE_gain;
    float LE_slope = opar->get_LE_slope();
    float LE_slope2 = LE_slope * LE_gain;
    float LE_linmax = pow(opar->get_LE_lin_max(),2.45);
    //float LE_linmax2 = SH_HL_mapping( LE_linmax, sh_compr, hl_compr, log_pivot );
    float LE_linmax2 = LE_linmax;
    float LE_Ylinmax = ( LE_linmax2 - LE_midgray ) * LE_slope2 + LE_Ymidgray;
    float LE_Srange = 1.0f - LE_Ylinmax;
    float LE_compr = opar->get_LE_compression();
    float LE_compr2 = LE_compr*2.5f;
    float LE_Sslope = opar->get_LE_shoulder_slope();
    float LE_Sslope2 = opar->get_LE_shoulder_slope2();
    float LE_Kstrength = opar->get_LE_knee_strength() * ( (LE_slope-1)*1 + 1 );
    float LE_Kmax = ((1.f-LE_slope)*LE_midgray)/(LE_slope/(sqrt(2.0f)*LE_Kstrength)-LE_slope);
    float LE_Kymax = (LE_Kmax-LE_midgray)*LE_slope + LE_midgray;
    float LE_Kexp = LE_Kmax * LE_slope / LE_Kymax;
*/
    float HL_desat = opar->get_hl_desaturation();

    ICCProfile* labprof = ICCStore::Instance().get_Lab_profile();
    ICCTransform rgb2lab, lab2rgb;
    rgb2lab.init(prof, labprof, VIPS_FORMAT_FLOAT);
    lab2rgb.init(labprof, prof, VIPS_FORMAT_FLOAT);

    float lc = opar->get_local_contrast_amount();

    //std::cout<<"gamma = "<<gamma<<std::endl;
    //std::cout<<"log(0.2*gamma+1) / gamma_scale = "<<log(0.2*gamma+1) / gamma_scale<<std::endl;
    //std::cout<<"log(gamma+1) / gamma_scale = "<<log(gamma+1) / gamma_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      psmooth = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pin = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {

        //pout[x] = psmooth[x];
        //pout[x+1] = psmooth[x+1];
        //pout[x+2] = psmooth[x+2];
        //continue;

        RGBin[0] = psmooth[x]*lc   + pin[x]*(1.0f-lc); if( RGBin[0] < 0 ) RGBin[0] = 0;
        dRGB[0] = (/*RGBin[0] > -1.0e-16 &&*/ RGBin[0] < 1.0e-16) ? 1 : ((pin[x]<0) ? 0 : pin[x]) / RGBin[0];


        RGBin[1] = psmooth[x+1]*lc + pin[x+1]*(1.0f-lc); if( RGBin[1] < 0 ) RGBin[1] = 0;
        dRGB[1] = (/*RGBin[1] > -1.0e-16 &&*/ RGBin[1] < 1.0e-16) ? 1 : ((pin[x+1]<0) ? 0 : pin[x+1]) / RGBin[1];

        RGBin[2] = psmooth[x+2]*lc + pin[x+2]*(1.0f-lc); if( RGBin[2] < 0 ) RGBin[2] = 0;
        dRGB[2] = (/*RGBin[2] > -1.0e-16 &&*/ RGBin[2] < 1.0e-16) ? 1 : ((pin[x+2]<0) ? 0 : pin[x+2]) / RGBin[2];

        RGBin[3] = prof->get_lightness(RGBin[0],RGBin[1],RGBin[2]);
        /*
        std::cout<<"pin: "<<pin[x]<<" "<<pin[x+1]<<" "<<pin[x+2]<<" "<<std::endl;
        std::cout<<"Lab: "<<Lab[0]<<" "<<Lab[1]<<" "<<Lab[2]<<" "<<std::endl;
        std::cout<<"RGB: "<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<" "<<std::endl;
        std::cout<<"dRGB: "<<dRGB[0]<<" "<<dRGB[1]<<" "<<dRGB[2]<<" "<<std::endl;
        std::cout<<std::endl;
        */
        for( k=0; k < 3; k++)
          RGB[k] = RGBin[k];

        if(RGB[0] < 0) RGB[0] = 0;
        if(RGB[1] < 0) RGB[1] = 0;
        if(RGB[2] < 0) RGB[2] = 0;
        float min = MIN3(RGB[0], RGB[1], RGB[2]);
        float max = MAX3(RGB[0], RGB[1], RGB[2]);
        int max_id = 0;
        if( RGB[1] > RGB[max_id]) max_id = 1;
        if( RGB[2] > RGB[max_id]) max_id = 2;
        //float S = (max > 1.0e-10) ? (max - min) / max : 0;
        //if( S < 0 ) S = 0; if( S > 1 ) S = 1;
        //S = powf(S,2.45);
        //float K = 1.0f - opar->get_saturation_scaling()*S;
        float K = 0;

        float saturation = 1;

        for( k=0; k < 3; k++) {
          //std::cout<<"lin+exp: RGB["<<k<<"] in:  "<<RGB[k]<<std::endl;
          //RGB[k] *= K;
          float out; TM_lin_exp(LE_params, RGB[k], out);

          if( k == max_id && /*HL_desat > 0 &&*/ RGB[k] > LE_params.LE_linmax2 ) {
            // reduced saturation proportionally to the compression factor
            float lRGB = (RGB[k] - LE_params.LE_midgray) * LE_params.LE_slope2 + LE_params.LE_Ymidgray;
            K = 1.0f; // - out/lRGB;
            //saturation = powf(out/lRGB, HL_desat);
            //std::cout<<"k="<<k<<"  RGBin="<<RGBin[k]<<"  RGB="<<RGB[k]<<"  lRGB="<<lRGB<<"  saturation="<<saturation<<std::endl;
          }
/*
          float lRGB = (RGB[k] - LE_midgray) * LE_slope2 + LE_Ymidgray;
          if( RGB[k] > LE_linmax2 ) {
            // shoulder
            float X = (RGB[k] - LE_linmax2) * LE_slope2 * LE_compr / LE_Srange;
            //float XD = pow(X,LE_Sslope2) * LE_Sslope  + 1;
            ////RGB[k] = 1.0f - LE_Srange * exp( -X / XD );
            //RGB[k] = (LE_Srange - LE_Srange * exp( -X / XD )) / LE_compr + LE_Ylinmax;

            float result = (LE_Srange - LE_Srange * exp( -log(X*(LE_Sslope)+1)/(LE_Sslope) )) / LE_compr + LE_Ylinmax;
            //if( RGB[k]>1) std::cout<<"RGB[k]="<<RGB[k]<<"  X="<<X<<"  result="<<result<<std::endl;
            RGB[k] = result;

            if( k == max_id && HL_desat > 0 ) {
              // reduced saturation proportionally to the compression factor
              saturation = powf(RGB[k] / lRGB, HL_desat);
              //std::cout<<"k="<<k<<"  RGBin="<<RGBin[k]<<"  RGB="<<RGB[k]<<"  lRGB="<<lRGB<<"  saturation="<<saturation<<std::endl;
            }
          } else if( RGB[k] < LE_Kmax ) {
            // knee
            float X = RGB[k] / LE_Kmax;
            RGB[k] = (X>=0) ? LE_Kymax * pow(X,LE_Kexp) : -LE_Kymax * pow(-X,LE_Kexp);
          } else {
            // linear part
            RGB[k] = lRGB; //(RGB[k] - LE_midgray) * LE_slope2 + LE_Ymidgray;
          }
          //RGB[k] *= K;
          //RGB[k] *= rRGB[k];
          //std::cout<<"lin+log: RGB["<<k<<"] out: "<<RGB[k]<<std::endl;
*/
          //std::cout<<"RGBin="<<RGBin[k]<<"  old="<<RGB[k]<<"  new="<<out<<"  diff="<<out-RGB[k]<<std::endl;
          RGB[k] = out;
        }

        //std::cout<<"LIN_POW: "<<LE_midgray<<" -> "<<RGB[3]<<std::endl;
        if( saturation != 1 ) {
          for( k=0; k < 3; k++) {
            RGB[k] = RGB[max_id] + saturation * (RGB[k] - RGB[max_id]);
          }
        }

        if( lumi_blend_frac > 0 && K > 0) {
          float lumi_blend_frac2 = K * lumi_blend_frac;
          float Rmax = (RGBin[max_id]>1.0e-10) ? RGB[max_id] / RGBin[max_id] : 0;
          for( k=0; k < 3; k++) {
            float RGBout = RGBin[k] * Rmax;
            RGB[k] = lumi_blend_frac2 * RGBout + (1.0f - lumi_blend_frac2) * RGB[k];
          }
        }

        if( prof && opar->get_hue_protection() ) {
          float Jab_in[3], JCH_in[3];
          float Jab[3], JCH[3];
          float Jz, az, bz, C, H;
          // convert to Jzazbz in polar coordinates
          prof->to_Jzazbz(RGBin[0], RGBin[1], RGBin[2], Jab_in[0], Jab_in[1], Jab_in[2]);
          PF::Lab2LCH(Jab_in, JCH_in, 1);

          //std::cout<<"RGBin:  "<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<" "<<std::endl;
          //std::cout<<"Jabin:  "<<Jab_in[0]<<" "<<Jab_in[1]<<" "<<Jab_in[2]<<" "<<std::endl;
          //std::cout<<"JCHin:  "<<JCH_in[0]<<" "<<JCH_in[1]<<" "<<JCH_in[2]<<" "<<std::endl;

          float max_before = MAX3(RGB[0], RGB[1], RGB[2]);
          prof->to_Jzazbz(RGB[0], RGB[1], RGB[2], Jab[0], Jab[1], Jab[2]);
          PF::Lab2LCH(Jab, JCH, 1);

          JCH[2] = JCH_in[2];
          //if( opar->get_gamut_compression() )
          //  prof->chroma_compression(JCH[0], JCH[1], JCH[2]);

          //std::cout<<"JCHout: "<<JCH[0]<<" "<<JCH[1]<<" "<<JCH[2]<<" "<<std::endl;
          PF::LCH2Lab(JCH, Jab, 1);
          //std::cout<<"Jabout: "<<Jab[0]<<" "<<Jab[1]<<" "<<Jab[2]<<" "<<std::endl;
          prof->from_Jzazbz( Jab[0], Jab[1], Jab[2], RGB[0], RGB[1], RGB[2] );
          //std::cout<<"RGBout: "<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<" "<<std::endl;
          float max_after = MAX3(RGB[0], RGB[1], RGB[2]);

          if(max_after > 1.0e-10) {
            float R = max_before / max_after;
            RGB[0] *= R;
            RGB[1] *= R;
            RGB[2] *= R;
          }

          //pout[x] = RGB[0];
          //pout[x+1] = RGB[1];
          //pout[x+2] = RGB[2];
        }
        pout[x] = RGB[0] * dRGB[0];
        pout[x+1] = RGB[1] * dRGB[1];
        pout[x+2] = RGB[2] * dRGB[2];
        //std::cout<<"dRGB: "<<dRGB[0]<<" "<<dRGB[1]<<" "<<dRGB[2]<<" "<<std::endl;
        //std::cout<<"pout: "<<pout[x+0]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<std::endl;
      }
    }
  }
};




template < OP_TEMPLATE_DEF_CS_SPEC >
class ToneMappingV2< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingParV2* opar = dynamic_cast<ToneMappingParV2*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    T* pin;
    T* pout;
    int x, y;

    float a, b;

    for( y = 0; y < height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {

        pout[x] = pin[x];
        pout[x+1] = pin[x+1];
        pout[x+2] = pin[x+2];
      }
    }
  }
};



ProcessorBase* new_tone_mapping_v2();
}

#endif 


