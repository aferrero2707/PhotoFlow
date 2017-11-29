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

#include <cmath>

#include "../../base/exif_data.hh"
#include "../../base/iccstore.hh"
#include "../../operations/icc_transform.hh"
#include "raw_output.hh"



/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../../external/darktable/src/common/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "../../external/darktable/src/external/adobe_coeff.c"
//#include "../vips/vips_layer.h"


PF::RawOutputV1Par::RawOutputV1Par():
          OpParBase(),
          image_data( NULL ),
          exposure("exposure",this,1),
          exposure_mode("exposure_mode",this,PF::EXP_NORMAL,"NORMAL","Normal"),
          exposure_clip_amount("exposure_clip_amount",this,0),
          hlreco_mode("hlreco_mode",this,PF::HLRECO_CLIP,"HLRECO_CLIP",_("clip")),
          profile_mode("profile_mode",this,PF::IN_PROF_MATRIX,"MATRIX","MATRIX"),
          //profile_mode("profile_mode",this,PF::IN_PROF_NONE,"NONE","NONE"),
          current_profile_mode( IN_PROF_MATRIX ),
          gamma_curve( NULL ),
          cam_profile_name("cam_profile_name", this),
          cam_profile( NULL ),
          gamma_mode("gamma_mode",this,PF::IN_GAMMA_NONE,"NONE","linear"),
          gamma_lin("gamma_lin", this, 0),
          gamma_exp("gamma_exp", this, 2.2),
          out_profile_mode("out_profile_mode",this,PF::PROF_TYPE_sRGB,"sRGB","sRGB"),
          //current_out_profile_mode( PF::PROF_TYPE_sRGB ),
          out_profile_name("out_profile_name", this),
          out_profile( NULL )
          //transform( NULL )
{
  exposure_mode.add_enum_value(PF::EXP_AUTO,"AUTO","Auto");

  hlreco_mode.add_enum_value(PF::HLRECO_BLEND,"HLRECO_BLEND",_("blend"));
  hlreco_mode.add_enum_value(PF::HLRECO_NONE,"HLRECO_NONE",_("none"));

  profile_mode.add_enum_value(PF::IN_PROF_NONE,"NONE","RAW");
  profile_mode.add_enum_value(PF::IN_PROF_MATRIX,"MATRIX","MATRIX");
  profile_mode.add_enum_value(PF::IN_PROF_ICC,"ICC","ICC");

  out_profile_mode.add_enum_value(PF::PROF_TYPE_NONE,"NONE","NONE");
  //out_profile_mode.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  out_profile_mode.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Built-in Adobe RGB 1998");
  out_profile_mode.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","Built-in ProPhoto RGB");
  out_profile_mode.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  out_profile_mode.add_enum_value(PF::PROF_TYPE_FROM_DISK,"CUSTOM","Custom");

  gamma_mode.add_enum_value(PF::IN_GAMMA_NONE,"NONE","linear");
  gamma_mode.add_enum_value(PF::IN_GAMMA_sRGB,"sRGB","sRGB");
  gamma_mode.add_enum_value(PF::IN_GAMMA_CUSTOM,"CUSTOM","Custom");

  cmsToneCurve* curve = Build_sRGBGamma(NULL);
  srgb_curve = cmsReverseToneCurve( curve );
  cmsFreeToneCurve( curve );

  cs_transform = new_icc_transform();

  set_type("raw_output" );
}


void PF::RawOutputV1Par::set_image_hints( VipsImage* img )
{
  if( !img ) return;
  PF::OpParBase::set_image_hints( img );
#ifndef NDEBUG
  std::cout<<"RawOutputPar::set_image_hints(): out_profile_mode="<<out_profile_mode.get_enum_value().first<<std::endl;
#endif
  if( out_profile_mode.get_enum_value().first == PF::PROF_TYPE_LAB ) {
#ifndef NDEBUG
    std::cout<<"RawOutputPar::set_image_hints(): calling lab_image()"<<std::endl;
#endif
    lab_image( get_xsize(), get_ysize() );
  } else {
#ifndef NDEBUG
    std::cout<<"RawOutputPar::set_image_hints(): calling rgb_image()"<<std::endl;
#endif
    rgb_image( get_xsize(), get_ysize() );
  }
}


VipsImage* PF::RawOutputV1Par::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (int)in.size() < first+1 )
    return NULL;

  VipsImage* image = in[first];
  if( !image )
    return NULL;

  size_t blobsz;
  if( vips_image_get_blob( in[0], "raw_image_data",
      (void**)&image_data,
      &blobsz ) ) {
    std::cout<<"RawOutputPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"RawOutputPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }

  bool mode_changed = profile_mode.is_modified();
  bool out_mode_changed = out_profile_mode.is_modified();
  bool gamma_mode_changed = gamma_mode.is_modified();
  bool cam_changed = cam_profile_name.is_modified();
  bool out_changed = out_profile_name.is_modified();
  /*
  if( profile_mode.get_enum_value().first != (int)current_profile_mode )
    mode_changed = true;
  if( out_profile_mode.get_enum_value().first != (int)current_out_profile_mode )
    out_mode_changed = true;
  if( cam_profile_name.get() != current_cam_profile_name )
    cam_changed = true;
  if( out_profile_name.get() != current_out_profile_name )
    out_changed = true;

  current_profile_mode = (input_profile_mode_t)profile_mode.get_enum_value().first;
  current_out_profile_mode = (output_profile_mode_t)out_profile_mode.get_enum_value().first;
  current_cam_profile_name = cam_profile_name.get();
  current_out_profile_name = out_profile_name.get();
   */

  //std::cout<<"RawOutputPar::build(): mode_changed="<<mode_changed
  //         <<"  out_mode_changed="<<out_mode_changed
  //         <<"  cam_changed="<<cam_changed
  //         <<"  out_changed="<<out_changed<<std::endl;

  bool changed = mode_changed || out_mode_changed || gamma_mode_changed || cam_changed || out_changed || out_mode_changed ||
      (cam_profile==NULL) || (out_profile==NULL);

  if( cam_profile && (mode_changed || cam_changed) ) {
    //cmsCloseProfile( cam_profile );
    cam_profile = NULL;
  }

  // create input camera profile based on Adobe matrices
  if( mode_changed || (cam_profile == NULL) ) {

    switch( profile_mode.get_enum_value().first ) {
    case PF::IN_PROF_MATRIX: {
      //cam_profile = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])image_data->color.cam_xyz);
      PF::exif_data_t* exif_data;
      if( vips_image_get_blob( in[0], PF_META_EXIF_NAME,
          (void**)&exif_data,
          &blobsz ) ) {
        std::cout<<"RawOutputPar::build() could not extract exif_custom_data."<<std::endl;
        return NULL;
      }
      if( blobsz != sizeof(PF::exif_data_t) ) {
        std::cout<<"RawOutputPar::build() wrong exif_custom_data size."<<std::endl;
        return NULL;
      }
      //char makermodel[1024];
      //dt_colorspaces_get_makermodel( makermodel, sizeof(makermodel), exif_data->exif_maker, exif_data->exif_model );
      //std::cout<<"RawOutputPar::build(): makermodel="<<makermodel<<std::endl;
      /*
      float cam_xyz[12];
      cam_xyz[0] = NAN;
      std::cout<<"Getting default camera matrix for makermodel=\""<<exif_data->camera_makermodel<<"\""<<std::endl;
      dt_dcraw_adobe_coeff(exif_data->camera_makermodel, (float(*)[12])cam_xyz);
      if(std::isnan(cam_xyz[0])) {
        std::cout<<"RawOutputPar::build(): isnan(cam_xyz[0])"<<std::endl;
        PF_REF(image,"RawOutputPar::build(): isnan(cam_xyz[0])");
        return image;
      }
      //cam_profile = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])cam_xyz);
       */
      cmsHPROFILE cam_prof_temp = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])image_data->color.cam_xyz);
      cam_profile = PF::ICCStore::Instance().get_profile( cam_prof_temp );
      //cmsCloseProfile( cam_prof_temp );
      break;
    }
    case PF::IN_PROF_ICC:
      if( !cam_profile_name.get().empty() )
        cam_profile = PF::ICCStore::Instance().get_profile( cam_profile_name.get() );
      break;
    default:
      break;
    }
  } else if( (profile_mode.get_enum_value().first == PF::IN_PROF_ICC) && cam_changed ) {
    cam_profile = PF::ICCStore::Instance().get_profile( cam_profile_name.get() );
  }

  if( out_profile && (out_mode_changed || out_changed) ) {
    //cmsCloseProfile( out_profile );
    out_profile = NULL;
  }

  if( out_mode_changed || out_changed || (out_profile == NULL) ) {
    //std::cout<<"RawOutputPar::build(): out_mode_changed="<<out_mode_changed
    //         <<"  out_changed="<<out_changed<<"  out_profile="<<out_profile<<std::endl;
    //std::cout<<"  out_profile_mode="<<out_profile_mode.get_enum_value().first<<std::endl;
    profile_type_t ptype = (profile_type_t)out_profile_mode.get_enum_value().first;
    TRC_type trc_type = PF::PF_TRC_STANDARD;
    switch( out_profile_mode.get_enum_value().first ) {
    case PROF_TYPE_NONE:
      out_profile = NULL;
      //std::cout<<"RawOutputPar::build(): created sRGB output profile"<<std::endl;
      break;
    case PROF_TYPE_sRGB:
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
      //std::cout<<"RawOutputPar::build(): created sRGB output profile"<<std::endl;
      break;
    case PROF_TYPE_ADOBE:
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
      //std::cout<<"RawOutputPar::build(): created AdobeRGB output profile"<<std::endl;
      break;
    case PROF_TYPE_PROPHOTO:
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
      //std::cout<<"RawOutputPar::build(): created ProPhoto output profile"<<std::endl;
      break;
    case PROF_TYPE_LAB:
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
      //std::cout<<"RawOutputPar::build(): created Lab output profile"<<std::endl;
      break;
    case PROF_TYPE_CUSTOM:
      //std::cout<<"  custom profile selected: \""<<cam_profile_name.get()<<"\""<<std::endl;
      if( !out_profile_name.get().empty() )
        out_profile = PF::ICCStore::Instance().get_profile( out_profile_name.get() );
      break;
    default:
      break;
    }
  }

  if( changed ) {
#ifndef NDEBUG
    std::cout<<"RawOutputPar::build(): color conversion changed, rebuilding transform"<<std::endl;
    std::cout<<"  cam_profile="<<(void*)cam_profile<<"  out_profile="<<(void*)out_profile<<std::endl;
#endif
    char tstr[1024];
#ifndef NDEBUG
    if( cam_profile ) {
      cmsGetProfileInfoASCII(cam_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"  cam_profile description: "<<tstr<<std::endl;
      std::cout<<"  cam_profile colorspace: "<<cmsGetColorSpace(cam_profile->get_profile())<<std::endl;
    }
    if( out_profile ) {
      cmsGetProfileInfoASCII(out_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"  out_profile description: "<<tstr<<std::endl;
      std::cout<<"  out_profile colorspace: "<<cmsGetColorSpace(out_profile->get_profile())<<std::endl;
    }
#endif
    /*if( transform ) {
      cmsDeleteTransform( transform );  
    }
    transform = NULL;
    if( cam_profile && out_profile ) {
      cmsUInt32Number out_lcms_type = TYPE_RGB_FLT;
      if( out_profile_mode.get_enum_value().first == PF::PROF_TYPE_LAB ) {
        out_lcms_type = TYPE_Lab_FLT;
      }
      transform = cmsCreateTransform( cam_profile->get_profile(), 
          //TYPE_YCbCr_8,//(FLOAT_SH(1)|COLORSPACE_SH(PT_YCbCr)|CHANNELS_SH(3)|BYTES_SH(4)),
          TYPE_RGB_FLT,
          out_profile->get_profile(),
          out_lcms_type,
          INTENT_RELATIVE_COLORIMETRIC,
          cmsFLAGS_NOCACHE | cmsFLAGS_NOOPTIMIZE );
#ifndef NDEBUG
      std::cout<<"RawOutputPar::build(): new transform="<<transform<<std::endl;
#endif
    }*/
  }
#ifndef NDEBUG
  std::cout<<"RawOutputPar::build(): transform="<<transform<<std::endl;
#endif

  if( gamma_curve )
    cmsFreeToneCurve( gamma_curve );
  float gamma = gamma_exp.get();
  gamma_curve = cmsBuildGamma( NULL, 1.0f/gamma );

  VipsImage* rotated = image;
  switch( image_data->sizes.flip ) {
  case 6:
    if( vips_rot(image, &rotated, VIPS_ANGLE_D90, NULL) )
      return NULL;
    break;
  case 3:
    if( vips_rot(image, &rotated, VIPS_ANGLE_D180, NULL) )
      return NULL;
    break;
  case 5:
    if( vips_rot(image, &rotated, VIPS_ANGLE_D270, NULL) )
      return NULL;
    break;
  default: 
    PF_REF( rotated, "RawOutputPar::build(): rotated ref" );
    break;
  }
  if( !rotated ) return NULL;
  set_image_hints( rotated );

  std::vector<VipsImage*> in2;
  in2.push_back( rotated );

  VipsImage* out = OpParBase::build( in2, first, NULL, NULL, level );
  if( out ) {
    PF_UNREF( rotated, "RawOutputPar::build(): rotated unref" );
  }
  /**/
  if( out_profile ) {
    if( cam_profile && out_profile && out_profile->get_profile() != cam_profile->get_profile() ) {
      PF::set_icc_profile( out, cam_profile );
      PF::ICCTransformPar* tr_par =
          dynamic_cast<PF::ICCTransformPar*>( cs_transform->get_par() );
      std::vector<VipsImage*> in2;
      in2.push_back( out );
      tr_par->set_image_hints( out );
      tr_par->set_format( get_format() );
      tr_par->set_out_profile( out_profile );
      tr_par->set_bpc( false );
      tr_par->set_adaptation_state( 0.f );
      tr_par->set_clip_negative(true);
      tr_par->set_clip_overflow(true);
      VipsImage* out2 = tr_par->build( in2, 0, NULL, NULL, level );
      if( out2 ) {
        PF_UNREF( out, "RawOutputPar::build(): out unref" );
      }
      out = out2;
    }
    /*
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( out_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( out_profile, buf, &out_length);
    vips_image_set_blob( out, VIPS_META_ICC_NAME, 
			 (VipsCallbackFn) g_free, buf, out_length );
    //std::cout<<"RawOutputPar::build(): icc profile metadata saved, image="<<out<<" data="<<buf<<" data_length="<<out_length<<std::endl;

    PF::ICCProfileData* iccdata = PF::ICCStore::Instance().get_profile( ptype, trc_type )->get_data();
    vips_image_set_blob( out, "pf-icc-profile-data",
       (VipsCallbackFn) PF::free_icc_profile_data, iccdata, sizeof(PF::ICCProfileData) );
    //char tstr[1024];
    //cmsGetProfileInfoASCII(out_profile, cmsInfoDescription, "en", "US", tstr, 1024);
    //std::cout<<"RawOutputPar::build(): image="<<out<<"  embedded profile: "<<tstr<<std::endl;
     */
    std::cout<<"RawOutputPar::build(): PF::set_icc_profile( out, out_profile ) called"<<std::endl;
    if( out_profile->get_profile() ) {
      char tstr[1024];
      cmsGetProfileInfoASCII(out_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"RawOutputPar::build(): output profile: "<<tstr<<std::endl;
      //cmsCloseProfile( profile_in );
    }
    PF::set_icc_profile( out, out_profile );
  } else if( cam_profile ) {
    /*
    cmsUInt32Number out_length;
    cmsSaveProfileToMem( cam_profile, NULL, &out_length);
    void* buf = malloc( out_length );
    cmsSaveProfileToMem( cam_profile, buf, &out_length);
    vips_image_set_blob( out, VIPS_META_ICC_NAME,
        (VipsCallbackFn) g_free, buf, out_length );

    ICCProfileData* iccdata = new ICCProfileData;
    iccdata->trc_type = PF::PF_TRC_LINEAR;
    memset( iccdata->perceptual_trc_vec, 0, sizeof(int)*65536 );
    memset( iccdata->perceptual_trc_inv_vec, 0, sizeof(int)*65536 );
    iccdata->perceptual_trc =  cmsBuildGamma (NULL, 1.00);
    iccdata->perceptual_trc_inv =  cmsBuildGamma (NULL, 1.00);
    iccdata->Y_R = 1;
    iccdata->Y_G = 1;
    iccdata->Y_B = 1;
    vips_image_set_blob( out, "pf-icc-profile-data",
       (VipsCallbackFn) PF::free_icc_profile_data, iccdata, sizeof(PF::ICCProfileData) );
     */
    std::cout<<"RawOutputPar::build(): PF::set_icc_profile( out, cam_profile ) called"<<std::endl;
    if( cam_profile ) {
      char tstr[1024];
      cmsGetProfileInfoASCII(cam_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"RawOutputPar::build(): output profile (cam): "<<tstr<<std::endl;
      //cmsCloseProfile( profile_in );
    }
    PF::set_icc_profile( out, cam_profile );
  }
  /**/
  return out;
}
