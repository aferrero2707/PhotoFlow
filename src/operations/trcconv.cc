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

#include "../base/processor.hh"
#include "../base/processor_imp.hh"
#include "trcconv.hh"

using namespace PF;

PF::TRCConvPar::TRCConvPar():
  OpParBase(),
  perceptual("perceptual",this,true),
  profile(NULL),
  linear_trc(false)
{
  set_type( "trc_conv" );

  set_default_name( _("TRC conv") );
}


VipsImage* PF::TRCConvPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size() < first+1 ) {
    return NULL;
  }

  VipsImage* image = in[0];
  if( !image ) {
    return NULL;
  }

  profile = PF::get_icc_profile( image );
  if( !profile ) {
    PF_REF( image, "TRCConvPar::build(): image ref for NULL profile");
    return image;
  }

  linear_trc = profile->is_linear();
  if( linear_trc && !perceptual.get() ) {
    PF_REF( image, "TRCConvPar::build(): image ref for linear TRC");
    return image;
  }
  if( !linear_trc && perceptual.get() ) {
    PF_REF( image, "TRCConvPar::build(): image ref for perceptual TRC");
    return image;
  }

  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );
  return out;
}





template < OP_TEMPLATE_DEF >
class TRCConvProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};

template < OP_TEMPLATE_DEF_TYPE_SPEC >
class TRCConvProc<OP_TEMPLATE_IMP_TYPE_SPEC(float)>
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
    int height = r->height;
    int x, y;

    float* pin;
    float* pout;
    PF::ICCProfile* profile = opar->get_profile();

    if( false ) std::cout<<"TRCConv: PF_COLORSPACE_ANY"<<std::endl;

    if( CS == PF_COLORSPACE_RGB || CS == PF_COLORSPACE_GRAYSCALE) {
      if( profile->is_linear() && opar->to_perceptual() ) {
        // convert from linear to perceptual

        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x = 0; x < line_size; x++ ) {
            pout[x] = profile->linear2perceptual(pin[x]);
            if( false && r->left==0 && r->top==0 )
              std::cout<<"TRCConv: in="<<pin[x]<<"    out="<<pout[x]<<std::endl;
          }
        }
      }
      if( !profile->is_linear() && !opar->to_perceptual() ) {
        // convert from perceptual to linear

        for( y = 0; y < height; y++ ) {
          pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
          pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x = 0; x < line_size; x++ ) {
            pout[x] = profile->perceptual2linear(pin[x]);
            if( false && r->left==0 && r->top==0 )
              std::cout<<"TRCConv: in="<<pin[x]<<"    out="<<pout[x]<<std::endl;
          }
        }
      }
    } else {
      // do nothing
      for( y = 0; y < height; y++ ) {
        pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
        memcpy(pout, pin, sizeof(float) * line_size);
      }
    }
  }
};


PF::ProcessorBase* PF::new_trcconv()
{ return( new PF::Processor<PF::TRCConvPar,TRCConvProc>() ); }
