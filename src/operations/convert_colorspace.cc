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

#include "icc_transform.hh"
#include "convert_colorspace.hh"

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../external/darktable/src/common/colorspaces.h"
//#include "../base/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

//#include "../dt/external/adobe_coeff.c"
//#include "../vips/vips_layer.h"







PF::ConvertColorspacePar::ConvertColorspacePar(): 
          OpParBase(),
          //out_profile_mode("profile_mode",this,PF::PROF_TYPE_sRGB,"sRGB","Built-in sRGB"),
          out_profile_mode("profile_mode2",this,PF::PROF_MODE_DEFAULT,"DEFAULT",_("default")),
          out_profile_type("profile_mode",this,PF::PROF_TYPE_REC2020,"REC2020","Rec.2020"),
          //out_profile_type("profile_mode",this,PF::PROF_TYPE_sRGB,"sRGB","sRGB"),
          //out_trc_type("trc_type",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
          out_trc_type("trc_type",this,PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard")),
          out_profile_name("profile_name", this),
          intent("rendering_intent",this,INTENT_RELATIVE_COLORIMETRIC,"INTENT_RELATIVE_COLORIMETRIC","relative colorimetric"),
          bpc("bpc", this, false),
          adaptation_state("adaptation_state", this, 0.f),
          assign("assign", this, false),
          clip_negative("clip_negative",this,true),
          clip_overflow("clip_overflow",this,true),
          out_profile_data( NULL ),
          transform( NULL ),
          gw_transform_in( NULL ),
          gw_transform_out( NULL ),
          softproof( false ),
          gamut_warning( false ),
          input_cs_type( cmsSigRgbData ),
          output_cs_type( cmsSigRgbData )
{
  //convert2lab = PF::new_convert2lab();

  //out_profile_mode.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  //out_profile_mode.add_enum_value(PF::PROF_MODE_EMBEDDED,"EMBEDDED",_("use input"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_CUSTOM,"CUSTOM",_("custom"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_ICC,"ICC",_("ICC from disk"));

  //out_profile_type.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  out_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  //out_profile_type.add_enum_value(PF::PROF_TYPE_sRGB_D50,"sRGB_D50","sRGB D50");
  out_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  out_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  out_profile_type.add_enum_value(PF::PROF_TYPE_XYZ,"XYZ","XYZ D50");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_SETTINGS,"FROM_SETTINGS","from settings");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_DISK,"FROM_DISK","ICC from disk");

  out_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR",_("linear"));
  out_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL",_("perceptual"));
  //out_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

  intent.add_enum_value( INTENT_PERCEPTUAL, "INTENT_PERCEPTUAL", "perceptual" );
  intent.add_enum_value( INTENT_ABSOLUTE_COLORIMETRIC, "INTENT_ABSOLUTE_COLORIMETRIC", "absolute colorimetric" );
  intent.add_enum_value( INTENT_SATURATION, "INTENT_SATURATION", "saturation" );

  cs_transform     = new_icc_transform();
  gw_transform_in  = new_icc_transform();
  gw_transform_out = new_icc_transform();
  gw = new_gamut_warning();

  set_type("convert_colorspace" );

  set_default_name( _("colorspace conversion") );
}


PF::ConvertColorspacePar::~ConvertColorspacePar()
{
  delete cs_transform;
  delete gw_transform_in;
  delete gw_transform_out;
  delete gw;
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

  cmsHPROFILE in_profile = NULL;
  PF::ICCProfile* iccprof_in = PF::get_icc_profile( in[first] );
  if( iccprof_in )  {
    in_profile = iccprof_in->get_profile();
  }

  /*
  void *data;
  size_t data_length;

  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
                           &data, &data_length ) ) {
    in_profile = cmsOpenProfileFromMem( data, data_length );
  }

  std::cout<<"ConvertColorspacePar::build(): image="<<in[0]<<" data="<<data<<" data_length="<<data_length<<std::endl;
   */
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
  bool out_type_changed = out_profile_type.is_modified();
  bool out_changed = out_profile_name.is_modified();
  bool out_trc_type_changed = out_trc_type.is_modified();

  bool changed = in_changed || out_mode_changed || out_type_changed || out_trc_type_changed || out_changed;

  cmsHPROFILE out_profile = NULL;
  profile_mode_t pmode = (profile_mode_t)out_profile_type.get_enum_value().first;
  profile_type_t ptype = (profile_type_t)out_profile_type.get_enum_value().first;
  TRC_type trc_type = (TRC_type)out_trc_type.get_enum_value().first;
  PF::ICCProfile* iccprof = NULL;

  if( pmode == PF::PROF_TYPE_FROM_SETTINGS ) {
    ptype = PF::PhotoFlow::Instance().get_options().get_working_profile_type();
    trc_type = PF::PhotoFlow::Instance().get_options().get_working_trc_type();
    //std::cout<<"ConvertColorspacePar::build(): Getting output profile from settings..."<<std::endl;
    iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
  } else if( pmode == PF::PROF_TYPE_FROM_DISK ) {
    //std::cout<<"ConvertColorspacePar::build(): Getting output profile from disk: "<<out_profile_name.get()<<std::endl;
    iccprof = PF::ICCStore::Instance().get_profile( out_profile_name.get() );
  } else {//if( pmode == PF::PROF_MODE_CUSTOM ) {
    ptype = (profile_type_t)out_profile_type.get_enum_value().first;
    trc_type = (TRC_type)out_trc_type.get_enum_value().first;
    //std::cout<<"ConvertColorspacePar::build(): Getting built-in profile: "<<out_profile_type.get_enum_value().second.first<<std::endl;
    iccprof = PF::ICCStore::Instance().get_profile( ptype, trc_type );
  }

  if( iccprof ) {
    out_profile = iccprof->get_profile();
  }
  //std::cout<<"ConvertColorspacePar::build(): out_mode_changed="<<out_mode_changed
  //    <<"  out_changed="<<out_changed<<"  out_profile="<<out_profile<<std::endl;
  //std::cout<<"  out_profile_mode="<<out_profile_mode.get_enum_value().first<<std::endl;

  if( assign.get() ) {
    VipsImage* out = image;
    PF_REF( out, "ConvertColorspacePar::build(): out ref for profile assignment" );
    PF::set_icc_profile( out, iccprof );
    return out;
  }


  bool matching = false;
  if( iccprof_in && iccprof && iccprof_in->equals_to(iccprof) ) {
    matching = true;
  }

  if( matching ) {
    PF_REF( in[first], "ConvertColorspacePar::build(): input image ref for equal input and output profiles" );
    //std::cout<<"ConvertColorspacePar::build(): matching input and output profiles, no transform needed"<<std::endl;
    return in[first];
  }


  /*
  if( changed ) {

    if( transform )
      cmsDeleteTransform( transform );  

    transform = NULL;
    if( !assign.get() && in_profile && out_profile ) {
      cmsUInt32Number infmt = vips2lcms_pixel_format( in[0]->BandFmt, in_profile );
      cmsUInt32Number outfmt = vips2lcms_pixel_format( in[0]->BandFmt, out_profile );

      cmsUInt32Number flags = cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE;
      if( bpc.get() ) flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;

      cmsFloat64Number old_state = cmsSetAdaptationState( adaptation_state.get() );
      std::cout<<"ConvertColorspacePar::build(): adaptation_state="<<adaptation_state.get()
          <<"  old_state="<<old_state<<std::endl;
      transform = cmsCreateTransform( in_profile, 
          infmt,
          out_profile,
          outfmt,
          intent.get_enum_value().first,//INTENT_RELATIVE_COLORIMETRIC,
          flags); //cmsFLAGS_NOOPTIMIZE | cmsFLAGS_NOCACHE );
      cmsSetAdaptationState( old_state );
    }
  }

  if( out_profile) {
    output_cs_type = cmsGetColorSpace(out_profile);
    switch( output_cs_type ) {
    case cmsSigGrayData:
      std::cout<<"ConvertColorspacePar::build(): image mode set to GRAYSCALE"<<std::endl;
      grayscale_image( get_xsize(), get_ysize() );
      break;
    case cmsSigRgbData:
      std::cout<<"ConvertColorspacePar::build(): image mode set to RGB"<<std::endl;
      rgb_image( get_xsize(), get_ysize() );
      break;
    case cmsSigXYZData:
      std::cout<<"ConvertColorspacePar::build(): image mode set to XYZ"<<std::endl;
      rgb_image( get_xsize(), get_ysize() );
      break;
    case cmsSigLabData:
      std::cout<<"ConvertColorspacePar::build(): image mode set to LAB"<<std::endl;
      lab_image( get_xsize(), get_ysize() );
      break;
    case cmsSigCmykData:
      std::cout<<"ConvertColorspacePar::build(): image mode set to CMYK"<<std::endl;
      cmyk_image( get_xsize(), get_ysize() );
      break;
    default:
      break;
    }
  }
   */
  //std::cout<<"ConvertColorspacePar::build(): transform="<<transform<<std::endl;

  //if( !in_profile && out_profile ) {
  // The input profile was not specified, so we simply assign the output
  // profile without any conversion
  //}

  /*
  if( in_profile && out_profile && !assign.get() && !transform ) {
    //if( in_profile )  cmsCloseProfile( in_profile );
    //if( out_profile ) cmsCloseProfile( out_profile );
    out_profile = NULL;
    out_profile_data = NULL;
    out_profile_data_length = 0;
    return NULL;
  }
  */

  VipsImage* out = NULL;
    PF::ICCTransformPar* tr_par =
        dynamic_cast<PF::ICCTransformPar*>( cs_transform->get_par() );
    std::vector<VipsImage*> in2;
    in2.push_back( image );
    tr_par->set_image_hints( image );
    tr_par->set_format( get_format() );
    tr_par->set_out_profile( iccprof );
    tr_par->set_intent(intent.get_enum_value().first);
    tr_par->set_bpc( bpc.get() );
    tr_par->set_adaptation_state( adaptation_state.get() );
    tr_par->set_clip_negative(clip_negative.get());
    tr_par->set_clip_overflow(clip_overflow.get());
    out = tr_par->build( in2, 0, NULL, NULL, level );
    //std::cout<<"ConvertColorspacePar::build(): tr_par output: "<<out<<std::endl;

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
    std::cout<<"ConvertColorspacePar::build(): image="<<out<<"  embedded profile: "<<tstr<<std::endl;
    if( assign.get() ) std::cout<<"    profile assigned"<<std::endl;
  }
     */
    if( iccprof ) PF::set_icc_profile( out, iccprof );

    //std::cout<<"ConvertColorspacePar::build(): gamut_warning="<<gamut_warning<<"  get_render_mode()="<<get_render_mode()<<std::endl;
    if( gamut_warning && (get_render_mode() == PF_RENDER_PREVIEW) ) {
      PF::ICCProfile* aces_prof =
          PF::ICCStore::Instance().get_profile( PF::PROF_TYPE_ACES, PF::PF_TRC_LINEAR );
      PF::ICCProfile* Lab_prof =
          PF::ICCStore::Instance().get_Lab_profile();
      PF::ICCProfile* gw_prof = Lab_prof;
      if( cmsIsMatrixShaper(out_profile) &&
          !cmsIsCLUT(out_profile, intent.get_enum_value().first, LCMS_USED_AS_OUTPUT) ) {
        gw_prof = aces_prof;
      }
      //std::cout<<"ConvertColorspacePar::build(): gw_prof="<<(void*)gw_prof<<std::endl;
      if( gw_prof && gw_prof->get_profile() ) {
        PF::ICCTransformPar* tr_in =
            dynamic_cast<PF::ICCTransformPar*>( gw_transform_in->get_par() );
        PF::ICCTransformPar* tr_out =
            dynamic_cast<PF::ICCTransformPar*>( gw_transform_out->get_par() );
        VipsImage* gw_in = NULL, *gw_out = NULL, *out2 = NULL;
        if( tr_in && tr_out ) {
          std::vector<VipsImage*> in2;
          in2.push_back( image );
          tr_in->set_image_hints( image );
          tr_in->set_format( get_format() );
          tr_in->set_out_profile( gw_prof );
          tr_in->set_bpc( false );
          gw_in = tr_in->build( in2, 0, NULL, NULL, level );

          in2.clear(); in2.push_back( out );
          tr_out->set_image_hints( out );
          tr_out->set_format( get_format() );
          tr_out->set_out_profile( gw_prof );
          tr_out->set_bpc( bpc.get() );
          gw_out = tr_out->build( in2, 0, NULL, NULL, level );
          PF_UNREF( out, "ConvertColorspacePar::build(): out unref after gamut warning transform" );

          in2.clear();
          in2.push_back( gw_in );
          in2.push_back( gw_out );
          in2.push_back( out );
          gw->get_par()->set_image_hints( out );
          gw->get_par()->set_format( get_format() );

          {
            char tstr[1024];
            cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
            std::cout<<"ConvertColorspacePar::build(): output profile="<<tstr
                <<"  cmsIsMatrixShaper(out_profile)="<<cmsIsMatrixShaper(out_profile)<<std::endl;
          }

          PF::GamutWarningPar* gw2 = dynamic_cast<PF::GamutWarningPar*>( gw->get_par() );
          if( gw2 ) {
            if( cmsIsMatrixShaper(out_profile) &&
                !cmsIsCLUT(out_profile, intent.get_enum_value().first, LCMS_USED_AS_OUTPUT) ) {
              gw2->set_dest_is_matrix( true );

            } else {
              gw2->set_delta( 4.9999 );
              gw2->set_dest_is_matrix( false );
            }
            std::cout<<"ConvertColorspacePar::build(): gw2->get_dest_is_matrix(): "<<gw2->get_dest_is_matrix()<<std::endl;
          }

          out2 = gw->get_par()->build( in2, 0, NULL, NULL, level );
          PF_UNREF( gw_in, "ConvertColorspacePar::build(): gw_in unref after gamut warning process" );
          PF_UNREF( gw_out, "ConvertColorspacePar::build(): gw_out unref after gamut warning process" );
          //PF_UNREF( out, "ConvertColorspacePar::build(): out unref after gamut warning process" );

          out = out2;
        }
      }
    }
  if( iccprof ) PF::set_icc_profile( out, iccprof );


  //if( in_profile )  cmsCloseProfile( in_profile );
  //if( out_profile ) cmsCloseProfile( out_profile );
  out_profile = NULL;
  out_profile_data = NULL;
  out_profile_data_length = 0;

  return out;
}
