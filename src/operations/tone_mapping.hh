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

#ifndef PF_TONE_MAPPING_H
#define PF_TONE_MAPPING_H

#include <string>

//#include <glibmm.h>

#include "../base/processor.hh"
#include "Filmic/FilmicCurve/FilmicToneCurve.h"


namespace PF 
{


#define GamutMapNYPoints 1000


enum tone_mapping_method_t
{
  TONE_MAPPING_LIN_POW,
  TONE_MAPPING_HD,
  TONE_MAPPING_FILMIC2,
  TONE_MAPPING_FILMIC,
  TONE_MAPPING_EXP_GAMMA,
  TONE_MAPPING_REINHARD,
  TONE_MAPPING_HEJL,
  TONE_MAPPING_ADAPTIVE_LOG
};


float HD_filmic(float x, float *par);
float HD_filmic2(float x, float *par);




class ToneMappingPar: public OpParBase
{
  PropertyBase method;
  Property<float> exposure;
  Property<float> gamma;
  Property<float> gamma_pivot;

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

  Property<float> HD_slope, HD_toe_range, HD_shoulder_range;

  Property<float> gamut_compression, gamut_compression_exponent, lumi_blend_frac;

  ICCProfile* icc_data;

  float exponent, filmic2_exponent;

public:

  float gamut_boundary[GamutMapNYPoints+1][360];

  ToneMappingPar();

  tone_mapping_method_t get_method() { return (tone_mapping_method_t)method.get_enum_value().first; }
  float get_exposure() { return exposure.get(); }
  float get_gamma() { return gamma.get(); }
  float get_gamma_pivot() { return gamma_pivot.get(); }
  float get_exponent() { return exponent; }

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

  float get_HD_slope() { return HD_slope.get(); }
  float get_HD_toe_range() { return HD_toe_range.get(); }
  float get_HD_shoulder_range() { return HD_shoulder_range.get();}

  float get_gamut_compression() { return gamut_compression.get(); }
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
class ToneMapping
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};




template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class ToneMapping< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingPar* opar = dynamic_cast<ToneMappingPar*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    tone_mapping_method_t method = opar->get_method();

    float exposure = opar->get_exposure();
    float gamma = opar->get_exponent();
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

    float gamma_pivot = prof->perceptual2linear( opar->get_gamma_pivot() );

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {
        RGBin[0] = pin[x];
        RGBin[1] = pin[x+1];
        RGBin[2] = pin[x+2];

        float exposure2 = exposure;
        float gamma2 = gamma;
        if( exposure2 != 0 ) {
          for( k=0; k < 4; k++) {
            RGBin[k] *= exposure2;
            //clip( exposure*RGB[k], RGB[k] );
          }
        }
        RGBin[3] = prof->get_lightness(RGBin[0],RGBin[1],RGBin[2]);
        /*
        std::cout<<"pin: "<<pin[x]<<" "<<pin[x+1]<<" "<<pin[x+2]<<" "<<std::endl;
        std::cout<<"Lab: "<<Lab[0]<<" "<<Lab[1]<<" "<<Lab[2]<<" "<<std::endl;
        std::cout<<"RGB: "<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<" "<<std::endl;
        std::cout<<std::endl;
        */
        for( k=0; k < 4; k++)
          RGB[k] = RGBin[k];

        switch( method ) {
        case TONE_MAPPING_EXP_GAMMA:
          if( gamma2 != 1 ) {
            for( k=0; k < 4; k++) {
              if(RGB[k] < 0) RGB[k] = powf( RGB[k]*minus/gamma_pivot, gamma2 )*minus*gamma_pivot;
              else RGB[k] = powf( RGB[k]/gamma_pivot, gamma2 ) * gamma_pivot;
              //clip( exposure*RGB[k], RGB[k] );
            }
          }
          break;
        case TONE_MAPPING_REINHARD: {
          for( k=0; k < 4; k++) {
            RGB[k] = RGB[k] / (RGB[k] + 1.f);
          }
          break;
        }
        case TONE_MAPPING_HEJL: {
          for( k=0; k < 4; k++) {
            float x = MAX(0,RGB[k]-0.004);
            RGB[k] = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
            RGB[k] = powf( RGB[k], 2.2f );
          }
          break;
        }
        case TONE_MAPPING_FILMIC: {
          for( k=0; k < 4; k++) {
            //std::cout<<"TONE_MAPPING_FILMIC: in RGB["<<k<<"]="<<RGB[k];
            RGB[k] *= 2;
            RGB[k] = ((RGB[k]*(A*RGB[k]+CB)+DE)/(RGB[k]*(A*RGB[k]+B)+DF))-EoF;
            RGB[k] *= whiteScale;
            //std::cout<<"    out RGB["<<k<<"]="<<RGB[k]<<std::endl;
          }
          break;
        }
        case TONE_MAPPING_FILMIC2: {
          for( k=0; k < 4; k++) {
            if(filmic2_gamma != 1) {
              if(RGB[k] < 0) RGB[k] = powf( RGB[k]*minus, filmic2_gamma )*minus;
              else RGB[k] = powf( RGB[k], filmic2_gamma );
            }
            if(RGB[k] < 0) RGB[k] = minus*filmic2_curve.Eval(minus*RGB[k]*filmic2_scale);
            else RGB[k] = filmic2_curve.Eval(RGB[k]*filmic2_scale);
            if(filmic2_gamma != 1) {
              if(RGB[k] < 0) RGB[k] = powf( RGB[k]*minus, filmic2_igamma )*minus;
              else RGB[k] = powf( RGB[k], filmic2_igamma );
            }
          }
          break;
        }
        case TONE_MAPPING_ADAPTIVE_LOG: {
          for( k=0; k < 4; k++) {
            if( RGB[k] > AL_Tmax ) {
              float Lw = (AL_Tlength+1-AL_Tshift)*(RGB[k]-AL_Tlength+AL_Tshift)*Lwmax;
              float LLw1p = log1p(Lw);
              float Rw = Lw/Lwmax;
              float D = log( (pow(Rw,AL_E))*8 + 2 );
              float B = LLw1p / D;
              RGB[k] = AL_Trange + (1.0f-AL_Trange)*AL_A*B;//A * B;
            } else {
              float Lw = (RGB[k]+AL_Tshift)*AL_iTlength;
              RGB[k] = AL_Tsize * pow(Lw,AL_Texp) - AL_Tvshift;
            }
          }
          break;
        }
        case TONE_MAPPING_LIN_POW: {
          float LP_slope = opar->get_LP_slope();
          float LP_linmax = pow(opar->get_LP_lin_max(),2.45);
          float LP_Ylinmax = ( LP_linmax - LP_midgray ) * LP_slope + LP_midgray;
          float LP_Srange = 1.0f - LP_Ylinmax;
          float LP_Sslope = LP_Srange * LP_slope;
          float LP_compr = opar->get_LP_compression();
          float LP_compr2 = LP_compr*2.5f;
          float LP_Ssmooth = opar->get_LP_shoulder_smoothness();
          float LP_Kstrength = opar->get_LP_knee_strength();
          float LP_Kmax = ((1.f-LP_slope)*LP_midgray)/(LP_slope/(sqrt(2.0f)*LP_Kstrength)-LP_slope);
          float LP_Kymax = (LP_Kmax-LP_midgray)*LP_slope + LP_midgray;
          float LP_Kexp = LP_Kmax * LP_slope / LP_Kymax;

          //RGB[3] = LP_midgray;

          for( k=0; k < 4; k++) {
            //std::cout<<"lin+log: RGB["<<k<<"] in:  "<<RGB[k]<<std::endl;
            if( RGB[k] > LP_linmax ) {
              // shoulder
              //float Lw = (LP_linmax - RGB[k]) / LP_Srange;
              //RGB[k] = LP_Srange * (1.0f - exp(Lw*LP_slope*LP_compr)) / LP_compr + LP_Ylinmax;
              float X = (RGB[k] - LP_linmax) * LP_slope;
              float Y1 = log(X*LP_compr2+1) / LP_compr2 + LP_Ylinmax;
              float Y2 = ( 1.0f - log(X*LP_compr+1) / (X*LP_compr) ) * 2.0f / LP_compr + LP_Ylinmax;
              RGB[k] = LP_Ssmooth * Y1 + (1.0f - LP_Ssmooth) * Y2;
            } else if( RGB[k] < LP_Kmax ) {
              // knee
              //float X = RGB[k] / LP_Kmax - 1.0f;
              //RGB[k] = LP_slope*LP_Kmax*( (sqrt(2.0f)*X/sqrt(1.0f+X*X) + 1.0f) / sqrt(2.0f) );
              float X = RGB[k] / LP_Kmax;
              RGB[k] = (X>=0) ? LP_Kymax * pow(X,LP_Kexp) : -LP_Kymax * pow(-X,LP_Kexp);
            } else {
              // linear part
              RGB[k] = (RGB[k] - LP_midgray) * LP_slope + LP_midgray;
            }
            //std::cout<<"lin+log: RGB["<<k<<"] out: "<<RGB[k]<<std::endl;
          }
          //std::cout<<"LIN_POW: "<<LP_midgray<<" -> "<<RGB[3]<<std::endl;
          break;
        }
        case TONE_MAPPING_HD: {
          for( k=0; k < 4; k++) {
            RGB[k] = HD_filmic2(RGB[k], HD_par);
          }
          break;
        }
        default:
          break;
        }

        pout[x] = RGB[0];
        pout[x+1] = RGB[1];
        pout[x+2] = RGB[2];

        if( lumi_blend_frac > 0 && prof ) {
          float Lout = RGB[3]; //prof->get_lightness(pout[x],pout[x+1],pout[x+2]);
          if( false ) {
          rgb2lab.apply(RGBin, Lab);
          /*if( Lout > 1 ) {
            Lab[1] = Lab[2] = 0;
          } else*/ {
            float LCH[3];
            PF::Lab2LCH(Lab, LCH, 1);
            int Yid = (Lout>1) ? GamutMapNYPoints : static_cast<int>(Lout*GamutMapNYPoints);
            int Hid = static_cast<int>(LCH[2]*180/M_PI);
            //float Cmax = opar->gamut_boundary[Yid][Hid];
            //if( LCH[1] > Cmax ) LCH[1] -= opar->get_gamut_compression()*(LCH[1] - Cmax);
            //std::cout<<"LCH="<<LCH[0]<<" "<<LCH[1]<<" "<<LCH[2]<<"  Lout="<<Lout<<"  Yid="<<Yid<<"  Hid="<<Hid
            //    <<"  opar->gamut_boundary[Yid][Hid]="<<opar->gamut_boundary[Yid][Hid]<<std::endl;
            if( LCH[1] > opar->gamut_boundary[Yid][Hid] ) {
              float Cmax = opar->gamut_boundary[Yid][Hid];
              LCH[1] -= opar->get_gamut_compression()*(LCH[1] - Cmax);
              //std::cout<<"  LCH[1]="<<LCH[1]<<"  Cmax="<<Cmax<<std::endl;
              /*
              if( LCH[1] > Cmax ) {
                Cmax = 0;
                for(int l = Yid; l >=0; l--) {
                  if( opar->gamut_boundary[l][Hid] < Cmax ) break;
                  Cmax = opar->gamut_boundary[l][Hid];
                  Lout = float(l)/100.0f;
                  std::cout<<"  l="<<l<<"  Lout="<<Lout<<"  L="<<prof->linear2perceptual(Lout)*100<<"  Cmax="<<Cmax<<std::endl;
                  if( LCH[1] <= Cmax ) break;
                }
                if( LCH[1] > Cmax ) {
                  LCH[1] = Cmax;
                }
              }
              */
              PF::LCH2Lab(LCH, Lab, 1);
              //std::cout<<"  LCH="<<LCH[0]<<" "<<LCH[1]<<" "<<LCH[2]<<std::endl;
            }
            //Lab[1] *= 1.0f - opar->get_gamut_compression()*pow( (Lout-0.2f)/0.8f, opar->get_gamut_compression_exponent());
            //Lab[2] *= 1.0f - opar->get_gamut_compression()*pow( (Lout-0.2f)/0.8f, opar->get_gamut_compression_exponent());
          }
          lab2rgb.apply(Lab, RGBin);
          //std::cout<<"RGBin="<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<std::endl;
          }

          float Lin = prof->get_lightness(RGBin[0],RGBin[1],RGBin[2]);
          float R = Lout / MAX(Lin, 0.0000000000000000001);
          //std::cout<<"R (before scaling)="<<R<<std::endl;
          //if( R*RGBin[0] > 1 ) R = 1.f/RGBin[0];
          //if( R*RGBin[1] > 1 ) R = 1.f/RGBin[1];
          //if( R*RGBin[2] > 1 ) R = 1.f/RGBin[2];
          //std::cout<<"R (after scaling)="<<R<<std::endl;
          float RGBout[3] = { R*RGBin[0], R*RGBin[1], R*RGBin[2] };
          //std::cout<<"RGBout="<<RGBout[0]<<" "<<RGBout[1]<<" "<<RGBout[2]<<std::endl;

          pout[x]   = (1.f-lumi_blend_frac)*pout[x]   + lumi_blend_frac*RGBout[0];
          pout[x+1] = (1.f-lumi_blend_frac)*pout[x+1] + lumi_blend_frac*RGBout[1];
          pout[x+2] = (1.f-lumi_blend_frac)*pout[x+2] + lumi_blend_frac*RGBout[2];
        }
      }
    }
  }
};




template < OP_TEMPLATE_DEF_CS_SPEC >
class ToneMapping< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingPar* opar = dynamic_cast<ToneMappingPar*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    float exposure = opar->get_exposure();
    float gamma = opar->get_gamma();

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



ProcessorBase* new_tone_mapping();
}

#endif 


