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




class ToneMappingParV2: public OpParBase
{
  PropertyBase method;
  Property<float> exposure;
  Property<float> log_sh, log_hl;
  Property<float> log_pivot;
  Property<bool> hue_protection;
  Property<bool> gamut_compression;

  Property<float> filmic_A;
  Property<float> filmic_B;
  Property<float> filmic_C;
  Property<float> filmic_D;
  Property<float> filmic_E;
  Property<float> filmic_F;
  Property<float> filmic_W;

  Property<bool> filmic2_preserve_midgray;
  Property<float> filmic2_gamma;
  Property<float> filmic2_TS;
  Property<float> filmic2_TL;
  Property<float> filmic2_SS;
  Property<float> filmic2_SL;
  Property<float> filmic2_SA;

  Property<float> AL_Lmax, AL_b, AL_Tsize, AL_Tlength, AL_Tstrength;

  Property<float> LP_compression, LP_slope, LP_lin_max, LP_knee_strength, LP_shoulder_smoothness;

  Property<float> LE_compression, LE_slope, LE_lin_max, LE_knee_strength, LE_shoulder_slope, LE_shoulder_slope2;

  Property<float> HD_slope, HD_toe_range, HD_shoulder_range;

  Property<float> gamut_compression_amount, gamut_compression_exponent;
  Property<float> lumi_blend_frac;

  ICCProfile* icc_data;

  float exponent, filmic2_exponent;

public:

  float gamut_boundary[GamutMapNYPoints+1][360];

  ToneMappingParV2();

  tone_mapping_method2_t get_method() { return (tone_mapping_method2_t)method.get_enum_value().first; }
  float get_exposure() { return exposure.get(); }
  float get_log_sh() { return( log_sh.get() ); }
  float get_log_hl() { return( log_hl.get() ); }
  float get_log_pivot() { return log_pivot.get(); }
  float get_exponent() { return exponent; }
  bool get_gamut_compression() { return gamut_compression.get(); }
  bool get_hue_protection() { return hue_protection.get(); }

  float get_filmic_A() { return filmic_A.get(); }
  float get_filmic_B() { return filmic_B.get(); }
  float get_filmic_C() { return filmic_C.get(); }
  float get_filmic_D() { return filmic_D.get(); }
  float get_filmic_E() { return filmic_E.get(); }
  float get_filmic_F() { return filmic_F.get(); }
  float get_filmic_W() { return filmic_W.get(); }

  bool get_filmic2_preserve_midgray() { return filmic2_preserve_midgray.get(); }
  float get_filmic2_gamma() { return filmic2_gamma.get(); }
  float get_filmic2_exponent() { return filmic2_exponent; }
  float get_filmic2_TS() { return filmic2_TS.get(); }
  float get_filmic2_TL() { return filmic2_TL.get(); }
  float get_filmic2_SS() { return filmic2_SS.get(); }
  float get_filmic2_SL() { return filmic2_SL.get(); }
  float get_filmic2_SA() { return filmic2_SA.get(); }

  float get_AL_Lmax() { return AL_Lmax.get(); }
  float get_AL_b() { return AL_b.get(); }
  float get_AL_Tsize() { return AL_Tsize.get(); }
  float get_AL_Tlength() { return AL_Tlength.get(); }
  float get_AL_Tstrength() { return AL_Tstrength.get(); }

  float get_LP_compression() { return LP_compression.get(); }
  float get_LP_slope() { return LP_slope.get()+1.0e-10; }
  float get_LP_lin_max() { return LP_lin_max.get(); }
  float get_LP_knee_strength() { return LP_knee_strength.get(); }
  float get_LP_shoulder_smoothness() { return LP_shoulder_smoothness.get(); }

  float get_LE_compression() { return LE_compression.get()+1.0e-5; }
  float get_LE_slope() { return LE_slope.get()+1.0e-10; }
  float get_LE_lin_max() { return LE_lin_max.get(); }
  float get_LE_knee_strength() { return LE_knee_strength.get(); }
  float get_LE_shoulder_slope() { return LE_shoulder_slope.get(); }
  float get_LE_shoulder_slope2() { return LE_shoulder_slope2.get(); }

  float get_HD_slope() { return HD_slope.get(); }
  float get_HD_toe_range() { return HD_toe_range.get(); }
  float get_HD_shoulder_range() { return HD_shoulder_range.get();}

  float get_gamut_compression_amount() { return gamut_compression_amount.get(); }
  float get_gamut_compression_exponent() { return gamut_compression_exponent.get(); }
  float get_lumi_blend_frac() { return lumi_blend_frac.get(); }

  ICCProfile* get_icc_data() { return icc_data; }

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_input() { return true; }

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

    tone_mapping_method2_t method = opar->get_method();

    float exposure = opar->get_exposure();
    //float gamma = opar->get_exponent();
    float log_sh = opar->get_log_sh();
    float log_hl = opar->get_log_hl();
    float filmic2_gamma = opar->get_filmic2_exponent();
    float filmic2_igamma = 1.f/filmic2_gamma;
    float lumi_blend_frac = opar->get_lumi_blend_frac();
    ICCProfile* prof = opar->get_icc_data();

    float* pin;
    float* pout;
    float RGB[4];
    float RGBin[4];
    float Lab[3];
    int x, y, k;

    const float minus = -1.f;

    float A = opar->get_filmic_A();
    float B = opar->get_filmic_B();
    float C = opar->get_filmic_C();
    float D = opar->get_filmic_D();
    float E = opar->get_filmic_E();
    float F = opar->get_filmic_F();
    float W = opar->get_filmic_W();
    float whiteScale = 1.0f/( ((W*(A*W+C*B)+D*E)/(W*(A*W+B)+D*F))-E/F );
    float CB = C*B;
    float DE = D*E;
    float DF = D*F;
    float EoF = E/F;

    float AL_Lmax = pow(10.0, opar->get_AL_Lmax());
    float AL_b = opar->get_AL_b();
    float AL_scale = 1; //pow(AL_b-0.85+1, 5);
    float Ldmax = 100;
    //float b = AL_b;
    float Lwmax = AL_Lmax/AL_scale;
    float LLwmax = log10(Lwmax+1);
    float AL_A = 0.01 * Ldmax / LLwmax;
    float AL_E = log(AL_b)/log(0.5);
    float AL_Tsize = pow(opar->get_AL_Tsize(),2.4);
    float AL_Tslope = (Lwmax*AL_A)/(log(2)*2);
    float AL_Tlength = opar->get_AL_Tlength()*AL_Tsize/AL_Tslope + 1.0e-15;
    float AL_iTlength = 1/AL_Tlength;
    float AL_Texp = AL_Tslope / (AL_Tsize*AL_iTlength);

    float AL_Trange = opar->get_AL_Tstrength()*AL_Tsize;
    float AL_Tshift = pow( (1.f-opar->get_AL_Tstrength()), 1.f/AL_Texp ) * AL_Tlength;
    float AL_Tmax = AL_Tlength - AL_Tshift;
    float AL_Tvshift = (1.f-opar->get_AL_Tstrength()) * AL_Tsize;

    float LP_midgray = pow(0.5,2.45);
    float LE_midgray = LP_midgray;

    if(false) {
      float Lw = (AL_Tlength+1)*(1.0f-AL_Tlength)*Lwmax;
      float LLw1p = log1p(Lw);
      float Rw = Lw/Lwmax;
      float D = log( (pow(Rw,AL_E))*8 + 2 );
      float B = LLw1p / D;
      std::cout<<"  toe size: "<<AL_Tsize<<"  toe length: "<<AL_Tlength<<"  n: "<<AL_Texp<<"  shift: "<<AL_Tshift<<std::endl;
      //std::cout<<"Lw="<<Lw<<"  AL_scale="<<AL_scale<<"  AL_A="<<AL_A<<"  LLw1p="<<LLw1p
      //    <<"  Rw="<<Rw<<"  D="<<D<<"  A="<<AL_A<<"  B="<<B<<"  A*B="<<AL_A*B<<std::endl;
    }


    float HD_fog = 0.;
    float HD_max = 4;
    float HD_lin_slope = opar->get_HD_slope() / HD_max;
    float HD_lin_pivot = log10(1.0f/pow(0.5,2.45));
    float HD_lin_Dmin = opar->get_HD_shoulder_range() * HD_max;
    float HD_lin_Dmax = HD_max * (1.0f - opar->get_HD_toe_range());
    float HD_par[4] = { HD_lin_slope, HD_lin_pivot, HD_lin_Dmin, HD_lin_Dmax };
    //std::cout<<"HD_filmic2(0.1813)="<<HD_filmic2(0.1813, HD_par)<<std::endl;



    // Filmic #2 parameters
    FilmicToneCurve::CurveParamsUser filmic2_user;
    filmic2_user.m_toeStrength      = opar->get_filmic2_TS();
    filmic2_user.m_toeLength        = opar->get_filmic2_TL();
    filmic2_user.m_shoulderStrength = opar->get_filmic2_SS();
    filmic2_user.m_shoulderLength   = opar->get_filmic2_SL();
    if(filmic2_user.m_shoulderLength > 0.9999) filmic2_user.m_shoulderLength = 0.9999;
    filmic2_user.m_shoulderAngle    = opar->get_filmic2_SA();
    filmic2_user.m_gamma            = 1;
    FilmicToneCurve::CurveParamsDirect filmic2_direct;
    FilmicToneCurve::CalcDirectParamsFromUser(filmic2_direct, filmic2_user);
    FilmicToneCurve::FullCurve filmic2_curve;
    FilmicToneCurve::CreateCurve(filmic2_curve, filmic2_direct);

    float filmic2_scale = opar->get_filmic2_preserve_midgray() ? filmic2_curve.EvalInv(0.1814)/0.1814 : 1.f;

    ICCProfile* labprof = ICCStore::Instance().get_Lab_profile();
    ICCTransform rgb2lab, lab2rgb;
    rgb2lab.init(prof, labprof, VIPS_FORMAT_FLOAT);
    lab2rgb.init(labprof, prof, VIPS_FORMAT_FLOAT);

    float log_pivot = prof->perceptual2linear( opar->get_log_pivot() );
    float log_scale_sh = log(log_pivot*log_sh+1);
    float log_scale_hl = log(log_pivot*log_hl+1);

    //std::cout<<"gamma = "<<gamma<<std::endl;
    //std::cout<<"log(0.2*gamma+1) / gamma_scale = "<<log(0.2*gamma+1) / gamma_scale<<std::endl;
    //std::cout<<"log(gamma+1) / gamma_scale = "<<log(gamma+1) / gamma_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {
        RGBin[0] = pin[x];
        RGBin[1] = pin[x+1];
        RGBin[2] = pin[x+2];
        /*
        if( exposure != 0 ) {
          for( k=0; k < 3; k++) {
            RGBin[k] *= exposure;
            //clip( exposure*RGB[k], RGB[k] );
          }
        }
        */
        RGBin[3] = prof->get_lightness(RGBin[0],RGBin[1],RGBin[2]);
        /*
        std::cout<<"pin: "<<pin[x]<<" "<<pin[x+1]<<" "<<pin[x+2]<<" "<<std::endl;
        std::cout<<"Lab: "<<Lab[0]<<" "<<Lab[1]<<" "<<Lab[2]<<" "<<std::endl;
        std::cout<<"RGB: "<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<" "<<std::endl;
        std::cout<<std::endl;
        */
        for( k=0; k < 4; k++)
          RGB[k] = RGBin[k];

        /*if( gamma2 != 1 ) {
          for( k=0; k < 4; k++) {
            if(RGB[k] < 0) RGB[k] = powf( RGB[k]*minus/gamma_pivot, gamma2 )*minus*gamma_pivot;
            else RGB[k] = powf( RGB[k]/gamma_pivot, gamma2 ) * gamma_pivot;
            //clip( exposure*RGB[k], RGB[k] );
          }
        }*/

        if( log_sh > 0 || log_hl > 0 ) {
          for( k=0; k < 4; k++) {
            if( log_hl > 0 && RGB[k] > log_pivot ) {
              RGB[k] = log(RGB[k]*log_hl+1) * log_pivot / log_scale_hl;
            } else if( log_sh > 0 && RGB[k] <= log_pivot ) {
              if(RGB[k] < 0) RGB[k] = log(RGB[k]*minus*log_sh+1) * minus * log_pivot / log_scale_sh;
              else RGB[k] = log(RGB[k]*log_sh+1) * log_pivot / log_scale_sh;
            }
            //clip( exposure*RGB[k], RGB[k] );
          }
        }

        switch( method ) {
        case TONE_MAPPING_LIN_EXP: {
          float LE_slope = opar->get_LE_slope();
          float LE_linmax = pow(opar->get_LE_lin_max(),2.45);
          float LE_Ylinmax = ( LE_linmax - LE_midgray ) * LE_slope + LE_midgray;
          float LE_Srange = 1.0f - LE_Ylinmax;
          float LE_compr = opar->get_LE_compression();
          float LE_compr2 = LE_compr*2.5f;
          float LE_Sslope = opar->get_LE_shoulder_slope();
          float LE_Sslope2 = opar->get_LE_shoulder_slope2();
          float LE_Kstrength = opar->get_LE_knee_strength();
          float LE_Kmax = ((1.f-LE_slope)*LE_midgray)/(LE_slope/(sqrt(2.0f)*LE_Kstrength)-LE_slope);
          float LE_Kymax = (LE_Kmax-LE_midgray)*LE_slope + LE_midgray;
          float LE_Kexp = LE_Kmax * LE_slope / LE_Kymax;

          //RGB[3] = LE_midgray;

          for( k=0; k < 4; k++) {
            //std::cout<<"lin+exp: RGB["<<k<<"] in:  "<<RGB[k]<<std::endl;
            if( RGB[k] > LE_linmax ) {
              // shoulder
              float X = (RGB[k] - LE_linmax) * LE_slope * LE_compr / LE_Srange;
              float XD = pow(X,LE_Sslope2) * LE_Sslope /*LE_compr*/ + 1;
              //RGB[k] = 1.0f - LE_Srange * exp( -X / XD );
              RGB[k] = (LE_Srange - LE_Srange * exp( -X / XD )) / LE_compr + LE_Ylinmax;
            } else if( RGB[k] < LE_Kmax ) {
              // knee
              float X = RGB[k] / LE_Kmax;
              RGB[k] = (X>=0) ? LE_Kymax * pow(X,LE_Kexp) : -LE_Kymax * pow(-X,LE_Kexp);
            } else {
              // linear part
              RGB[k] = (RGB[k] - LE_midgray) * LE_slope + LE_midgray;
            }
            //std::cout<<"lin+log: RGB["<<k<<"] out: "<<RGB[k]<<std::endl;
          }
          //std::cout<<"LIN_POW: "<<LE_midgray<<" -> "<<RGB[3]<<std::endl;
          break;
        }
        default:
          break;
        }

        pout[x] = RGB[0];
        pout[x+1] = RGB[1];
        pout[x+2] = RGB[2];
        if( prof && opar->get_hue_protection() ) {
          float Jab_in[3], JCH_in[3];
          float Jab[3], JCH[3];
          float Jz, az, bz, C, H;
          // convert to Jzazbz in polar coordinates
          prof->to_Jzazbz(RGBin[0], RGBin[1], RGBin[2], Jab_in[0], Jab_in[1], Jab_in[2]);
          PF::Lab2LCH(Jab_in, JCH_in, 1);

          prof->to_Jzazbz(RGB[0], RGB[1], RGB[2], Jab[0], Jab[1], Jab[2]);
          PF::Lab2LCH(Jab, JCH, 1);

          JCH[2] = JCH_in[2];
          if( opar->get_gamut_compression() )
            prof->chroma_compression(JCH[0], JCH[1], JCH[2]);

          PF::LCH2Lab(JCH, Jab, 1);
          prof->from_Jzazbz( Jab[0], Jab[1], Jab[2], RGB[0], RGB[1], RGB[2] );

          pout[x] = RGB[0];
          pout[x+1] = RGB[1];
          pout[x+2] = RGB[2];
        }
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

    float exposure = opar->get_exposure();
    float gamma = 1;//opar->get_gamma();

    T* pin;
    T* pout;
    int x, y;

    float a, b;

    for( y = 0; y < height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      float exposure2 = exposure;
      float gamma2 = gamma;

      for( x = 0; x < line_size; x+=3 ) {

        pout[x] = pin[x];
        pout[x+1] = pin[x+1];
        pout[x+2] = pin[x+2];

        if( exposure2 != 0 ) {
          pout[x] *= exposure;
        }

        if( gamma2 != 1 ) {
          pout[x] = powf( pout[x], gamma );
        }
      }
    }
  }
};



ProcessorBase* new_tone_mapping_v2();
}

#endif 


