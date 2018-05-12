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

#ifndef BASIC_ADJUSTMENTS_H
#define BASIC_ADJUSTMENTS_H

#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../base/color.hh"
#include "../base/processor.hh"
#include "../base/splinecurve.hh"

//#define CLIPRAW(a) ((a)>0.0?((a)<1.0?(a):1.0):0.0)
#define CLIPRAW(a) (a)

namespace PF 
{

  class BasicAdjustmentsPar: public OpParBase
  {
    Property<float> brightness;
    Property<float> exposure;
    Property<float> gamma;
    Property<float> white_level;
    Property<float> black_level;

    float exposure_pow;
    float exponent;

    Property<float> hue, hue_eq;
    Property<float> saturation, saturation_eq;
    Property<float> contrast, contrast_eq;
    Property<SplineCurve> hue_H_equalizer;
    Property<SplineCurve> hue_S_equalizer;
    Property<SplineCurve> hue_L_equalizer;
    Property<bool> hue_H_equalizer_enabled;
    Property<bool> hue_S_equalizer_enabled;
    Property<bool> hue_L_equalizer_enabled;
    Property<SplineCurve> saturation_H_equalizer;
    Property<SplineCurve> saturation_S_equalizer;
    Property<SplineCurve> saturation_L_equalizer;
    Property<SplineCurve> contrast_H_equalizer;
    Property<SplineCurve> contrast_S_equalizer;
    Property<SplineCurve> contrast_L_equalizer;

    Property<bool> show_mask;
    Property<bool> invert_mask;
    Property<bool> feather_mask;
    Property<float> feather_radius;

    Property<SplineCurve>* eq_vec[12];

    ProcessorBase* mask;
    ProcessorBase* blur;

    ICCProfile* icc_data;

    cmsHPROFILE lab_profile;
    cmsHTRANSFORM transform, transform_inv;

    void update_curve( Property<SplineCurve>* grey_curve, float* vec );

  public:

    float vec[12][65535];
    bool eq_enabled[12];

    BasicAdjustmentsPar();

    cmsHTRANSFORM get_transform() { return transform; }
    cmsHTRANSFORM get_transform_inv() { return transform_inv; }

    ICCProfile* get_icc_data() { return icc_data; }

    float get_brightness() { return brightness.get(); }
    float get_exposure() { return exposure.get(); /*exposure_pow*/; }
    float get_gamma() { return exponent; }
    float get_white_level() { return white_level.get(); }
    float get_black_level() { return black_level.get(); }

    float get_hue() { return hue.get(); }
    float get_hue_eq() { return hue_eq.get(); }
    float get_saturation() { return saturation.get(); }
    float get_saturation_eq() { return saturation_eq.get(); }
    float get_contrast() { return contrast.get(); }
    float get_contrast_eq() { return contrast_eq.get(); }

    bool get_show_mask() { return show_mask.get(); }
    bool get_invert_mask() { return invert_mask.get(); }

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_input() { return true; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class BasicAdjustments
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class BasicAdjustments< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      BasicAdjustmentsPar* opar = dynamic_cast<BasicAdjustmentsPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      float gamma = opar->get_gamma();
      float* pin;
      float* pmask;
      float* pout;
      float* p;
      float* q;
      float RGB[3];
      int x, y, k, i;

      const float minus = -1.f;

      /* Do the actual processing
       */


      float brightness = opar->get_brightness();
      float exposure = opar->get_exposure();
      //float gamma = opar->get_gamma();
      float black_level = opar->get_black_level();
      float white_level = opar->get_white_level();


      //if( exposure != 1 ) std::cout<<"EXPOSURE: "<<exposure<<std::endl;

      float hue = opar->get_hue();
      float saturation = opar->get_saturation();
      float contrast = opar->get_contrast();
      bool inv = opar->get_invert_mask();

      typename FormatInfo<float>::SIGNED tempval;
      float h_in, s_in, v_in, l_in;
      float h, s, v, l;

      float midpoint = 0.5;
      if( opar->get_icc_data() && (opar->get_icc_data()->is_linear()) )
        midpoint = 0.18;

      if( PREVIEW && opar->get_show_mask() ) {
        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pmask = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

          float fmask;
          for( x = 0; x < line_size; x+=3 ) {
            to_float( pmask[x], fmask );
            pout[x] = fmask*pin[x] + (1.0f-fmask)*FormatInfo<float>::MAX;
            pout[x+1] = fmask*pin[x+1] + (1.0f-fmask)*FormatInfo<float>::MIN;
            pout[x+2] = fmask*pin[x+2] + (1.0f-fmask)*FormatInfo<float>::MIN;
          }
        }
      } else {
        //for(int ti=0;ti<100;ti++) {
          //if((ti%10)==0) std::cout<<"basic_adjustments: ti="<<ti<<" corner="<<r->left<<","<<r->top<<std::endl;
        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pmask = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

          for( x = 0; x < line_size; x+=3 ) {
            RGB[0] = pin[x];
            RGB[1] = pin[x+1];
            RGB[2] = pin[x+2];

            float h_eq = 1;
            to_float( pmask[x], h_eq );

            float black_level2 = black_level;
            float white_level2 = white_level;
            float brightness2 = brightness;
            float exposure2 = exposure;
            float gamma2 = gamma;
            /*
            if( h_eq < 1 ) {
              black_level2 *= h_eq;
              white_level2 *= h_eq;
              brightness2 *= h_eq;
              exposure2 *= h_eq;
              float dgamma = (gamma2-1) * h_eq;
              gamma2 = dgamma + 1.f;
            }
            */
            if( (black_level2 != 0) || (white_level2 != 0) ) {
              float delta = (white_level2 + 1.f - black_level2);
              if( fabs(delta) < 0.0001f ) {
                if( delta > 0 ) delta = 0.0001f;
                else delta = -0.0001f;
              }
              for( k=0; k < 3; k++) {
                RGB[k] = (RGB[k] - black_level2) / delta;
                //clip( exposure*RGB[k], RGB[k] );
              }
            }

            if( brightness2 != 0 ) {
              for( k=0; k < 3; k++) {
                RGB[k] += brightness2*FormatInfo<float>::RANGE;
                //clip( (contrast2+1.0f)*tempval+brightness2*FormatInfo<float>::RANGE+FormatInfo<float>::HALF, RGB[k] );
              }
            }

            if( exposure2 != 1 ) {
              for( k=0; k < 3; k++) {
                RGB[k] *= exposure2;
                //clip( exposure*RGB[k], RGB[k] );
              }
            }

            if( gamma2 != 1 ) {
              for( k=0; k < 3; k++) {
                //RGB[k] = pin[x+k]; //powf( RGB[k], gamma );
                if(RGB[k] < 0) RGB[k] = powf( RGB[k]*minus, gamma2 )*minus;
                else RGB[k] = powf( RGB[k], gamma2 );
                //clip( exposure*RGB[k], RGB[k] );
              }
            }

            //std::cout<<"h_in="<<h_in<<"  h_eq="<<h_eq<<" ("<<h_eq1<<" "<<h_eq2<<" "<<h_eq3<<")"<<std::endl;

            float hue2 = hue;
            float saturation2 = saturation;
            float contrast2 = contrast;

            //if( back_level2 >= 1.f ) black_level2 = 0.9999f;
            //if( white_level2 <= -1.f ) white_level2 = -0.9999f;
            /*
            if( h_eq < 1 ) {
              hue2 *= h_eq;
              saturation2 *= h_eq;
              contrast2 *= h_eq;
            }
            */
            /*
          if( opar->get_brightness_eq() != 0 ) {
            //float c_eq1 = opar->vec[6][hid];
            //float c_eq2 = opar->vec[7][sid];
            //float c_eq3 = opar->vec[8][lid];
            //float c_eq = MIN3( c_eq1, c_eq2, c_eq3 );
            float b_eq = h_eq;
            brightness2 += opar->get_brightness_eq()*b_eq;
          }
          if( opar->get_contrast_eq() != 0 ) {
            //float c_eq1 = opar->vec[6][hid];
            //float c_eq2 = opar->vec[7][sid];
            //float c_eq3 = opar->vec[8][lid];
            //float c_eq = MIN3( c_eq1, c_eq2, c_eq3 );
            float c_eq = h_eq;
            contrast2 += opar->get_contrast_eq()*c_eq;
          }
             */

            if( contrast2 != 0 ) {
              for( k=0; k < 3; k++) {
                tempval = (typename FormatInfo<float>::SIGNED)RGB[k] - midpoint;
                RGB[k] = (contrast2+1.0f)*tempval + midpoint;
                //clip( (contrast2+1.0f)*tempval+brightness2*FormatInfo<float>::RANGE+FormatInfo<float>::HALF, RGB[k] );
              }
            }

            if( hue2 != 0 || saturation2 != 0 ) {
              rgb2hsv( RGB[0], RGB[1], RGB[2], h_in, s_in, l_in );
              //rgb2hsl( RGB[0], RGB[1], RGB[2], h_in, s_in, l_in );
              //std::cout<<"in RGB: "<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<"  HSL: "<<h_in<<" "<<s_in<<" "<<l_in<<std::endl;
            }
            /*
            unsigned short int hid = static_cast<unsigned short int>( h_in*65535/360 );
            unsigned short int sid = static_cast<unsigned short int>( s_in*65535 );
            unsigned short int lid = static_cast<unsigned short int>( l_in*65535 );
            float h_eq1 = opar->eq_enabled[0] ? opar->vec[0][hid] : 1;
            float h_eq2 = opar->eq_enabled[1] ? opar->vec[1][sid] : 1;
            float h_eq3 = opar->eq_enabled[2] ? opar->vec[2][lid] : 1;
            float h_eq = MIN3( h_eq1, h_eq2, h_eq3 );
             */

            /*
          //h = h_in + hue + opar->get_hue_eq()*h_eq;
          if( opar->get_hue_eq() ) {
            hue2 += opar->get_hue_eq()*h_eq;
          }
             */
            if( hue2 != 0 ) {
              h = h_in + hue2;
              if( h > 360 ) h -= 360;
              else if( h < 0 ) h+= 360;
            } else {
              h = h_in;
            }

            /*
          if( opar->get_saturation_eq() ) {
            //float s_eq1 = opar->vec[3][hid];
            //float s_eq2 = opar->vec[4][sid];
            //float s_eq3 = opar->vec[5][lid];
            //float s_eq = MIN3( s_eq1, s_eq2, s_eq3 );
            float s_eq = h_eq;
            saturation2 += opar->get_saturation_eq()*s_eq;
          }
             */
            if( saturation2 != 0 ) {
              s = s_in*(1.0f+saturation2);
              if( s < 0 ) s = 0;
              else if( s > 1 ) s = 1;
            } else {
              s = s_in;
            }

            l = l_in;

            //h = h_in;
            //s = s_in;

            //hsv2rgb2( h, s, v, R, G, B );
            if( (h != h_in) || (s != s_in) ) {
              hsv2rgb( h, s, l, RGB[0], RGB[1], RGB[2] );
              //hsl2rgb( h, s, l, RGB[0], RGB[1], RGB[2] );
              //std::cout<<"out RGB: "<<R<<" "<<G<<" "<<B<<"  HSV: "<<h<<" "<<s<<" "<<v<<std::endl;
            }

            pout[x] = h_eq*RGB[0] + (1.0f-h_eq)*pin[x];
            pout[x+1] = h_eq*RGB[1] + (1.0f-h_eq)*pin[x+1];
            pout[x+2] = h_eq*RGB[2] + (1.0f-h_eq)*pin[x+2];
          }
        }
      }
      //}
    }
  };




  template < OP_TEMPLATE_DEF_CS_SPEC >
  class BasicAdjustments< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      BasicAdjustmentsPar* opar = dynamic_cast<BasicAdjustmentsPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      float brightness = opar->get_brightness();
      float exposure = opar->get_exposure();
      float gamma = opar->get_gamma();
      float black_level = opar->get_black_level();
      float white_level = opar->get_white_level();

      float hue = opar->get_hue();
      float saturation = opar->get_saturation();
      float contrast = opar->get_contrast();
      bool inv = opar->get_invert_mask();

      T* pin;
      T* pout;
      typename PF::FormatInfo<T>::SIGNED L, a, b;
      int x, y;

      float sat = 1.0f;
      if( saturation > 0 ) sat += opar->get_saturation();
      else {
        sat -= saturation;
        sat = 1.0f / sat;
      }

      for( y = 0; y < height; y++ ) {
        pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x+=3 ) {

          L = pin[x];
          a = pin[x+1];
          b = pin[x+2];
          if( exposure != 1.0f ) {
            L *= exposure;
          }

          if( sat != 1.0f ) {

          a -= PF::FormatInfo<T>::HALF;
          b -= PF::FormatInfo<T>::HALF;

          a *= sat;
          b *= sat;

          a += PF::FormatInfo<T>::HALF;
          b += PF::FormatInfo<T>::HALF;
          }

          pout[x] = L;
          clip( a, pout[x+1] );
          clip( b, pout[x+2] );
        }
      }
    }
  };



  ProcessorBase* new_basic_adjustments();
}

#endif 


