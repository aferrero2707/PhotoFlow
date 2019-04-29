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

#ifndef PF_TRC_CONV_H
#define PF_TRC_CONV_H

#include <iostream>

#include "../base/format_info.hh"

namespace PF 
{

  class TRCConvPar: public OpParBase
  {
    Property<bool> perceptual;
    PF::ICCProfile* profile;
    cmsToneCurve* trc;
    cmsBool linear_trc;
    cmsToneCurve *perceptual_trc_inv;
  public:
    TRCConvPar();

    bool has_intensity() { return false; }
    bool has_target_channel() { return false; }

    bool to_perceptual() { return perceptual.get(); }
    PF::ICCProfile* get_profile() { return profile; }
    cmsToneCurve* get_trc() { return trc; }
    cmsBool is_linear_trc() { return linear_trc; }
    cmsToneCurve* get_linear2perceptual_curve() { return perceptual_trc_inv; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF >
  class TRCConvProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      TRCConvPar* opar = dynamic_cast<TRCConvPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;
      int x, y;

      if( false ) std::cout<<"TRCConv: PF_COLORSPACE_ANY"<<std::endl;

      float* pin;
      float* pout;
      if( opar->is_linear_trc() && opar->to_perceptual() ) {
        // convert from linear to perceptual
        cmsToneCurve *perceptual_trc_inv = opar->get_linear2perceptual_curve();
        if( !perceptual_trc_inv ) return;

        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x = 0; x < line_size; x++ ) {
            pout[x] = cmsEvalToneCurveFloat( perceptual_trc_inv, pin[x] );
            if( false && r->left==0 && r->top==0 )
              std::cout<<"TRCConv: in="<<pin[x]<<"    out="<<pout[x]<<std::endl;
          }
        }
      }
    }
  };


  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class TRCConvProc< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      TRCConvPar* opar = dynamic_cast<TRCConvPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;
      int x, y;

      if( false ) std::cout<<"TRCConv: PF_COLORSPACE_RGB"<<std::endl;

      float* pin;
      float* pout;
      if( opar->is_linear_trc() && opar->to_perceptual() ) {
        // convert from linear to perceptual
        cmsToneCurve *perceptual_trc_inv = opar->get_linear2perceptual_curve();
        if( !perceptual_trc_inv ) return;

        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x = 0; x < line_size; x++ ) {
            pout[x] = cmsEvalToneCurveFloat( perceptual_trc_inv, pin[x] );
            if( false && r->left==0 && r->top==0 )
              std::cout<<"TRCConv: in="<<pin[x]<<"    out="<<pout[x]<<std::endl;
          }
        }
      }
    }
  };


  ProcessorBase* new_trcconv();
}

#endif 


