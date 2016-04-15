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


namespace PF 
{


enum tone_mapping_method_t
{
  TONE_MAPPING_EXP_GAMMA,
  TONE_MAPPING_REINHARD,
  TONE_MAPPING_FILMIC
};


class ToneMappingPar: public OpParBase
{
  PropertyBase method;
  Property<float> exposure;
  Property<float> gamma;

  Property<float> filmic_A;
  Property<float> filmic_B;
  Property<float> filmic_C;
  Property<float> filmic_D;
  Property<float> filmic_E;
  Property<float> filmic_F;
  Property<float> filmic_W;

  Property<float> lumi_blend_frac;

  ICCProfile* icc_data;

  float exponent;

public:

  ToneMappingPar();

  tone_mapping_method_t get_method() { return (tone_mapping_method_t)method.get_enum_value().first; }
  float get_exposure() { return exposure.get(); }
  float get_gamma() { return exponent; }

  float get_filmic_A() { return filmic_A.get(); }
  float get_filmic_B() { return filmic_B.get(); }
  float get_filmic_C() { return filmic_C.get(); }
  float get_filmic_D() { return filmic_D.get(); }
  float get_filmic_E() { return filmic_E.get(); }
  float get_filmic_F() { return filmic_F.get(); }
  float get_filmic_W() { return filmic_W.get(); }

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
    float gamma = opar->get_gamma();
    float lumi_blend_frac = opar->get_lumi_blend_frac();
    ICCProfile* prof = opar->get_icc_data();

    float* pin;
    float* pout;
    float RGB[3];
    int x, y, k;

    float A = opar->get_filmic_A();
    float B = opar->get_filmic_B();
    float C = opar->get_filmic_C();
    float D = opar->get_filmic_D();
    float E = opar->get_filmic_E();
    float F = opar->get_filmic_F();
    float W = opar->get_filmic_W();

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {
        RGB[0] = pin[x];
        RGB[1] = pin[x+1];
        RGB[2] = pin[x+2];

        float exposure2 = exposure;
        float gamma2 = gamma;


        if( exposure2 != 0 ) {
          for( k=0; k < 3; k++) {
            RGB[k] *= exposure;
            //clip( exposure*RGB[k], RGB[k] );
          }
        }

        switch( method ) {
        case TONE_MAPPING_EXP_GAMMA:
          if( gamma2 != 1 ) {
            for( k=0; k < 3; k++) {
              RGB[k] = powf( RGB[k], gamma );
              //clip( exposure*RGB[k], RGB[k] );
            }
          }
          break;
        case TONE_MAPPING_REINHARD: {
          for( k=0; k < 3; k++) {
            RGB[k] = RGB[k] / (RGB[k] + 1.f);
          }
          break;
        }
        case TONE_MAPPING_FILMIC: {
          float whiteScale = 1.0f/( ((W*(A*W+C*B)+D*E)/(W*(A*W+B)+D*F))-E/F );
          for( k=0; k < 3; k++) {
            RGB[k] = ((RGB[k]*(A*RGB[k]+C*B)+D*E)/(RGB[k]*(A*RGB[k]+B)+D*F))-E/F;
            RGB[k] *= whiteScale;
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
          float Lin = prof->get_lightness(pin[x],pin[x+1],pin[x+2]);
          float Lout = prof->get_lightness(pout[x],pout[x+1],pout[x+2]);
          float R = Lout / MAX(Lin, 0.0000000000000000001);
          pout[x]   = (1.f-lumi_blend_frac)*pout[x]   + lumi_blend_frac*R*pin[x];
          pout[x+1] = (1.f-lumi_blend_frac)*pout[x+1] + lumi_blend_frac*R*pin[x+1];
          pout[x+2] = (1.f-lumi_blend_frac)*pout[x+2] + lumi_blend_frac*R*pin[x+2];
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


