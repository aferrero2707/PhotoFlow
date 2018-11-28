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

#pragma once

#include <string>

#include "../base/color.hh"
#include "../base/processor.hh"

namespace PF 
{

  class NoiseGeneratorPar: public OpParBase
  {
    Property<bool> monochrome;
    Property<bool> perceptual;
    Property<float> center;
    Property<float> range;

    ICCProfile* icc_data;

  public:

    NoiseGeneratorPar(): OpParBase(),
    monochrome("monochrome",this,true),
    perceptual("perceptual",this,true),
    center("center",this,0.5),
    range("range",this,0.5),
    icc_data(NULL)
    {
      set_type("noise_generator" );
      set_default_name( _("noise") );
    }
    //~NoiseGeneratorPar();

    ICCProfile* get_icc_data() { return icc_data; }

    bool needs_caching() { return true; }

    bool get_monochrome() { return (monochrome.get()); }
    bool get_perceptual() { return (perceptual.get()); }
    float get_range() { return (range.get()); }
    float get_center() { return (center.get()); }

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_input() { return true; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level)
    {
      icc_data = NULL;
      if(in.size()>0 && in[0]) icc_data = PF::get_icc_profile( in[0] );

      VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );
      //std::cout<<"NoiseGeneratorPar::build: out="<<out<<std::endl;
      return out;
    }
 };

  

  template < OP_TEMPLATE_DEF > 
  class NoiseGeneratorProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class NoiseGeneratorProc< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      NoiseGeneratorPar* opar = dynamic_cast<NoiseGeneratorPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
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

      bool monochrome = opar->get_monochrome();
      bool perceptual = opar->get_perceptual();
      float center = opar->get_center();
      float range = opar->get_range();

      ICCProfile* profile = opar->get_icc_data();

      //for(int ti=0;ti<100;ti++) {
      //if((ti%10)==0) std::cout<<"basic_adjustments: ti="<<ti<<" corner="<<r->left<<","<<r->top<<std::endl;
      for( y = 0; y < height; y++ ) {
        pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x+=3 ) {

          float R1 = rand();
          R1 /= RAND_MAX;
          R1 -= 0.5;
          R1 *= range;
          float R2 = R1;
          float R3 = R1;
          if( !monochrome ) {
            R2 = rand();
            R2 /= RAND_MAX;
            R2 -= 0.5;
            R2 *= range;
            R3 = rand();
            R3 /= RAND_MAX;
            R3 -= 0.5;
            R3 *= range;
          }

          /*
          if( true && perceptual && profile && profile->is_linear() ) {
            RGB[0] = profile->linear2perceptual(pin[x]);
            RGB[1] = profile->linear2perceptual(pin[x+1]);
            RGB[2] = profile->linear2perceptual(pin[x+2]);
          } else {
            RGB[0] = pin[x];
            RGB[1] = pin[x+1];
            RGB[2] = pin[x+2];
          }
          */

          RGB[0] = center + R1;
          RGB[1] = center + R2;
          RGB[2] = center + R3;

          if( true && perceptual && profile && profile->is_linear() ) {
            pout[x]   = profile->perceptual2linear(RGB[0]);
            pout[x+1] = profile->perceptual2linear(RGB[1]);
            pout[x+2] = profile->perceptual2linear(RGB[2]);
          } else {
            pout[x] = RGB[0];
            pout[x+1] = RGB[1];
            pout[x+2] = RGB[2];
          }
        }
      }
    }
  };




  ProcessorBase* new_noise_generator();
}

