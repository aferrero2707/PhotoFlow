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
#include "icc_transform.hh"
#include "image_reader.hh"


PF::ImageReaderPar::ImageReaderPar():
OpParBase(),
file_name("file_name", this),
//out_profile_mode("profile_mode",this,PF::PROF_TYPE_REC2020,"REC2020","Rec.2020"),
in_profile_mode("in_profile_mode",this,PF::PROF_MODE_EMBEDDED_sRGB,"EMBEDDED",_("embedded")),
in_profile_type("in_profile_type",this,PF::PROF_TYPE_REC2020,"REC2020",_("Rec.2020")),
in_trc_type("in_trc_type",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
in_profile_name("in_profile_name",this),
//out_profile_mode("out_profile_mode",this,PF::PROF_TYPE_EMBEDDED,"EMBEDDED",_("same")),
out_profile_mode("out_profile_mode",this,PF::PROF_MODE_DEFAULT,"DEFAULT",_("default")),
out_profile_type("out_profile_type",this,PF::PROF_TYPE_REC2020,"REC2020",_("Rec.2020")),
//out_profile_type("out_profile_type",this,PF::PROF_TYPE_EMBEDDED,"EMBEDDED",_("use input")),
out_trc_type("out_trc_type",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
out_profile_name("out_profile_name",this),
image(NULL),
current_format(VIPS_FORMAT_NOTSET),
raster_image( NULL )
{
  //in_profile_mode.add_enum_value(PF::PROF_MODE_EMBEDDED,"EMBEDDED",_("embedded"));
  //in_profile_mode.add_enum_value(PF::PROF_MODE_EMBEDDED,"EMBEDDED_sRGB",_("embedded (sRGB)"));
  in_profile_mode.add_enum_value(PF::PROF_MODE_NONE,"NONE",_("none"));
  in_profile_mode.add_enum_value(PF::PROF_MODE_CUSTOM,"CUSTOM",_("custom"));
  in_profile_mode.add_enum_value(PF::PROF_MODE_ICC,"ICC",_("ICC from disk"));

  //in_profile_type.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  in_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  in_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  in_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  in_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  in_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  in_profile_type.add_enum_value(PF::PROF_TYPE_XYZ,"XYZ","XYZ D50");
  //in_profile_type.add_enum_value(PF::PROF_TYPE_CUSTOM,"CUSTOM","Custom");

  //out_profile_mode.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  out_profile_mode.add_enum_value(PF::PROF_MODE_EMBEDDED,"EMBEDDED",_("use input"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_CUSTOM,"CUSTOM",_("custom"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_ICC,"ICC",_("ICC from disk"));

  out_profile_type.add_enum_value(PF::PROF_TYPE_EMBEDDED,"EMBEDDED",_("use input"));
  out_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  out_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  //out_profile_type.add_enum_value(PF::PROF_TYPE_CUSTOM,"CUSTOM","Custom");
  out_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  out_profile_type.add_enum_value(PF::PROF_TYPE_XYZ,"XYZ","XYZ D50");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_SETTINGS,"FROM_SETTINGS","from settings");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_DISK,"FROM_DISK","ICC from disk");


  //in_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR","linear");
  in_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL","perceptual");
  in_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

  //out_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR","linear");
  out_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL","perceptual");
  out_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

  convert_format = new_convert_format();
  blender = new_blender();
  cs_transform = new_icc_transform();

  set_type("imageread" );

  set_default_name( _("image layer") );
}


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

  delete convert_format;
  delete blender;
  delete cs_transform;
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
    std::cout<<"ImageReaderPar::build(): RasterImage for file "<<file_name.get()<<" created"<<std::endl;
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

  VipsImage* tmp;
  vips_copy( image, &tmp, NULL );
  PF_UNREF( image, "ImageReaderPar::build(): image unref after vips_copy" );
  image = tmp;

  if( image->Bands == 1 ) {

    VipsImage* out;
    VipsImage* bandv[3];
    bandv[0] = image;
    bandv[1] = image;
    bandv[2] = image;
    if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
      PF_UNREF( image, "ImageReaderPar::build(): image unref after bandjoin failure" );
      return NULL;
    }
    PF_UNREF( image, "ImageReaderPar::build(): image unref" );
    rgb_image( out->Xsize, out->Ysize );

    vips_image_init_fields( out,
        image->Xsize, image->Ysize,
        3, image->BandFmt,
        VIPS_CODING_NONE,
        VIPS_INTERPRETATION_RGB,
        1.0, 1.0);

    image = out;
    std::cout<<"ImageReaderPar::build(): converted from 1 to 3 bands"<<std::endl;
    std::cout<<"ImageReaderPar::build(): image->Type"<<image->Type<<std::endl;

  }



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
  std::cout<<"image: "<<image<<"    image->Interpretation: "<<image->Type<<std::endl;
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
  std::cout<<std::endl<<"================="<<std::endl;
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
    if( !PF_VIPS_IMAGE_GET_BLOB( image, PF_META_EXIF_NAME, &exif_data, &exifsz ) ) {
      std::cout<<"ImageReaderPar::build(): exif_custom_data found in converted image("<<image<<")"<<std::endl;
    } else {
      std::cout<<"ImageReaderPar::build(): exif_custom_data not found in converted image("<<image<<")"<<std::endl;
    }
  }
  {
    size_t exifsz;
    PF::exif_data_t* exif_data;
    if( !PF_VIPS_IMAGE_GET_BLOB( out, PF_META_EXIF_NAME, &exif_data, &exifsz ) ) {
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


  bool changed = in_profile_mode.is_modified() || in_profile_type.is_modified() || in_trc_type.is_modified() ||
      out_profile_mode.is_modified() || out_profile_type.is_modified() || out_trc_type.is_modified();

  //if( in_profile ) cmsCloseProfile( in_profile );
  //if( out_profile ) cmsCloseProfile( out_profile );
  profile_type_t ptype;
  //profile_mode_t pmode;
  TRC_type trc_type;
  PF::ICCProfile* in_iccprof = NULL;

  if( (profile_mode_t)in_profile_mode.get_enum_value().first == PF::PROF_MODE_NONE ) {
    // do nothing
  } else if( (profile_mode_t)in_profile_mode.get_enum_value().first == PF::PROF_MODE_EMBEDDED ||
      (profile_mode_t)in_profile_mode.get_enum_value().first == PF::PROF_MODE_EMBEDDED_sRGB ) {
    void *data;
    size_t data_length;
    if( !PF_VIPS_IMAGE_GET_BLOB( image, VIPS_META_ICC_NAME, &data, &data_length ) ) {
      in_iccprof = PF::ICCStore::Instance().get_profile( data, data_length );
      if( in_iccprof ) {
        cmsHPROFILE in_profile = in_iccprof->get_profile();
        if( false && in_profile ) {
          char tstr[1024];
          cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
          std::cout<<"ImageReader: Embedded profile found: "<<tstr<<std::endl;
          //cmsCloseProfile( profile_in );
        }
      }
    } else {
      in_iccprof = PF::ICCStore::Instance().get_profile( PROF_TYPE_sRGB, PF_TRC_STANDARD );
      if( in_iccprof ) {
        cmsHPROFILE in_profile = in_iccprof->get_profile();
        if( false && in_profile ) {
          char tstr[1024];
          cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
          std::cout<<"ImageReader: using default profile: "<<tstr<<std::endl;
          //cmsCloseProfile( profile_in );
        }
      }
    }
  } else if( (profile_mode_t)in_profile_mode.get_enum_value().first == PF::PROF_MODE_ICC ) {
    in_iccprof = PF::ICCStore::Instance().get_profile( in_profile_name.get() );
    if( in_iccprof ) {
      cmsHPROFILE in_profile = in_iccprof->get_profile();
      if( false && in_profile ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(in_profile, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<"ImageReader: custom input profile from disk: "<<tstr<<std::endl;
      }
    }
  } else {
    ptype = (profile_type_t)in_profile_type.get_enum_value().first;
    trc_type = (TRC_type)in_trc_type.get_enum_value().first;
    //std::cout<<"Getting input profile..."<<std::endl;
    in_iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    if( in_iccprof ) {
      //std::cout<<"... OK"<<std::endl;
    } else {
      std::cout<<"ImageReader: cannot load input profile"<<std::endl;
    }
  }

  PF::ICCProfile* out_iccprof = NULL;
  if( (profile_mode_t)in_profile_mode.get_enum_value().first != PF::PROF_MODE_NONE ) {
    PF::set_icc_profile( out, in_iccprof );
    // only retrieve the working profile if the image is color managed
    ptype = (profile_type_t)out_profile_type.get_enum_value().first;
    if( ptype == PF::PROF_TYPE_EMBEDDED ) {
      // do nothing
      //std::cout<<"Using embedded profile"<<std::endl;
      out_iccprof = in_iccprof;
    } else if( ptype == PF::PROF_TYPE_FROM_SETTINGS && in_iccprof ) {
      ptype = PF::PhotoFlow::Instance().get_options().get_working_profile_type();
      trc_type = PF::PhotoFlow::Instance().get_options().get_working_trc_type();
      //std::cout<<"Getting output profile..."<<std::endl;
      out_iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    } else if( ptype == PF::PROF_TYPE_FROM_DISK && in_iccprof ) {
      out_iccprof = PF::ICCStore::Instance().get_profile( out_profile_name.get() );
    } else {//if( pmode == PF::PROF_MODE_CUSTOM && in_iccprof ) {
      ptype = (profile_type_t)out_profile_type.get_enum_value().first;
      trc_type = (TRC_type)out_trc_type.get_enum_value().first;
      //std::cout<<"Getting output profile..."<<std::endl;
      out_iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    }
    if( out_iccprof ) {
      //std::cout<<"... OK"<<std::endl;
    } else {
      std::cout<<"ImageReader: cannot load output profile"<<std::endl;
    }

    if( in_iccprof && out_iccprof && in_iccprof->get_profile() != out_iccprof->get_profile() ) {
      PF::ICCTransformPar* tr_par =
          dynamic_cast<PF::ICCTransformPar*>( cs_transform->get_par() );
      std::vector<VipsImage*> in2;
      in2.push_back( out );
      tr_par->set_image_hints( out );
      tr_par->set_format( get_format() );
      tr_par->set_out_profile( out_iccprof );
      tr_par->set_bpc( false );
      tr_par->set_adaptation_state( 0.f );
      tr_par->set_clip_negative(false);
      tr_par->set_clip_overflow(false);
      VipsImage* out2 = tr_par->build( in2, 0, NULL, NULL, level );
      if( out2 ) {
        PF_UNREF( out, "RawOutputPar::build(): out unref" );
      }
      out = out2;
    }
  }

  //PF::print_embedded_profile( out );
  return out;
}
