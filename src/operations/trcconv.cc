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
#include "trcconv.hh"

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



PF::ProcessorBase* PF::new_trcconv()
{
  return( new PF::Processor<PF::TRCConvPar,PF::TRCConvProc>() );
}
