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

//#include <arpa/inet.h>

#include "convert_colorspace.hh"

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../dt/common/colorspaces.h"
//#include "../base/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

//#include "../dt/external/adobe_coeff.c"
//#include "../vips/vips_layer.h"


PF::ConvertColorspacePar::ConvertColorspacePar(): 
  OpParBase(),
  out_profile_mode("profile_mode",this,PF::OUT_PROF_sRGB,"sRGB","Built-in sRGB"),
  out_profile_name("profile_name", this),
  out_profile_data( NULL ),
  transform( NULL ),
  input_cs_type( cmsSigRgbData ),
  output_cs_type( cmsSigRgbData )
{
  convert2lab = PF::new_convert2lab();

  out_profile_mode.add_enum_value(PF::OUT_PROF_NONE,"NONE","NONE");
  //out_profile_mode.add_enum_value(PF::OUT_PROF_sRGB,"sRGB","Built-in sRGB");
  out_profile_mode.add_enum_value(PF::OUT_PROF_ADOBE,"ADOBE","Built-in Adobe RGB 1998");
  out_profile_mode.add_enum_value(PF::OUT_PROF_PROPHOTO,"PROPHOTO","Built-in ProPhoto RGB");
  out_profile_mode.add_enum_value(PF::OUT_PROF_LAB,"LAB","Lab");
  out_profile_mode.add_enum_value(PF::OUT_PROF_CUSTOM,"CUSTOM","Custom");

  set_type("convert_colorspace" );
}


VipsImage* PF::ConvertColorspacePar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( in.size() < first+1 ) {
    out_profile_data = NULL;
    out_profile_data_length = 0;
    return NULL;
  }

  VipsImage* image = in[first];
  if( !image ) {
    out_profile_data = NULL;
    out_profile_data_length = 0;
    return NULL;
  }

  void *data;
  size_t data_length;
  
  if( vips_image_get_blob( in[0], VIPS_META_ICC_NAME, 
                           &data, &data_length ) ) {
    out_profile_data = NULL;
    out_profile_data_length = 0;
    return NULL;
  }

  cmsHPROFILE in_profile = cmsOpenProfileFromMem( data, data_length );

  bool in_changed = false;
  if( in_profile ) {
    char tstr[1024];
    cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
#ifndef NDEBUG
    std::cout<<"convert2lab: Embedded profile found: "<<tstr<<std::endl;
#endif
    
    if( in_profile_name != tstr ) {
      in_changed = true;
    }

    input_cs_type = cmsGetColorSpace(in_profile);
  }

  bool out_mode_changed = out_profile_mode.is_modified();
  bool out_changed = out_profile_name.is_modified();

  bool changed = in_changed || out_mode_changed || out_changed;

  cmsHPROFILE out_profile = NULL;
  if( changed ) {
    //std::cout<<"ConvertColorspacePar::build(): out_mode_changed="<<out_mode_changed
    //         <<"  out_changed="<<out_changed<<"  out_profile="<<out_profile<<std::endl;
    //std::cout<<"  out_profile_mode="<<out_profile_mode.get_enum_value().first<<std::endl;
    switch( out_profile_mode.get_enum_value().first ) {
    case OUT_PROF_sRGB:
      out_profile = dt_colorspaces_create_srgb_profile();
      //std::cout<<"ConvertColorspacePar::build(): created sRGB output profile"<<std::endl;
      break;
    case OUT_PROF_ADOBE:
      out_profile = dt_colorspaces_create_adobergb_profile();
      //std::cout<<"ConvertColorspacePar::build(): created AdobeRGB output profile"<<std::endl;
      break;
    case OUT_PROF_PROPHOTO:
      out_profile = dt_colorspaces_create_prophotorgb_profile();
      //std::cout<<"ConvertColorspacePar::build(): created ProPhoto output profile"<<std::endl;
      break;
    case OUT_PROF_LAB:
      out_profile = dt_colorspaces_create_lab_profile();
      //std::cout<<"ConvertColorspacePar::build(): created Lab output profile"<<std::endl;
      break;
    case OUT_PROF_CUSTOM:
      //std::cout<<"  custom profile selected: \""<<cam_profile_name.get()<<"\""<<std::endl;
      if( out_profile_data && out_profile_data_length>0 ) 
        out_profile = cmsOpenProfileFromMem( out_profile_data, out_profile_data_length );
      else if( !out_profile_name.get().empty() )
        out_profile = cmsOpenProfileFromFile( out_profile_name.get().c_str(), "r" );
      break;
    default:
      break;
    }

    if( transform )
      cmsDeleteTransform( transform );  

    transform = NULL;
    if( in_profile && out_profile ) {
      cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, in_profile );
      cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, out_profile );

      transform = cmsCreateTransform( in_profile, 
                                      infmt,
                                      out_profile, 
                                      outfmt,
                                      INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
    }

    if( out_profile) {
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
  }
  //std::cout<<"ConvertColorspacePar::build(): transform="<<transform<<std::endl;

  if( !in_profile && out_profile ) {
    // The input profile was not specified, so we simply assign the output
    // profile without any conversion
  }

  if( !transform ) {
    if( in_profile )  cmsCloseProfile( in_profile );
    if( out_profile ) cmsCloseProfile( out_profile );
    out_profile_data = NULL;
    out_profile_data_length = 0;
    return NULL;
  }

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  /**/
  if( out_profile ) {
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( out_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( out_profile, buf, &out_length);
    vips_image_set_blob( out, VIPS_META_ICC_NAME, 
			 (VipsCallbackFn) g_free, buf, out_length );
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    //std::cout<<"ConvertColorspacePar::build(): image="<<out<<"  embedded profile: "<<tstr<<std::endl;
  }
  /**/

  if( in_profile )  cmsCloseProfile( in_profile );
  if( out_profile ) cmsCloseProfile( out_profile );
  out_profile_data = NULL;
  out_profile_data_length = 0;

  return out;
}



PF::ProcessorBase* PF::new_convert_colorspace()
{
  return new PF::Processor<PF::ConvertColorspacePar,PF::ConvertColorspace>();
}
