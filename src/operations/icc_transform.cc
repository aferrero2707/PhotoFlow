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
  transform( NULL ),
  input_cs_type( cmsSigRgbData ),
  output_cs_type( cmsSigRgbData )
{
  set_type("icc_transform" );

  set_default_name( _("ICC transform") );
}


VipsImage* PF::ICCTransformPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( in.size() < first+1 ) {
    return NULL;
  }

  VipsImage* image = in[first];
  if( !image ) {
    return NULL;
  }

  void *data;
  size_t data_length;
  
  in_profile = NULL;
  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
                           &data, &data_length ) ) {
    in_profile = cmsOpenProfileFromMem( data, data_length );
  }

  std::cout<<"ICCTransformPar::build(): image="<<in[0]<<" data="<<data<<" data_length="<<data_length<<std::endl;

  bool in_changed = false;
  if( in_profile ) {
    char tstr[1024];
    cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
//#ifndef NDEBUG
    std::cout<<"icc_transform: embedded profile: "<<in_profile<<std::endl;
    std::cout<<"icc_transform: embedded profile name: "<<tstr<<std::endl;
//#endif
    
    if( in_profile_name != tstr ) {
      in_changed = true;
    }

    input_cs_type = cmsGetColorSpace(in_profile);
  }

  std::cout<<"icc_transform: out_profile="<<out_profile<<std::endl;

  if( transform )
    cmsDeleteTransform( transform );

  transform = NULL;
  if( in_profile && out_profile ) {
    std::cout<<"icc_transform: getting input profile format"<<std::endl;
    cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, in_profile );
    std::cout<<"icc_transform: getting output profile format"<<std::endl;
    cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, out_profile );

    transform = cmsCreateTransform( in_profile,
        infmt,
        out_profile,
        outfmt,
        INTENT_RELATIVE_COLORIMETRIC,
        cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE );
    std::cout<<"icc_transform: transform: "<<transform<<std::endl;
    std::cout<<"icc_transform: in_profile: "<<in_profile<<std::endl;
    std::cout<<"icc_transform: infmt: "<<infmt<<std::endl;
    std::cout<<"icc_transform: outfmt: "<<outfmt<<std::endl;
  }

  if( out_profile) {
    std::cout<<"icc_transform: output profile: "<<out_profile<<std::endl;
    //#ifndef NDEBUG
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"icc_transform: output profile: "<<tstr<<std::endl;
    //#endif
    output_cs_type = cmsGetColorSpace(out_profile);
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

  if( in_profile )  cmsCloseProfile( in_profile );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  /*
  if( out_profile ) {
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( out_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( out_profile, buf, &out_length);
    vips_image_set_blob( out, VIPS_META_ICC_NAME, 
			 (VipsCallbackFn) g_free, buf, out_length );
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"ICCTransformPar::build(): image="<<out<<"  embedded profile: "<<tstr<<std::endl;
  }
  */

  return out;
}



PF::ProcessorBase* PF::new_icc_transform()
{
  return new PF::Processor<PF::ICCTransformPar,PF::ICCTransform>();
}
