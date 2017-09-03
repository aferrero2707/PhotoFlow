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

#include "convert2lab.hh"
#include "../base/processor.hh"


static void lcms2ErrorLogger(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
  std::cout<<"LCMS2 error: "<<Text<<std::endl;
}


PF::Convert2LabPar::Convert2LabPar(): 
  OpParBase(),
  profile_in( NULL ),
  profile_out( NULL ),
  transform( NULL )
{
  cmsCIExyY white;
  cmsWhitePointFromTemp( &white, 5000 );
  //cmsWhitePointFromTemp( &white, 6500 );
  profile_out = cmsCreateLab4Profile( &white );
  //profile_out = cmsCreateLab4Profile( NULL );
  cmsSetLogErrorHandler( lcms2ErrorLogger );

  set_type( "convert2lab" );
}


VipsImage* PF::Convert2LabPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  if( in[0]->Type == VIPS_INTERPRETATION_LAB ) {
    PF_REF( in[0], "Convert2LabPar::build(): ref input Lab image" );
#ifndef NDEBUG
    std::cout<<"Convert2LabPar::build(): input image already in Lab colorspace, no transform needed"<<std::endl;
#endif
    return in[0];
  }

  cmsHPROFILE profile_in = NULL;
  PF::ICCProfile* iccprof_in = PF::get_icc_profile( in[first] );
  if( iccprof_in )  {
    profile_in = iccprof_in->get_profile();
  }

  if( !profile_in ) 
    return in[0];
  
  char tstr[1024];
  cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
#ifndef NDEBUG
  std::cout<<"convert2lab: Embedded profile found: "<<tstr<<std::endl;
#endif

  cmsHPROFILE profile_out = PF::ICCStore::Instance().get_Lab_profile()->get_profile();

  if( transform )
    cmsDeleteTransform( transform );

  cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_in );
  cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, profile_out );

  transform = cmsCreateTransform( profile_in, 
				  infmt,
				  profile_out, 
				  outfmt,
				  INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
  if( !transform )
    return NULL;

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  
  PF::set_icc_profile( out, PF::ICCStore::Instance().get_Lab_profile() );

  //cmsUInt32Number out_length;
  //cmsSaveProfileToMem( profile_out, NULL, &out_length);
  //void* buf = malloc( out_length );
  //cmsSaveProfileToMem( profile_out, buf, &out_length);
  //vips_image_set_blob( out, VIPS_META_ICC_NAME,
	//	       (VipsCallbackFn) g_free, buf, out_length );

  return out;
}



PF::ProcessorBase* PF::new_convert2lab()
{
  return( new PF::Processor<PF::Convert2LabPar,PF::Convert2LabProc>() );
}
