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

#include <string>
#include "photoflow.hh"
#include "iccstore.hh"


cmsToneCurve* PF::Lstar_trc;
cmsToneCurve* PF::iLstar_trc;

static cmsCIExyYTRIPLE rec2020_primaries = {
    {0.7079, 0.2920, 1.0},
    {0.1702, 0.7965, 1.0},
    {0.1314, 0.0459, 1.0}
};

static cmsCIExyYTRIPLE rec2020_primaries_prequantized = {
    {0.708012540607, 0.291993664388, 1.0},
    {0.169991652439, 0.797007778423, 1.0},
    {0.130997824007, 0.045996550894, 1.0}
};

static cmsCIExyYTRIPLE aces_cg_primaries =
{
    {0.713, 0.293,  1.0},
    {0.165, 0.830,  1.0},
    {0.128, 0.044,  1.0}
};


/* ************************** WHITE POINTS ************************** */

/* D50 WHITE POINTS */

static cmsCIExyY d50_romm_spec= {0.3457, 0.3585, 1.0};
/* http://photo-lovers.org/pdf/color/romm.pdf */

static cmsCIExyY d50_illuminant_specs = {0.345702915, 0.358538597, 1.0};
/* calculated from D50 illuminant XYZ values in ICC specs */


/* D65 WHITE POINTS */

static cmsCIExyY  d65_srgb_adobe_specs = {0.3127, 0.3290, 1.0};
/* White point from the sRGB.icm and AdobeRGB1998 profile specs:
 * http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
 * 4.2.1 Reference Display White Point
 * The chromaticity coordinates of white displayed on
 * the reference color monitor shall be x=0.3127, y=0.3290.
 * . . . [which] correspond to CIE Standard Illuminant D65.
 *
 * Wikipedia gives this same white point for SMPTE-C.
 * This white point is also given in the sRGB color space specs.
 * It's probably correct for most or all of the standard D65 profiles.
 *
 * The D65 white point values used in the LCMS virtual sRGB profile
 * is slightly different than the D65 white point values given in the
 * sRGB color space specs, so the LCMS virtual sRGB profile
 * doesn't match sRGB profiles made using the values given in the
 * sRGB color space specs.
 *
 * */


/* Various C and E WHITE POINTS */

static cmsCIExyY c_astm  = {0.310060511, 0.316149551, 1.0};
/* see http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html */
static cmsCIExyY e_astm  = {0.333333333, 0.333333333, 1.0};
/* see http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html */

static cmsCIExyY c_cie= {0.310, 0.316};
/* https://en.wikipedia.org/wiki/NTSC#Colorimetry */
static cmsCIExyY e_cie= {0.333, 0.333};

static cmsCIExyY c_6774_robertson= {0.308548930, 0.324928102, 1.0};
/* see http://en.wikipedia.org/wiki/Standard_illuminant#White_points_of_standard_illuminants
 * also see  http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html for the equations */
static cmsCIExyY e_5454_robertson= {0.333608970, 0.348572909, 1.0};
/* see http://en.wikipedia.org/wiki/Standard_illuminant#White_points_of_standard_illuminants
 * also see http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html for the equations */


/* ACES white point, taken from
 * Specification S-2014-004
 * ACEScg – A Working Space for CGI Render and Compositing
 */
static cmsCIExyY d60_aces= {0.32168, 0.33767, 1.0};



PF::ICCProfile::ICCProfile()
{
  profile_data = NULL;
  profile_size = 0;
  trc_type = PF_TRC_STANDARD;
  perceptual_trc = NULL;
  perceptual_trc_inv = NULL;
}


PF::ICCProfile::~ICCProfile()
{

}


void PF::ICCProfile::set_profile( cmsHPROFILE p )
{
  profile = p;
  cmsSaveProfileToMem( profile, NULL, &profile_size);
  profile_data = malloc( profile_size );
  cmsSaveProfileToMem( profile, profile_data, &profile_size);

  char tstr[1024];
  cmsGetProfileInfoASCII(profile, cmsInfoDescription, "en", "US", tstr, 1024);
  std::cout<<"ICCProfile::set_profile(): data="<<profile_data<<" size="<<profile_size<<"  name="<<tstr<<std::endl;

  init_colorants();
  init_trc();
}


void PF::ICCProfile::init_colorants()
{
  if( !profile ) return;
  /* get the profile colorant information and fill in colorants */
  cmsCIEXYZ *red            = (cmsCIEXYZ*)cmsReadTag(profile, cmsSigRedColorantTag);
  cmsCIEXYZ  red_colorant   = *red;
  cmsCIEXYZ *green          = (cmsCIEXYZ*)cmsReadTag(profile, cmsSigGreenColorantTag);
  cmsCIEXYZ  green_colorant = *green;
  cmsCIEXYZ *blue           = (cmsCIEXYZ*)cmsReadTag(profile, cmsSigBlueColorantTag);
  cmsCIEXYZ  blue_colorant  = *blue;

  /* Get the Red channel XYZ values */
  colorants[0]=red_colorant.X;
  colorants[1]=red_colorant.Y;
  colorants[2]=red_colorant.Z;

  /* Get the Green channel XYZ values */
  colorants[3]=green_colorant.X;
  colorants[4]=green_colorant.Y;
  colorants[5]=green_colorant.Z;

  /* Get the Blue channel XYZ values */
  colorants[6]=blue_colorant.X;
  colorants[7]=blue_colorant.Y;
  colorants[8]=blue_colorant.Z;

  //for( int i = 0; i < 9; i++ ) std::cout<<"colorants["<<i<<"]="<<colorants[i]<<std::endl;
  //getchar();

  Y_R = colorants[1];
  Y_G = colorants[4];
  Y_B = colorants[7];

/*
  std::string xyzprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/XYZ-D50-Identity-elle-V4.icc";
  cmsHPROFILE xyzprof = cmsOpenProfileFromFile( xyzprofname.c_str(), "r" );
  cmsHTRANSFORM transform = cmsCreateTransform( profile,
      TYPE_RGB_FLT,
      xyzprof,
      TYPE_RGB_FLT,
      INTENT_RELATIVE_COLORIMETRIC,
      cmsFLAGS_NOCACHE );

  float RGB[3], XYZ[3];
  RGB[0] = 1; RGB[1] = 0; RGB[2] = 0;
  cmsDoTransform(transform, RGB, XYZ, 1);
  Y_R = XYZ[1];
  RGB[0] = 0; RGB[1] = 1; RGB[2] = 0;
  cmsDoTransform(transform, RGB, XYZ, 1);
  Y_G = XYZ[1];
  RGB[0] = 0; RGB[1] = 0; RGB[2] = 1;
  cmsDoTransform(transform, RGB, XYZ, 1);
  Y_B = XYZ[1];
*/
}


void PF::ICCProfile::init_trc()
{
  if( !profile ) return;
  /* get the profile colorant information and fill in colorants */
  cmsToneCurve *red_trc   = (cmsToneCurve*)cmsReadTag(profile, cmsSigRedTRCTag);
  //cmsToneCurve *green_trc = (cmsToneCurve*)cmsReadTag(profile, cmsSigGreenTRCTag);
  //cmsToneCurve *blue_trc  = (cmsToneCurve*)cmsReadTag(profile, cmsSigBlueTRCTag);

  if( !red_trc ) return;

  cmsBool is_linear = cmsIsToneCurveLinear(red_trc);
  cmsInt32Number tcpt = cmsGetToneCurveParametricType(red_trc);
  bool is_parametric = (tcpt>0) ? true : false;

  std::cout<<"ICCProfile::init_trc(): is_linear="<<is_linear<<"  is_parametric="<<is_parametric<<std::endl;

  if( is_linear ) {
    /* LAB "L" (perceptually uniform) TRC */
    cmsFloat64Number labl_parameters[5] =
    { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
    cmsToneCurve *labl_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    cmsToneCurve *labl_parametic_curve_inv = cmsReverseToneCurve( labl_parametic_curve );
    init_trc( labl_parametic_curve, labl_parametic_curve_inv );
    set_trc_type( PF::PF_TRC_LINEAR );
  } else {
    cmsToneCurve* red_trc_inv = cmsReverseToneCurve( red_trc );
    init_trc( red_trc, red_trc_inv );
    set_trc_type( PF::PF_TRC_PERCEPTUAL );
  }
}


void PF::ICCProfile::init_trc( cmsToneCurve* trc, cmsToneCurve* trc_inv )
{
  perceptual_trc = trc;
  perceptual_trc_inv = trc_inv;

  for( int i = 0; i < 65536; i++ ) {
    cmsFloat32Number in = i, out;
    in /= 65535;
    out = cmsEvalToneCurveFloat( perceptual_trc, in );
    if( out > 1 ) out = 1;
    if( out < 0 ) out = 0;
    perceptual_trc_vec[i] = out;
    out = cmsEvalToneCurveFloat( perceptual_trc_inv, in );
    if( out > 1 ) out = 1;
    if( out < 0 ) out = 0;
    perceptual_trc_inv_vec[i] = out;
  }
}


cmsHPROFILE PF::ICCProfile::get_profile()
{
  return profile;

  cmsHPROFILE result;
  cmsUInt32Number out_length;
  cmsSaveProfileToMem( profile, NULL, &out_length);
  void* buf = malloc( out_length );
  cmsSaveProfileToMem( profile, buf, &out_length);

  result = cmsOpenProfileFromMem( buf, out_length );
  std::cout<<"ICCProfile::get_profile(): buf="<<buf<<std::endl;
  //free( buf );
  return result;
}


float PF::ICCProfile::get_lightness( float R, float G, float B )
{
  if( is_linear() ) {
    return( Y_R*R + Y_G*G + Y_B*B );
  } else {
    float lR = perceptual2linear(R);
    float lG = perceptual2linear(G);
    float lB = perceptual2linear(B);
    float L = Y_R*lR + Y_G*lG + Y_B*lB;
    return linear2perceptual(L);
  }
}

void PF::ICCProfile::get_lightness( float* RGBv, float* Lv, size_t size )
{
  if( is_linear() ) {
    for( size_t i = 0, pos = 0; i < size; i++, pos += 3 ) {
      Lv[i] = Y_R * RGBv[pos] + Y_G * RGBv[pos+1] + Y_B * RGBv[pos+2];
    }
  } else {
    float lR, lG, lB, L;
    for( size_t i = 0, pos = 0; i < size; i++, pos += 3 ) {
      lR = perceptual2linear(RGBv[pos]);
      lG = perceptual2linear(RGBv[pos+1]);
      lB = perceptual2linear(RGBv[pos+2]);
      L = Y_R*lR + Y_G*lG + Y_B*lB;
      Lv[i] = linear2perceptual(L);
    }
  }
}


/*
PF::ICCProfileData* PF::get_icc_profile_data( VipsImage* img )
{
  if( !img ) return NULL;
  PF::ICCProfileData* data;
  size_t data_length;
  if( vips_image_get_blob( img, "pf-icc-profile-data",
          (void**)(&data), &data_length ) ) {
    std::cout<<"get_icc_profile_data(): cannot find ICC profile data"<<std::endl;
    data = NULL;
  }
  if( data_length != sizeof(PF::ICCProfileData) ) {
    std::cout<<"get_icc_profile_data(): wrong size of ICC profile data"<<std::endl;
    data = NULL;
  }
  return data;
}


void PF::free_icc_profile_data( ICCProfileData* data )
{
  cmsFreeToneCurve( data->perceptual_trc );
  cmsFreeToneCurve( data->perceptual_trc_inv );

  delete data;
}


cmsFloat32Number PF::linear2perceptual( ICCProfileData* data, cmsFloat32Number val )
{
  return cmsEvalToneCurveFloat( data->perceptual_trc_inv, val );
}


cmsFloat32Number PF::perceptual2linear( ICCProfileData* data, cmsFloat32Number val )
{
  return cmsEvalToneCurveFloat( data->perceptual_trc, val );
}
*/


struct ICCProfileContainer
{
  PF::ICCProfile* profile;
};


void PF::set_icc_profile( VipsImage* img, PF::ICCProfile* prof )
{
  if( !prof ) return;
  ICCProfileContainer* pc = (ICCProfileContainer*)g_malloc( sizeof(ICCProfileContainer) );
  if( pc ) {
    pc->profile = prof;
    vips_image_set_blob( img, "pf-icc-profile",
        (VipsCallbackFn)g_free, pc, sizeof(ICCProfileContainer) );

    void* buf = g_malloc( prof->get_profile_size() );
    if( buf ) {
      memcpy( buf, prof->get_profile_data(), prof->get_profile_size() );
      vips_image_set_blob( img, VIPS_META_ICC_NAME,
          (VipsCallbackFn) g_free, buf, prof->get_profile_size() );
      std::cout<<"PF::set_icc_profile(): VIPS_META_ICC blob set (size="<<prof->get_profile_size()<<")"<<std::endl;
    } else {
      std::cerr<<"PF::set_icc_profile(): cannot allocate "<<prof->get_profile_size()<<" bytes"<<std::endl;
    }
  } else {
    std::cerr<<"PF::set_icc_profile(): cannot allocate "<<prof->get_profile_size()<<" bytes"<<std::endl;
  }
}



PF::ICCProfile* PF::get_icc_profile( VipsImage* img )
{
  if( !img ) return NULL;
  ICCProfileContainer* pc;
  size_t data_size;
  if( vips_image_get_blob( img, "pf-icc-profile",
          (void**)(&pc), &data_size ) ) {
    std::cout<<"get_icc_profile_data(): cannot find ICC profile data"<<std::endl;
    return( NULL );
  }
  if( data_size != sizeof(ICCProfileContainer) ) {
    std::cout<<"get_icc_profile(): wrong size of ICC profile data"<<std::endl;
    return( NULL );
  }
  if( !pc ) return NULL;
  return(pc->profile);
}


void PF::iccprofile_unref( void* prof )
{
  PF::ICCProfile* iccprof = (PF::ICCProfile*)( prof );
  if( !iccprof ) return;
  if( iccprof->get_ref_count() < 1 ) {
    std::cerr<<"WARNING!!! PF::iccprofile_unref("<<prof<<"): wrong refcount: "<<iccprof->get_ref_count()<<std::endl;
    return;
  }

  iccprof->unref();
  if( iccprof->get_ref_count() == 0 ) {
    std::cout<<"PF::iccprofile_unref("<<prof<<"): deleting profile"<<std::endl;
    delete iccprof;
  }
}




PF::ICCStore::ICCStore()
{
  srgb_profiles[0] = new sRGBProfile( PF::PF_TRC_STANDARD );
  srgb_profiles[1] = new sRGBProfile( PF::PF_TRC_PERCEPTUAL );
  srgb_profiles[2] = new sRGBProfile( PF::PF_TRC_LINEAR );
  srgb_profiles[0]->ref(); profiles.push_back( srgb_profiles[0] );
  srgb_profiles[1]->ref(); profiles.push_back( srgb_profiles[1] );
  srgb_profiles[2]->ref(); profiles.push_back( srgb_profiles[2] );

  rec2020_profiles[0] = new Rec2020Profile( PF::PF_TRC_STANDARD );
  rec2020_profiles[1] = new Rec2020Profile( PF::PF_TRC_PERCEPTUAL );
  rec2020_profiles[2] = new Rec2020Profile( PF::PF_TRC_LINEAR );
  rec2020_profiles[0]->ref(); profiles.push_back( rec2020_profiles[0] );
  rec2020_profiles[1]->ref(); profiles.push_back( rec2020_profiles[1] );
  rec2020_profiles[2]->ref(); profiles.push_back( rec2020_profiles[2] );

  aces_profiles[0] = new ACESProfile( PF::PF_TRC_STANDARD );
  aces_profiles[1] = new ACESProfile( PF::PF_TRC_PERCEPTUAL );
  aces_profiles[2] = new ACESProfile( PF::PF_TRC_LINEAR );
  aces_profiles[0]->ref(); profiles.push_back( aces_profiles[0] );
  aces_profiles[1]->ref(); profiles.push_back( aces_profiles[1] );
  aces_profiles[2]->ref(); profiles.push_back( aces_profiles[2] );

  acescg_profiles[0] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACEScg-elle-V4-srgbtrc.icc");
  acescg_profiles[1] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACEScg-elle-V4-labl.icc");
  acescg_profiles[2] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACEScg-elle-V4-g10.icc");

  adobe_profiles[0] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ClayRGB-elle-V4-g22.icc");
  adobe_profiles[1] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ClayRGB-elle-V4-labl.icc");
  adobe_profiles[2] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/ClayRGB-elle-V4-g10.icc");

  prophoto_profiles[0] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/LargeRGB-elle-V4-g18.icc");
  prophoto_profiles[1] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/LargeRGB-elle-V4-labl.icc");
  prophoto_profiles[2] = get_profile(PF::PhotoFlow::Instance().get_data_dir() + "/icc/LargeRGB-elle-V4-g10.icc");

  /*
  std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/Rec2020-elle-V4-g10.icc";
  profile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );

  d50_wp_primaries = rec2020_primaries_prequantized;
   */

  /* LAB "L" (perceptually uniform) TRC */

  cmsFloat64Number labl_parameters[5] =
  { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
  Lstar_trc = cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
  iLstar_trc = cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
  iLstar_trc = cmsReverseToneCurve( iLstar_trc );

  /*
  for( int i = 0; i < 65536; i++ ) {
    cmsFloat32Number in = i, out;
    in /= 65535;
    out = cmsEvalToneCurveFloat( perceptual_trc, in )*65535;
    if( out > 65535 ) out = 65535;
    if( out < 0 ) out = 0;
    perceptual_trc_vec[i] = (int)out;
    out = cmsEvalToneCurveFloat( perceptual_trc_inv, in )*65535;
    if( out > 65535 ) out = 65535;
    if( out < 0 ) out = 0;
    perceptual_trc_inv_vec[i] = (int)out;
  }
   */

  defaultMonitorProfile = "";

#ifdef WIN32
  /*
  // Get current main monitor. Could be fine tuned to get the current windows monitor (multi monitor setup),
  // but problem is that we live in RTEngine with no GUI window to query around
  HDC hDC = GetDC(NULL);

  if (hDC != NULL) {
      if (SetICMMode(hDC, ICM_ON)) {
          char profileName[MAX_PATH + 1];
          DWORD profileLength = MAX_PATH;

          if (GetICMProfileA(hDC, &profileLength, profileName)) {
              defaultMonitorProfile = Glib::ustring(profileName);
          }

          // might fail if e.g. the monitor has no profile
      }

      ReleaseDC(NULL, hDC);
  }
  */
#else
// TODO: Add other OS specific code here
#endif
}


PF::ICCProfile* PF::ICCStore::get_profile( PF::profile_type_t ptype, PF::TRC_type trc_type)
{
  switch( ptype ) {
  case PF::OUT_PROF_sRGB: return srgb_profiles[trc_type];
  case PF::OUT_PROF_REC2020: return rec2020_profiles[trc_type];
  case PF::OUT_PROF_ACES: return aces_profiles[trc_type];
  case PF::OUT_PROF_ACEScg: return acescg_profiles[trc_type];
  case PF::OUT_PROF_ADOBE: return adobe_profiles[trc_type];
  case PF::OUT_PROF_PROPHOTO: return prophoto_profiles[trc_type];
  default: return NULL;
  }
}


PF::ICCProfile* PF::ICCStore::get_profile( Glib::ustring pname )
{
  // First we loop over all the opened profiles to see if there is
  // already one associated to the same file
  for(unsigned int pi = 0; pi < profiles.size(); pi++ ) {
    if( profiles[pi]->get_file_name() == pname ) {
      return profiles[pi];
    }
  }

  // Next, we check if an equivalent profile exists, even if not directly opened from disk
  cmsHPROFILE temp_profile = cmsOpenProfileFromFile( pname.c_str(), "r" );
  if( temp_profile ) {
    cmsUInt32Number psize;
    cmsSaveProfileToMem( temp_profile, NULL, &psize);
    if( psize > 0 ) {
      void* pdata = malloc( psize );
      if( pdata ) {
        cmsSaveProfileToMem( temp_profile, pdata, &psize);

        // We loop over all the opened profiles to see if there is one with matching data
        for(unsigned int pi = 0; pi < profiles.size(); pi++ ) {
          if( profiles[pi]->get_profile_size() == psize &&
              memcmp(profiles[pi]->get_profile_data(), pdata, psize) == 0 ) {
            return profiles[pi];
          }
        }

        free( pdata );
      }
    }

    PF::ICCProfile* new_profile = new PF::ICCProfile();
    new_profile->set_profile( temp_profile );
    new_profile->set_file_name( pname );
    new_profile->ref();

    profiles.push_back( new_profile );
    return new_profile;
  }

  return NULL;
}


PF::ICCProfile* PF::ICCStore::get_profile( void* pdata, cmsUInt32Number psize )
{
  if( !pdata || psize==0 ) return NULL;
  // We loop over all the opened profiles to see if there is one with matching data
  for(unsigned int pi = 0; pi < profiles.size(); pi++ ) {
    if( profiles[pi]->get_profile_size() == psize &&
        memcmp(profiles[pi]->get_profile_data(), pdata, psize) == 0 ) {
      return profiles[pi];
    }
  }

  void* buf = malloc( psize );
  if( !buf ) return NULL;
  memcpy( buf, pdata, psize );

  cmsHPROFILE temp_profile = cmsOpenProfileFromMem( buf, psize );

  std::cout<<"ICCStore::get_profile( void* pdata, cmsUInt32Number psize ): temp_profile="<<temp_profile<<std::endl;

  PF::ICCProfile* new_profile = new PF::ICCProfile();
  new_profile->set_profile( temp_profile );

  profiles.push_back( new_profile );
  return new_profile;
}


PF::ICCProfile* PF::ICCStore::get_profile( cmsHPROFILE prof )
{
  if( !prof ) return NULL;
  cmsUInt32Number psize;
  cmsSaveProfileToMem( prof, NULL, &psize);
  if( psize > 0 ) {
    void* pdata = g_malloc( psize );
    if( pdata ) {
      cmsSaveProfileToMem( prof, pdata, &psize);

      // We loop over all the opened profiles to see if there is one with matching data
      for(unsigned int pi = 0; pi < profiles.size(); pi++ ) {
        if( profiles[pi]->get_profile_size() == psize &&
            memcmp(profiles[pi]->get_profile_data(), pdata, psize) == 0 ) {
          g_free( pdata );
          cmsCloseProfile( prof );
          return profiles[pi];
        }
      }

      cmsHPROFILE temp_profile = cmsOpenProfileFromMem( pdata, psize );
      PF::ICCProfile* new_profile = new PF::ICCProfile();
      new_profile->set_profile( prof );

      profiles.push_back( new_profile );
      return new_profile;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }

  return NULL;
}




PF::ICCStore* PF::ICCStore::instance = NULL;

PF::ICCStore& PF::ICCStore::Instance()
{
  if(!PF::ICCStore::instance)
    PF::ICCStore::instance = new PF::ICCStore();
  return( *instance );
};
