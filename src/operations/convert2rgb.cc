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


#include "convert2rgb.hh"
#include "../base/processor.hh"


static void lcms2ErrorLogger(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
  std::cout<<"LCMS2 error: "<<Text<<std::endl;
}


PF::Convert2RGBPar::Convert2RGBPar(): 
  OpParBase(),
  profile_in( NULL ),
  profile_out( NULL ),
  transform( NULL )
{
  profile_out = cmsOpenProfileFromFile( "/usr/share/color/icc/sRGB.icc", "r" );
  cmsSetLogErrorHandler( lcms2ErrorLogger );

  set_type( "convert2rgb" );
}


VipsImage* PF::Convert2RGBPar::build(std::vector<VipsImage*>& in, int first, 
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
  
      char tstr[1024];
      cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"convert2rgb: Embedded profile found: "<<tstr<<std::endl;
      
      cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_in );
      cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_out );
      
      transform = cmsCreateTransform( profile_in, 
				      infmt,
				      profile_out, 
				      outfmt,
				      INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
    }
  }

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  /**/
  cmsUInt32Number out_length;
  cmsSaveProfileToMem( profile_out, NULL, &out_length);
  void* buf = malloc( out_length );
  cmsSaveProfileToMem( profile_out, buf, &out_length);
  vips_image_set_blob( out, VIPS_META_ICC_NAME, 
		       (VipsCallbackFn) g_free, buf, out_length );
  char tstr[1024];
  cmsGetProfileInfoASCII(profile_out, cmsInfoDescription, "en", "US", tstr, 1024);
  std::cout<<"convert2rgb: Output profile: "<<tstr<<std::endl;
      
  /**/

  return out;
}



PF::ProcessorBase* PF::new_convert2rgb()
{
  return( new PF::Processor<PF::Convert2RGBPar,PF::Convert2RGBProc>() );
}
