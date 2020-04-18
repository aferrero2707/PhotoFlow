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
  trc(NULL),
  linear_trc(false),
  perceptual_trc_inv(NULL)
{
  set_type( "trc_conv" );

  /* LAB "L" (perceptually uniform) TRC */
  cmsFloat64Number labl_parameters[5] =
  { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
  cmsToneCurve *labl_parametic_curve =
      cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
  perceptual_trc_inv = cmsReverseToneCurve( labl_parametic_curve );

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

  cmsHPROFILE iccprof  = profile->get_profile();
  if( !iccprof ) {
    PF_REF( image, "TRCConvPar::build(): image ref for NULL ICC profile");
    return image;
  }

  trc   = (cmsToneCurve*)cmsReadTag(iccprof, cmsSigRedTRCTag);
  //cmsToneCurve *green_trc = (cmsToneCurve*)cmsReadTag(profile, cmsSigGreenTRCTag);
  //cmsToneCurve *blue_trc  = (cmsToneCurve*)cmsReadTag(profile, cmsSigBlueTRCTag);

  if( !trc ) {
    PF_REF( image, "TRCConvPar::build(): image ref for NULL TRC");
    return image;
  };


  linear_trc = cmsIsToneCurveLinear(trc);
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
          //if( true && r->left==0 && r->top==0 )
            //std::cout<<"TRCConv: in["<<y<<","<<x<<"]="<<pin[x]<<"    out="<<pout[x]<<std::endl;
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



PF::ProcessorBase* PF::new_trcconv()
{ return( new PF::Processor<PF::TRCConvPar,TRCConvProc>() ); }
