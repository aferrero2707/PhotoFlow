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

#ifndef PF_LEVELS_H
#define PF_LEVELS_H

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

class LevelsPar: public OpParBase
{
  Property<float> brightness;
  Property<float> exposure;
  Property<float> gamma;
  Property<float> white_level;
  Property<float> black_level;

  float exponent;

public:

  LevelsPar();

  float get_brightness() { return brightness.get(); }
  float get_exposure() { return exposure.get(); }
  float get_gamma() { return exponent; }
  float get_white_level() { return white_level.get(); }
  float get_black_level() { return black_level.get(); }

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_input() { return true; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
                   VipsImage* imap, VipsImage* omap, unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class Levels
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};




template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class Levels< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    LevelsPar* opar = dynamic_cast<LevelsPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    float brightness = opar->get_brightness();
    float exposure = opar->get_exposure();
    float gamma = opar->get_gamma();
    float black_level = opar->get_black_level();
    float white_level = opar->get_white_level();

    float* pin;
    float* pout;
    float RGB[3];
    int x, y, k;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {
        RGB[0] = pin[x];
        RGB[1] = pin[x+1];
        RGB[2] = pin[x+2];

        float black_level2 = black_level;
        float white_level2 = white_level;
        float brightness2 = brightness;
        float exposure2 = exposure;
        float gamma2 = gamma;


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

        if( exposure2 != 0 ) {
          for( k=0; k < 3; k++) {
            RGB[k] *= exposure;
            //clip( exposure*RGB[k], RGB[k] );
          }
        }

        if( gamma2 != 1 ) {
          for( k=0; k < 3; k++) {
            RGB[k] = powf( RGB[k], gamma );
            //clip( exposure*RGB[k], RGB[k] );
          }
        }

        pout[x] = RGB[0];
        pout[x+1] = RGB[1];
        pout[x+2] = RGB[2];
      }
    }
  }
};




template < OP_TEMPLATE_DEF_CS_SPEC >
class Levels< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    LevelsPar* opar = dynamic_cast<LevelsPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
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
        a = pin[x+1];
        b = pin[x+2];

        pout[x] = pin[x];
        clip( a, pout[x+1] );
        clip( b, pout[x+2] );
      }
    }
  }
};



ProcessorBase* new_levels();
}

#endif 


