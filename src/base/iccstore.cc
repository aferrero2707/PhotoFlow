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

cmsCIExyY d50_romm_spec= {0.3457, 0.3585, 1.0};
/* http://photo-lovers.org/pdf/color/romm.pdf */

cmsCIExyY d50_illuminant_specs = {0.345702915, 0.358538597, 1.0};
/* calculated from D50 illuminant XYZ values in ICC specs */


/* D65 WHITE POINTS */

cmsCIExyY  d65_srgb_adobe_specs = {0.3127, 0.3290, 1.0};
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


PF::ICCProfile::ICCProfile( TRC_type type )
{
  trc_type = type;
  perceptual_trc = NULL;
  perceptual_trc_inv = NULL;
}


PF::ICCProfile::~ICCProfile()
{

}


void PF::ICCProfile::calc_primaries()
{
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
}


void PF::ICCProfile::init_trc( cmsToneCurve* trc, cmsToneCurve* trc_inv )
{
  perceptual_trc = trc;
  perceptual_trc_inv = trc_inv;

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
}


float PF::ICCProfile::get_luminosity( float R, float G, float B )
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

void PF::ICCProfile::get_luminosity( float* RGBv, float* Lv, size_t size )
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



PF::Rec2020Profile::Rec2020Profile(TRC_type type): ICCProfile( type )
{
  if( type == PF::PF_TRC_STANDARD ) {
    /* Rec 709 TRC */
    cmsFloat64Number rec709_parameters[5] =
    { 1.0 / 0.45, 1.099,  0.099, 4.500, 0.018 };
    cmsToneCurve *rec709_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    cmsToneCurve *rec709_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    rec709_parametic_curve_inv = cmsReverseToneCurve( rec709_parametic_curve_inv );
    init_trc( rec709_parametic_curve, rec709_parametic_curve_inv );
  } else {
    /* LAB "L" (perceptually uniform) TRC */
    cmsFloat64Number labl_parameters[5] =
    { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
    cmsToneCurve *labl_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    cmsToneCurve *labl_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    labl_parametic_curve_inv = cmsReverseToneCurve( labl_parametic_curve_inv );
    init_trc( labl_parametic_curve, labl_parametic_curve_inv );
  }

  /* ***** Make profile: Rec.2020, D65, Rec709 TRC */
  /*
   * */
  cmsCIExyYTRIPLE primaries = rec2020_primaries_prequantized;
  cmsCIExyY whitepoint = d65_srgb_adobe_specs;
  /* rec.709 */
  cmsToneCurve* tone_curve[3];
  switch( type ) {
  case PF::PF_TRC_STANDARD: {
    /* Rec 709 TRC */
    cmsFloat64Number rec709_parameters[5] =
    { 1.0 / 0.45, 1.099,  0.099, 4.500, 0.018 };
    cmsToneCurve *curve = cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  case PF::PF_TRC_PERCEPTUAL: {
    cmsFloat64Number labl_parameters[5] =
    { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
    cmsToneCurve *curve =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  case PF::PF_TRC_LINEAR: {
    cmsToneCurve *curve = cmsBuildGamma (NULL, 1.00);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  }
  cmsHPROFILE profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
  cmsMLU *copyright = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(copyright, "en", "US", "Copyright 2015, Elle Stone (website: http://ninedegreesbelow.com/; email: ellestone@ninedegreesbelow.com). This ICC profile is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License (https://creativecommons.org/licenses/by-sa/3.0/legalcode).");
  cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
  /* V4 */
  cmsMLU *description = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(description, "en", "US", "Rec2020-elle-V4.icc");
  cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
  const char* filename = "Rec2020-elle-V4-rec709.icc";
  cmsSaveProfileToFile(profile, filename);
  cmsMLUfree(description);

  set_profile( profile );
}



PF::ICCStore::ICCStore()
{
  rec2020_profiles[0] = new Rec2020Profile( PF::PF_TRC_STANDARD );
  rec2020_profiles[1] = new Rec2020Profile( PF::PF_TRC_PERCEPTUAL );
  rec2020_profiles[2] = new Rec2020Profile( PF::PF_TRC_LINEAR );
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

  for(int i = 0; i < 256; i++) {
    float fi = i;
    fi /= 65535;
    float fo = cmsEvalToneCurveFloat( Lstar_trc, fi );
    //std::cout<<"lin="<<fi*65535<<"  perc="<<fo*65535<<"  ("<<fo/fi<<")"<<std::endl;
  }

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
}


PF::ICCStore* PF::ICCStore::instance = NULL;

PF::ICCStore& PF::ICCStore::Instance()
{
  if(!PF::ICCStore::instance)
    PF::ICCStore::instance = new PF::ICCStore();
  return( *instance );
};
