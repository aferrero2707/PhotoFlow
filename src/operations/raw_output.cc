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

#include "../base/exif_data.hh"
#include "../base/iccstore.hh"
#include "raw_output.hh"



/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../dt/common/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "../dt/external/adobe_coeff.c"
//#include "../vips/vips_layer.h"


PF::RawOutputPar::RawOutputPar(): 
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
      cam_dcp_profile_name("cam_dcp_profile_name", this),
      cam_profile( NULL ),
      apply_hue_sat_map( "apply_hue_sat_map", this, true ),
      apply_look_table( "apply_look_table", this, true ),
      use_tone_curve( "use_tone_curve", this, true ),
      apply_baseline_exposure_offset( "apply_baseline_exposure_offset", this, true ),
      gamma_mode("gamma_mode",this,PF::IN_GAMMA_NONE,"NONE","linear"),
      gamma_lin("gamma_lin", this, 0),
      gamma_exp("gamma_exp", this, 2.2),
      //out_profile_mode("out_profile_mode",this,PF::PROF_TYPE_sRGB,"sRGB","Built-in sRGB"),
      out_profile_mode("out_profile_mode2",this,PF::PROF_MODE_DEFAULT,"DEFAULT",_("default")),
      out_profile_type("out_profile_mode",this,PF::PROF_TYPE_REC2020,"REC2020","Rec.2020"),
      //out_profile_type("out_profile_mode",this,PF::PROF_TYPE_sRGB,"sRGB","sRGB"),
      out_trc_type("out_trc_type",this,PF::PF_TRC_LINEAR,"TRC_LINEAR","linear"),
      //out_trc_type("out_trc_type",this,PF::PF_TRC_STANDARD,"TRC_STANDARD",_("standard")),
      current_out_profile_type( PROF_TYPE_sRGB ),
      current_out_trc_type( PF::PF_TRC_STANDARD ),
      out_profile_name("out_profile_name", this),
      out_profile( NULL ),
      transform( NULL ),
      clip_negative("clip_negative",this,true),
      clip_overflow("clip_overflow",this,true)
{
  exposure_mode.add_enum_value(PF::EXP_AUTO,"AUTO","Auto");

  hlreco_mode.add_enum_value(PF::HLRECO_BLEND,"HLRECO_BLEND",_("blend"));
  hlreco_mode.add_enum_value(PF::HLRECO_NONE,"HLRECO_NONE",_("none"));

  profile_mode.add_enum_value(PF::IN_PROF_NONE,"NONE","Raw color");
  profile_mode.add_enum_value(PF::IN_PROF_MATRIX,"MATRIX","Standard profile");
  //profile_mode.add_enum_value(PF::IN_PROF_MATRIX,"MATRIX","Standard camera profile");
  profile_mode.add_enum_value(PF::IN_PROF_ICC,"ICC","ICC from disk");
  profile_mode.add_enum_value(PF::IN_PROF_DCP,"DCP","DCP from disk");

  out_profile_type.add_enum_value(PF::PROF_TYPE_EMBEDDED,"EMBEDDED",_("use camera profile"));
  out_profile_type.add_enum_value(PF::PROF_TYPE_sRGB,"sRGB","sRGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_REC2020,"REC2020","Rec.2020");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ADOBE,"ADOBE","Adobe RGB 1998");
  out_profile_type.add_enum_value(PF::PROF_TYPE_PROPHOTO,"PROPHOTO","ProPhoto RGB");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACEScg,"ACEScg","ACEScg");
  out_profile_type.add_enum_value(PF::PROF_TYPE_ACES,"ACES","ACES");
  //out_profile_type.add_enum_value(PF::PROF_TYPE_CUSTOM,"CUSTOM","Custom");
  out_profile_type.add_enum_value(PF::PROF_TYPE_LAB,"LAB","Lab");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_SETTINGS,"FROM_SETTINGS","from settings");
  out_profile_type.add_enum_value(PF::PROF_TYPE_FROM_DISK,"FROM_DISK","open from disk");

  out_profile_mode.add_enum_value(PF::PROF_MODE_EMBEDDED,"EMBEDDED",_("use camera profile"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_CUSTOM,"CUSTOM",_("custom"));
  out_profile_mode.add_enum_value(PF::PROF_MODE_ICC,"ICC",_("ICC from disk"));

  out_trc_type.add_enum_value(PF::PF_TRC_LINEAR,"TRC_LINEAR","linear");
  out_trc_type.add_enum_value(PF::PF_TRC_PERCEPTUAL,"TRC_PERCEPTUAL","perceptual");
  out_trc_type.add_enum_value(PF::PF_TRC_STANDARD,"TRC_STANDARD","standard");

  gamma_mode.add_enum_value(PF::IN_GAMMA_NONE,"NONE","linear");
  gamma_mode.add_enum_value(PF::IN_GAMMA_sRGB,"sRGB","sRGB");
  gamma_mode.add_enum_value(PF::IN_GAMMA_CUSTOM,"CUSTOM","Custom");

  cmsToneCurve* curve = Build_sRGBGamma(NULL);
  srgb_curve = cmsReverseToneCurve( curve );
  cmsFreeToneCurve( curve );

  set_type("raw_output_v2" );
}

/*
void PF::RawOutputPar::set_image_hints( VipsImage* img )
{
  if( !img ) return;
  PF::OpParBase::set_image_hints( img );
  std::cout<<"RawOutputPar::set_image_hints(): out_profile_mode="<<out_profile_mode.get_enum_value().first<<std::endl;
  if( out_profile_mode.get_enum_value().first == PF::PROF_TYPE_LAB ) {
    std::cout<<"RawOutputPar::set_image_hints(): calling lab_image()"<<std::endl;
    lab_image( get_xsize(), get_ysize() );
  } else {
    std::cout<<"RawOutputPar::set_image_hints(): calling rgb_image()"<<std::endl;
    rgb_image( get_xsize(), get_ysize() );
  }
}
 */

VipsImage* PF::RawOutputPar::build(std::vector<VipsImage*>& in, int first, 
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

  if( exposure_mode.get_enum_value().first == PF::EXP_NORMAL )
    exposure_current = exposure.get();

  bool mode_changed = profile_mode.is_modified();
  bool out_mode_changed = out_profile_mode.is_modified();
  bool out_type_changed = out_profile_type.is_modified();
  bool out_trc_type_changed = out_trc_type.is_modified();
  bool gamma_mode_changed = gamma_mode.is_modified();
  bool cam_changed = cam_profile_name.is_modified();
  bool cam_dcp_changed = cam_dcp_profile_name.is_modified();
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

  bool changed = mode_changed || out_mode_changed || out_type_changed || out_trc_type_changed || gamma_mode_changed || cam_changed || cam_dcp_changed || out_changed || out_mode_changed ||
      (cam_profile==NULL) || (out_profile==NULL);

  if( cam_profile && (mode_changed || cam_changed || cam_dcp_changed) ) {
    //cmsCloseProfile( cam_profile );
    cam_profile = NULL;
  }

  // create input camera profile based on Adobe matrices
  if( mode_changed || (cam_profile == NULL) ) {

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
    dt_dcraw_adobe_coeff(exif_data->camera_makermodel, (float(*)[12])cam_xyz);
    if(std::isnan(cam_xyz[0])) {
      std::cout<<"RawOutputPar::build(): isnan(cam_xyz[0])"<<std::endl;
      PF_REF(image,"RawOutputPar::build(): isnan(cam_xyz[0])");
      return image;
    }
    */
    double dcam_xyz[3][3];
    for(int i = 0; i < 3; i++)
      for(int j = 0; j < 3; j++)
        dcam_xyz[i][j] = image_data->color.cam_xyz[i][j];

    std::cout<<"dcam_xyz:"<<std::endl;
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        std::cout<<dcam_xyz[i][j]<<" ";
      }
      std::cout<<std::endl;
    }

    switch( profile_mode.get_enum_value().first ) {
    case PF::IN_PROF_MATRIX: {
      //cam_profile = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])image_data->color.cam_xyz);
      /*
      cmsHPROFILE cam_prof_temp = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])cam_xyz);
      cam_profile = PF::ICCStore::Instance().get_profile( cam_prof_temp );
      //cmsCloseProfile( cam_prof_temp );
      */
      cmsHPROFILE cam_prof_temp = dt_colorspaces_create_xyzimatrix_profile((float (*)[3])image_data->color.cam_xyz);
      cam_profile = PF::ICCStore::Instance().get_profile( cam_prof_temp );
      std::cout<<"profile colorants:"<<std::endl;
      for(int i = 0, ci = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++, ci++) {
          std::cout<<cam_profile->get_colorants()[ci]<<" ";
        }
        std::cout<<std::endl;
      }
      break;
    }
    case PF::IN_PROF_ICC:
      if( !cam_profile_name.get().empty() )
        cam_profile = PF::ICCStore::Instance().get_profile( cam_profile_name.get() );
      break;
    case PF::IN_PROF_DCP:
      rtengine::Color::init();
      if( !cam_dcp_profile_name.get().empty() ) {
        std::cout<<"dcam_xyz:"<<std::endl;
        for(int i = 0; i < 3; i++) {
          for(int j = 0; j < 3; j++) {
            std::cout<<dcam_xyz[i][j]<<" ";
          }
          std::cout<<std::endl;
        }

        int preferred_illuminant = 0;
        cam_dcp_profile = new rtengine::DCPProfile( cam_dcp_profile_name.get() );
        double cam_wb[3] = {get_wb_red(), get_wb_green(), get_wb_blue()};
        //double mXYZCAM[3][3];
        //cam_dcp_profile->makeXyzCam( cam_wb, dcam_xyz, 0, mXYZCAM );
        rtengine::DCPProfile::Matrix mXYZCAM;
        mXYZCAM = cam_dcp_profile->makeXyzCam( cam_wb, dcam_xyz, preferred_illuminant );
        std::cout<<"mXYZCAM:"<<std::endl;
        for(int i = 0; i < 3; i++) {
          for(int j = 0; j < 3; j++) {
            std::cout<<mXYZCAM[i][j]<<" ";
          }
          std::cout<<std::endl;
        }
        float xyz_cam_dcp[3][3];
        for(int i = 0; i < 3; i++)
          for(int j = 0; j < 3; j++)
            xyz_cam_dcp[i][j] = mXYZCAM[i][j];

        delta_base = cam_dcp_profile->makeHueSatMap( cam_wb, preferred_illuminant );
        delta_info = cam_dcp_profile->get_delta_info();

        look_table = cam_dcp_profile->get_look_table();
        look_info = cam_dcp_profile->get_look_info();

        dcp_exp_scale = 1.f;
        if( get_apply_baseline_exposure_offset() ) {
          dcp_exp_scale = powf(2, cam_dcp_profile->baseline_exposure_offset);
        }
        if( get_apply_hue_sat_map() || get_apply_look_table() || get_use_tone_curve() ) {
          // LUT available --> Calculate matrix for conversion raw>ProPhoto
          for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
              prophoto_cam[i][j] = 0;
              for (int k = 0; k < 3; ++k) {
                prophoto_cam[i][j] += rtengine::prophoto_xyz[i][k] * xyz_cam_dcp[k][j];
              }
            }
          }
          std::cout<<"pro_photo:"<<std::endl;
          for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
              std::cout<<prophoto_cam[i][j]<<" ";
            }
            std::cout<<std::endl;
          }
          //cmsHPROFILE cam_prof_temp = dt_colorspaces_create_xyzmatrix_profile((float (*)[3])rtengine::xyz_prophoto);
          //std::cout<<"cam_prof_temp(prophoto): "<<cam_prof_temp<<std::endl;
          cam_profile = PF::ICCStore::Instance().get_profile( PF::PROF_TYPE_PROPHOTO, PF::PF_TRC_LINEAR );
          std::cout<<"cam_profile(prophoto): "<<cam_profile<<std::endl;
        } else {
          cmsHPROFILE cam_prof_temp = dt_colorspaces_create_xyzmatrix_profile((float (*)[3])xyz_cam_dcp);
          cam_profile = PF::ICCStore::Instance().get_profile( cam_prof_temp );
        }
        std::cout<<"RawOutputPar::build(): DCP cam_profile="<<cam_profile;
        if( cam_profile )
          std::cout<<"  cam_profile->get_profile()="<<cam_profile->get_profile();
        std::cout<<std::endl;
      }
      break;
    default:
      break;
    }
  } else if( (profile_mode.get_enum_value().first == PF::IN_PROF_ICC) && cam_changed ) {
    cam_profile = PF::ICCStore::Instance().get_profile( cam_profile_name.get() );
  }

  if( out_profile && (out_mode_changed || out_type_changed || out_trc_type_changed || out_changed) ) {
    //cmsCloseProfile( out_profile );
    out_profile = NULL;
  }

  if( transform )
    cmsDeleteTransform( transform );
  transform = NULL;

  profile_type_t ptype;
  profile_mode_t pmode;
  TRC_type trc_type;
  if( (profile_mode_t)profile_mode.get_enum_value().first != PF::PROF_MODE_NONE ) {
    // only retrieve the working profile if the image is color managed
    pmode = (profile_mode_t)out_profile_type.get_enum_value().first;
    if( pmode == PF::PROF_TYPE_EMBEDDED ) {
      // do nothing
      std::cout<<"Using embedded profile"<<std::endl;
      out_profile = cam_profile;
    } else if( pmode == PF::PROF_TYPE_FROM_SETTINGS && cam_profile ) {
      ptype = PF::PhotoFlow::Instance().get_options().get_working_profile_type();
      trc_type = PF::PhotoFlow::Instance().get_options().get_working_trc_type();
      std::cout<<"Getting output profile..."<<std::endl;
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    } else if( pmode == PF::PROF_TYPE_FROM_DISK && cam_profile ) {
      out_profile = PF::ICCStore::Instance().get_profile( out_profile_name.get() );
    } else { //if( pmode == PF::PROF_MODE_CUSTOM && cam_profile ) {
      ptype = (profile_type_t)out_profile_type.get_enum_value().first;
      trc_type = (TRC_type)out_trc_type.get_enum_value().first;
      std::cout<<"Getting output profile..."<<std::endl;
      out_profile = PF::ICCStore::Instance().get_profile( ptype, trc_type );
    }

    if( cam_profile && out_profile && out_profile->get_profile() != cam_profile->get_profile() ) {
      bool supported = cmsIsIntentSupported(cam_profile->get_profile(),
          INTENT_RELATIVE_COLORIMETRIC,LCMS_USED_AS_INPUT);
      std::cout<<"Relative calorimetric supported by input profile: "<<supported<<std::endl;
      transform = cmsCreateTransform( cam_profile->get_profile(),
          TYPE_RGB_FLT,
          out_profile->get_profile(),
          TYPE_RGB_FLT,
          INTENT_RELATIVE_COLORIMETRIC,
          cmsFLAGS_NOCACHE | cmsFLAGS_NOOPTIMIZE );
    }
  }
  std::cout<<"RawOutputPar::build(): transform="<<transform<<std::endl;

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
    if( out_profile ) {
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



PF::ProcessorBase* PF::new_raw_output()
{
  return new PF::Processor<PF::RawOutputPar,PF::RawOutput>();
}
