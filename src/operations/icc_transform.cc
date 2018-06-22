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

#include "icc_transform.hh"



PF::ICCTransformPar::ICCTransformPar():
  OpParBase(),
  out_profile( NULL ),
  intent( INTENT_RELATIVE_COLORIMETRIC ),
  bpc( true ),
  adaptation_state(-1),
  input_cs_type( cmsSigRgbData ),
  output_cs_type( cmsSigRgbData ),
  clip_negative(false),
  clip_overflow(false)
{
  set_type("icc_transform" );

  set_default_name( _("ICC transform") );
}


VipsImage* PF::ICCTransformPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (int)in.size() < first+1 ) {
    return NULL;
  }

  VipsImage* image = in[first];
  if( !image ) {
    return NULL;
  }

  void *data;
  size_t data_length;
  
  in_profile = PF::get_icc_profile( in[first] );

  if( !in_profile || !out_profile ) {
    PF_REF( in[first], "ICCTransformPar::build(): input image ref for missing input or output profiles" );
    std::cout<<"ICCTransformPar::build(): missing input or output profiles, no transform needed"<<std::endl;
    return in[first];
  }


  bool matching = false;
  if( in_profile && out_profile && in_profile->equals_to(out_profile) ) {
    matching = true;
  }

  if( matching ) {
    PF_REF( in[first], "ICCTransformPar::build(): input image ref for equal input and output profiles" );
    std::cout<<"ICCTransformPar::build(): matching input and output profiles, no transform needed"<<std::endl;
    return in[first];
  }

  //std::cout<<"ICCTransformPar::build(): image="<<in[0]<<" data="<<data<<" data_length="<<data_length<<std::endl;

  bool in_changed = false;
  if( in_profile && in_profile->get_profile() ) {
    char tstr[1024];
    cmsGetProfileInfoASCII(in_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
#ifndef NDEBUG
    std::cout<<"icc_transform: embedded profile: "<<in_profile<<std::endl;
    std::cout<<"icc_transform: embedded profile name: "<<tstr<<std::endl;
#endif
    
    if( in_profile_name != tstr ) {
      in_changed = true;
    }

    input_cs_type = cmsGetColorSpace( in_profile->get_profile() );
  }

#ifndef NDEBUG
  if( out_profile )
    std::cout<<"icc_transform: out_profile="<<out_profile<<" ("<<out_profile->get_profile()<<")"<<std::endl;
#endif

  if( in_profile && out_profile && out_profile->get_profile() ) {
    transform.init( in_profile, out_profile, in[0]->BandFmt, intent, get_bpc(), adaptation_state );
  }

  if( out_profile && out_profile->get_profile() ) {
#ifndef NDEBUG
    std::cout<<"icc_transform: output profile: "<<out_profile<<std::endl;
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"icc_transform: output profile: "<<tstr<<std::endl;
#endif
    output_cs_type = cmsGetColorSpace( out_profile->get_profile() );
    switch( output_cs_type ) {
    case cmsSigGrayData:
      grayscale_image( get_xsize(), get_ysize() );
      break;
    case cmsSigRgbData:
      rgb_image( get_xsize(), get_ysize() );
      break;
    case cmsSigLabData:
      lab_image( get_xsize(), get_ysize() );
      break;
    case cmsSigCmykData:
      cmyk_image( get_xsize(), get_ysize() );
      break;
    default:
      break;
    }
  }

  //if( in_profile )  cmsCloseProfile( in_profile );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );

  if( out && out_profile ) {
    PF::set_icc_profile( out, out_profile );
  }

  return out;
}
