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

#ifndef PF_ENHANCED_USM_H
#define PF_ENHANCED_USM_H

#include "../rt/rtengine/sleef.c"
#include "../base/processor.hh"

namespace PF 
{

  class EnhancedUnsharpMaskPar: public OpParBase
  {
    Property<bool> do_sum;
    Property<float> amount;
    Property<float> radius, threshold_l, threshold_h;

    ProcessorBase* loglumi;
    ProcessorBase* guided[2];

    PF::ICCProfile* in_profile;
  public:
    EnhancedUnsharpMaskPar();

    bool has_intensity() { return false; }
    bool needs_caching() {
      return true;
    }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    PF::ICCProfile* get_profile() { return in_profile; }
    void set_amount( float a ) { amount.set( a ); }
    float get_amount() { return amount.get(); }
    void set_radius( float r ) { radius.set( r ); }
    void set_threshold_l( float t ) { threshold_l.set( t ); }
    float get_threshold_l() { return threshold_l.get(); }
    void set_threshold_h( float t ) { threshold_h.set( t ); }
    float get_threshold_h() { return threshold_h.get(); }

    void set_do_sum(bool b) { do_sum.set(b); }
    bool get_do_sum() { return do_sum.get(); }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class EnhancedUnsharpMaskProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, OpParBase* par) 
    {
      std::cout<<"EnhancedUnsharpMaskProc::render() called."<<std::endl;
    }
  };


  template < OP_TEMPLATE_DEF_CS_SPEC >
  class EnhancedUnsharpMaskProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      if( n != 4 ) return;
      if( ireg[0] == NULL ) return;
      if( ireg[1] == NULL ) return;
      if( ireg[2] == NULL ) return;
      if( ireg[3] == NULL ) return;

      EnhancedUnsharpMaskPar* opar = dynamic_cast<EnhancedUnsharpMaskPar*>(par);
      if( !opar ) return;

      PF::ICCProfile* profile = opar->get_profile();
      if( !profile ) return;

      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      T* pin;
      T* pL;
      T* pbh;
      T* pbl;
      T* pout;
      int x, x0, y;
      float bias = profile->perceptual2linear( 0.5 );
      float scale = opar->get_amount();
      bool do_sum = opar->get_do_sum();

      for( y = 0; y < height; y++ ) {
        // original image
        pin = (float*)VIPS_REGION_ADDR( ireg[3], r->left, r->top + y );
        // log-lumi image
        pL = (float*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
        // blurred log-lumi image (high threshold)
        pbh = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        // blurred log-lumi image (low threshold)
        pbl = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        // output image
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x0 = 0, x = 0; x < line_size; x+=3, x0++ ) {
          // compute difference between low and high thresholds
          float l1 = pbl[x0];
          float l2 = pbh[x0];
          float lin = pL[x0];
          if( do_sum ) {
            l1 = xexp10( l1 );
            l2 = xexp10( l2 );
            lin = xexp10( lin );
          }
          float delta = l1 - l2;
          // add the difference back to the log-lumi
          float lout = lin + (delta * scale);
          //std::cout<<"l1="<<l1<<"  l2="<<l2<<"  lwhite2="<<lwhite2<<"  boost="<<boost<<"  out="<<out<<std::endl;

          float in = do_sum ? lin : xexp10( lin );
          float out = do_sum ? lout : xexp10( lout );
          float R = out / in;

          pout[x] = pin[x] * R;
          pout[x+1] = pin[x+1] * R;
          pout[x+2] = pin[x+2] * R;
        }
      }
    }
  };


  ProcessorBase* new_enhanced_usm();

}

#endif 


