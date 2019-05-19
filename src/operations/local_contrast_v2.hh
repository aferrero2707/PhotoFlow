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

#ifndef PF_LOCAL_CONTRAST_V2_H
#define PF_LOCAL_CONTRAST_V2_H

#include "../rt/rtengine/sleef.c"
#include "../base/processor.hh"

namespace PF 
{

  class LocalContrastV2Par: public OpParBase
  {
    Property<float> amount;
    Property<float> radius, threshold, white_level;

    ProcessorBase* loglumi;
    ProcessorBase* guided[10];

    PF::ICCProfile* in_profile;
    float threshold_scale[10];
  public:
    LocalContrastV2Par();

    bool has_intensity() { return false; }
    bool needs_caching() {
      return true;
    }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    PF::ICCProfile* get_profile() { return in_profile; }
    float get_amount() { return amount.get(); }
    float get_white_level() { return white_level.get(); }
      
    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class LocalContrastV2Proc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, OpParBase* par) 
    {
      std::cout<<"LocalContrastV2Proc::render() called."<<std::endl;
    }
  };


  template < OP_TEMPLATE_DEF_CS_SPEC >
  class LocalContrastV2Proc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
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

      LocalContrastV2Par* opar = dynamic_cast<LocalContrastV2Par*>(par);
      if( !opar ) return;

      PF::ICCProfile* profile = opar->get_profile();
      if( !profile ) return;

      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      T* pin0;
      T* pin1;
      T* pin2;
      T* pout;
      int x, x0, y;
      float bias = profile->perceptual2linear( 0.5 );
      float white = profile->perceptual2linear( opar->get_white_level() );
      float lwhite = xlog10(white/bias);
      float lwhite2 = xlog10(white/bias);

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
          float scale = opar->get_amount();
          float boost = 1;
          float delta2 = delta;
          if( scale > 0 ) {
            if( delta < 0 ) {
              boost = 2.0f - (lwhite - l2)/lwhite;
              if(boost < 0) boost = 0;
              if(boost > 1.5) boost = 1.5;
              //boost *= 2;
              //scale *= boost;
            }
            if( delta > 0 ) {
              boost = (lwhite2 - l2)/lwhite2;
              //std::cout<<"-->  boost="<<boost<<std::endl;
              if(boost < 0) boost = 0;
              if(boost > 1.5) boost = 1.5;
              //std::cout<<"-->  boost="<<boost<<std::endl;
              //boost *= 2;
              //scale *= boost;
            }
          }
          delta2 *= scale * boost;
          float out = delta2 + l1;
          //std::cout<<"l1="<<l1<<"  l2="<<l2<<"  lwhite2="<<lwhite2<<"  boost="<<boost<<"  out="<<out<<std::endl;

          l1 = xexp10( l1 );
          out = xexp10( out );
          float R = out / l1;

          pout[x] = pin0[x] * R;
          pout[x+1] = pin0[x+1] * R;
          pout[x+2] = pin0[x+2] * R;
        }
      }
    }
  };


  ProcessorBase* new_local_contrast_v2();

}

#endif 


