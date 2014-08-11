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


#include <stdlib.h>

#include "convert2srgb.hh"
#include "../base/processor.hh"


static cmsUInt32Number vips2lcms_pixel_format( VipsBandFormat vipsFmt, cmsHPROFILE pin )
{
  cmsUInt32Number result;
  switch( vipsFmt ) {
  case VIPS_FORMAT_UCHAR:
  case VIPS_FORMAT_CHAR:
    switch( cmsGetColorSpace( pin ) ) {
    case cmsSigRgbData:
      result = TYPE_RGB_8;
      break;
    case cmsSigLabData:
      result = TYPE_Lab_8;
      break;
    case cmsSigCmykData:
      result = TYPE_CMYK_8;
      break;
    default: break;
    }
    break;
  case VIPS_FORMAT_USHORT:
  case VIPS_FORMAT_SHORT:
    // short int is 16-bit
#if (USHRT_MAX == 65535U)
    switch( cmsGetColorSpace( pin ) ) {
    case cmsSigRgbData:
      result = TYPE_RGB_16;
      break;
    case cmsSigLabData:
      result = TYPE_Lab_16;
      break;
    case cmsSigCmykData:
      result = TYPE_CMYK_16;
      break;
    default: break;
    }
#endif
    break;
  case VIPS_FORMAT_UINT:
  case VIPS_FORMAT_INT:
#if (UINT_MAX == 65535U)
    switch( cmsGetColorSpace( pin ) ) {
    case cmsSigRgbData:
      result = TYPE_RGB_16;
      break;
    case cmsSigLabData:
      result = TYPE_Lab_16;
      break;
    case cmsSigCmykData:
      result = TYPE_CMYK_16;
      break;
    default: break;
    }
#endif
    break;
  case VIPS_FORMAT_FLOAT:
    switch( cmsGetColorSpace( pin ) ) {
    case cmsSigRgbData:
      result = TYPE_RGB_FLT;
      break;
    case cmsSigLabData:
      result = TYPE_Lab_FLT;
      break;
    case cmsSigCmykData:
      result = TYPE_CMYK_FLT;
      break;
    default: break;
    }
    break;    
  case VIPS_FORMAT_DOUBLE:
    switch( cmsGetColorSpace( pin ) ) {
    case cmsSigRgbData:
      result = TYPE_RGB_DBL;
      break;
    case cmsSigLabData:
      result = TYPE_Lab_DBL;
      break;
    case cmsSigCmykData:
      result = TYPE_CMYK_DBL;
      break;
    default: break;
    }
    break;    
  default:
    break;
  }
  return result;
}



static void lcms2ErrorLogger(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
  std::cout<<"LCMS2 error: "<<Text<<std::endl;
}


PF::Convert2sRGBPar::Convert2sRGBPar(): 
  OpParBase(),
  profile_in( NULL ),
  profile_out( NULL ),
  transform( NULL )
{
  profile_out = cmsCreate_sRGBProfile();
  cmsSetLogErrorHandler( lcms2ErrorLogger );

  set_type( "convert2srgb" );
}


VipsImage* PF::Convert2sRGBPar::build(std::vector<VipsImage*>& in, int first, 
				      VipsImage* imap, VipsImage* omap, 
				      unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  if( transform )
    cmsDeleteTransform( transform );
  transform = NULL;

  if( profile_in )
    cmsCloseProfile( profile_in );
  profile_in  = NULL;

  if( profile_out && 
      !vips_image_get_blob( in[0], VIPS_META_ICC_NAME, 
			    &data, &data_length ) ) {
    
    profile_in = cmsOpenProfileFromMem( data, data_length );
    if( profile_in ) {
  
#ifndef NDEBUG    
      char tstr[1024];
      cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"convert2rgb: Embedded profile found: "<<tstr<<std::endl;
#endif
      
      cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_in );
      cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_out );
      
      transform = cmsCreateTransform( profile_in, 
				      infmt,
				      profile_out, 
				      outfmt,
				      INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
    }
  }

  if( !transform ) {
		std::cout<<"Convert2sRGBPar::build(): null transform"<<std::endl;
    PF_REF( in[0], "Convert2sRGBPar::build(): null transform" );
    return( in[0] );
  }

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  /**/
  cmsUInt32Number out_length;
  cmsSaveProfileToMem( profile_out, NULL, &out_length);
  void* buf = malloc( out_length );
  cmsSaveProfileToMem( profile_out, buf, &out_length);
  vips_image_set_blob( out, VIPS_META_ICC_NAME, 
		       (VipsCallbackFn) g_free, buf, out_length );
  /**/
  return out;
}



PF::ProcessorBase* PF::new_convert2srgb()
{
  return( new PF::Processor<PF::Convert2sRGBPar,
			    PF::Convert2sRGBProc>() );
}
