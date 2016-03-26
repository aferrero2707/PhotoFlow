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

#include "../base/exif_data.hh"
#include "image_reader.hh"


PF::ImageReaderPar::~ImageReaderPar()
{
	std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
}


VipsImage* PF::ImageReaderPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  bool modified = false;

  if( file_name.get().empty() )
    return NULL;

  
  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( file_name.get() );

  RasterImage* new_raster_image = NULL;
  
  if( i == raster_images.end() ) {
    std::cout<<"ImageReaderPar::build(): creating new RasterImage for file "<<file_name.get()<<std::endl;
    new_raster_image = new RasterImage( file_name.get() );
    if( new_raster_image ) 
      raster_images.insert( make_pair(file_name.get(), new_raster_image) );
  } else {
    //std::cout<<"ImageReaderPar::build(): raster_image found ("<<file_name.get()<<")"<<std::endl;
    new_raster_image = i->second;
    new_raster_image->ref();
  }

  //std::cout<<"ImageReaderPar::build(): raster_image="<<(void*)raster_image<<std::endl;
  //if( raster_image ) std::cout<<"raster_image->get_nref(): "<<raster_image->get_nref()<<std::endl;
  //if( new_raster_image ) std::cout<<"new_raster_image->get_nref(): "<<new_raster_image->get_nref()<<std::endl;

  if( raster_image ) {
    raster_image->unref();
    //std::cout<<"ImageReaderPar::build(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
        raster_images.find( file_name.get() );
      if( i != raster_images.end() ) 
        raster_images.erase( i );
      delete raster_image;
      std::cout<<"ImageReaderPar::build(): raster_image deleted"<<std::endl;
    }
  }

  raster_image = new_raster_image;


  if( !raster_image )
    return NULL;
  
  VipsImage* image = raster_image->get_image( level );
  
  if( !image ) return NULL;

  /*
  {
    size_t exifsz;
    PF::exif_data_t* exif_data;
    if( !vips_image_get_blob( image, PF_META_EXIF_NAME,
        (void**)&exif_data,&exifsz ) ) {
      //std::cout<<"ImageReaderPar::build(): exif_custom_data found in image("<<image<<")"<<std::endl;
    } else {
      //std::cout<<"ImageReaderPar::build(): exif_custom_data not found in image("<<image<<")"<<std::endl;
    }
  }
   */
#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): "<<std::endl
	   <<"input images:"<<std::endl;
  for(int i = 0; i < in.size(); i++) {
    std::cout<<"  "<<(void*)in[i]<<std::endl;
  }
  std::cout<<"image->Interpretation: "<<image->Type<<std::endl;
  std::cout<<"imap: "<<(void*)imap<<std::endl<<"omap: "<<(void*)omap<<std::endl;
#endif

  if( is_map() && image->Bands > 1 ) {
    VipsImage* out;
    int nbands = 1;
    if( vips_extract_band( image, &out, 0, "n", nbands, NULL ) ) {
      std::cout<<"ImageReaderPar::build(): vips_extract_band() failed"<<std::endl;
      return NULL;
    }

    vips_image_init_fields( out,
                            image->Xsize, image->Ysize, 
                            nbands, image->BandFmt,
                            image->Coding,
                            image->Type,
                            1.0, 1.0);
    PF_UNREF( image, "ImageReaderPar::build(): image unref after extract_band" );
    image = out;
  }



#ifndef NDEBUG
  void *data;
  size_t data_length;
  if( !vips_image_get_blob( image, VIPS_META_ICC_NAME, 
			    &data, &data_length ) ) {
    cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
    if( profile_in ) {  
      char tstr[1024];
      cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"ImageReader: Embedded profile found: "<<tstr<<std::endl;
      cmsCloseProfile( profile_in );
    }
  }
#endif


#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): get_format()="<<get_format()<<"  image->BandFmt="<<image->BandFmt<<std::endl;
#endif
  VipsImage* out = image;
  std::vector<VipsImage*> in2;
  in2.push_back( image );
  convert_format->get_par()->set_image_hints( image );
  convert_format->get_par()->set_format( get_format() );
  out = convert_format->get_par()->build( in2, 0, NULL, NULL, level );

  if( !out ) return NULL;
  PF_UNREF( image, "ImageReaderPar::build(): image unref after convert_format" );

#ifndef NDEBUG
  {
    size_t exifsz;
    PF::exif_data_t* exif_data;
    if( !vips_image_get_blob( image, PF_META_EXIF_NAME,
        (void**)&exif_data,&exifsz ) ) {
      std::cout<<"ImageReaderPar::build(): exif_custom_data found in converted image("<<image<<")"<<std::endl;
    } else {
      std::cout<<"ImageReaderPar::build(): exif_custom_data not found in converted image("<<image<<")"<<std::endl;
    }
  }
  {
    size_t exifsz;
    PF::exif_data_t* exif_data;
    if( !vips_image_get_blob( out, PF_META_EXIF_NAME,
        (void**)&exif_data,&exifsz ) ) {
      std::cout<<"ImageReaderPar::build(): exif_custom_data found in out("<<out<<")"<<std::endl;
    } else {
      std::cout<<"ImageReaderPar::build(): exif_custom_data not found in out("<<out<<")"<<std::endl;
    }
  }
#endif

  set_image_hints( out );
#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): image refcount ("<<(void*)image<<") = "<<G_OBJECT(image)->ref_count<<std::endl;
  std::cout<<"                         out refcount ("<<(void*)out<<") = "<<G_OBJECT(out)->ref_count<<std::endl;
#endif


  bool changed = in_profile_mode.is_modified() || in_trc_mode.is_modified() ||
      out_profile_mode.is_modified() || out_trc_mode.is_modified();

  //if( in_profile ) cmsCloseProfile( in_profile );
  //if( out_profile ) cmsCloseProfile( out_profile );
  in_profile = NULL;
  out_profile = NULL;
  profile_type_t ptype;
  TRC_type trc_type;
  PF::ICCProfile* in_iccprof = NULL;

  if( (profile_type_t)in_profile_mode.get_enum_value().first == PF::OUT_PROF_EMBEDDED ) {
    void *data;
    size_t data_length;
    if( !vips_image_get_blob( image, VIPS_META_ICC_NAME,
        &data, &data_length ) ) {
      in_iccprof = PF::ICCStore::Instance().get_profile( data, data_length );
      if( in_iccprof ) {
        in_profile = in_iccprof->get_profile();
        if( in_profile ) {
          char tstr[1024];
          cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
          std::cout<<"ImageReader: Embedded profile found: "<<tstr<<std::endl;
          //cmsCloseProfile( profile_in );
        }
      }
    }
  } else {
    ptype = (profile_type_t)in_profile_mode.get_enum_value().first;
    trc_type = (TRC_type)in_trc_mode.get_enum_value().first;
    std::cout<<"Getting input profile..."<<std::endl;
    in_iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    if( in_iccprof ) {
      in_profile = in_iccprof->get_profile();
      std::cout<<"... OK"<<std::endl;
    } else {
      std::cout<<"... FAILED"<<std::endl;
    }
  }

  ptype = (profile_type_t)out_profile_mode.get_enum_value().first;
  trc_type = (TRC_type)out_trc_mode.get_enum_value().first;
  std::cout<<"Getting output profile..."<<std::endl;
  PF::ICCProfile* out_iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
  if( out_iccprof ) {
    out_profile = out_iccprof->get_profile();
    std::cout<<"... OK"<<std::endl;
  } else {
    std::cout<<"... FAILED"<<std::endl;
  }

  std::cout<<"ImageReaderPar::build(): in_profile="<<in_profile<<std::endl;

  if( changed ) {
    if( transform )
      cmsDeleteTransform( transform );
    transform = NULL;
    if( in_profile && out_profile ) {
      cmsUInt32Number infmt = vips2lcms_pixel_format( out->BandFmt, in_profile );
      cmsUInt32Number outfmt = vips2lcms_pixel_format( out->BandFmt, out_profile );

      transform = cmsCreateTransform( in_profile,
          infmt,
          out_profile,
          outfmt,
          INTENT_RELATIVE_COLORIMETRIC,
          cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE );
    } else if( !out_profile ) {
      out_iccprof = in_iccprof;
      out_profile = in_profile;
    }
  } else if( !out_profile ) {
    out_iccprof = in_iccprof;
    out_profile = in_profile;
  }

  std::cout<<"ImageReaderPar::build(): out_profile="<<out_profile<<std::endl;
  std::cout<<"ImageReaderPar::build(): transform="<<transform<<std::endl;

  if( out && out_profile ) {

    if( transform ) {
      cmsColorSpaceSignature output_cs_type = cmsGetColorSpace(out_profile);
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

      std::vector<VipsImage*> in2; in2.push_back( out );
      VipsImage* converted = OpParBase::build( in2, 0, NULL, NULL, level );

      out = converted;
    }

    /*
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( out_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( out_profile, buf, &out_length);
    vips_image_set_blob( out, VIPS_META_ICC_NAME,
       (VipsCallbackFn) g_free, buf, out_length );
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"ImageReaderPar::build(): image="<<out<<"  embedded profile: "<<tstr<<std::endl;

    if( out_iccprof ) {
      PF::ICCProfileData* iccdata = out_iccprof->get_data();
      vips_image_set_blob( out, "pf-icc-profile-data",
          (VipsCallbackFn) PF::free_icc_profile_data, iccdata, sizeof(PF::ICCProfileData) );
    }
    */
    if( out_iccprof ) {
      std::cout<<"ImageReaderPar::build(): setting embedded profile..."<<std::endl;
      PF::set_icc_profile( out, out_iccprof );
      PF::print_embedded_profile( out );
    }
  }

  return out;
}


PF::ProcessorBase* PF::new_image_reader()
{
  return new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
}
