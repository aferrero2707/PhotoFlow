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
#include <iostream>
#include "../photoflow.hh"
#include "../iccstore.hh"


/* ***** Make profile: ACES, D60, gamma=1.00 */
/* ACES chromaticities taken from
 * Specification
 * */
static cmsCIExyYTRIPLE aces_primaries =
{
    {0.73470,  0.26530,  1.0},
    {0.00000,  1.00000,  1.0},
    {0.00010, -0.07700,  1.0}
};

static cmsCIExyYTRIPLE aces_primaries_prequantized =
{
    {0.734704192222, 0.265298276252,  1.0},
    {-0.000004945077, 0.999992850272,  1.0},
    {0.000099889199, -0.077007518685,  1.0}
};

/* ACES white point, taken from
 * Specification S-2014-004
 * ACEScg – A Working Space for CGI Render and Compositing
 */
cmsCIExyY d60_aces= {0.32168, 0.33767, 1.0};


PF::ACESProfile::ACESProfile(TRC_type type): ICCProfile()
{
  set_trc_type( type );

  /*
  if( type == PF::PF_TRC_STANDARD ) {
    // Rec 709 TRC
    cmsFloat64Number rec709_parameters[5] =
    { 1.0 / 0.45, 1.0 / 1.099,  0.099 / 1.099,  1.0 / 4.5, 0.018 };
    cmsToneCurve *rec709_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    cmsToneCurve *rec709_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    rec709_parametic_curve_inv = cmsReverseToneCurve( rec709_parametic_curve_inv );
    //init_trc( rec709_parametic_curve, rec709_parametic_curve_inv );
  } else {
    // LAB "L" (perceptually uniform) TRC
    cmsFloat64Number labl_parameters[5] =
    { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
    cmsToneCurve *labl_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    cmsToneCurve *labl_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    labl_parametic_curve_inv = cmsReverseToneCurve( labl_parametic_curve_inv );
    //init_trc( labl_parametic_curve, labl_parametic_curve_inv );
  }
  */

  /* ***** Make profile: Rec.2020, D65, Rec709 TRC */
  /*
   * */
  cmsCIExyYTRIPLE primaries = aces_primaries_prequantized;
  cmsCIExyY whitepoint = d60_aces;
  /* rec.709 */
  cmsToneCurve* tone_curve[3] = {NULL};
  cmsHPROFILE profile = NULL;
  switch( type ) {
  case PF::PF_TRC_STANDARD: {
    /* Rec 709 TRC */
    cmsFloat64Number rec709_parameters[5] =
    { 1.0 / 0.45, 1.0 / 1.099,  0.099 / 1.099,  1.0 / 4.5, 0.018 };
    cmsToneCurve *curve = cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    //std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACES-elle-V4-rec709.icc";
    //std::cout<<"loading profile "<<wprofname<<std::endl;
    //profile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );
    break;
  }
  case PF::PF_TRC_PERCEPTUAL: {
    cmsFloat64Number labl_parameters[5] =
    { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
    cmsToneCurve *curve =
        cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    //std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACES-elle-V4-labl.icc";
    //std::cout<<"loading profile "<<wprofname<<std::endl;
    //profile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );
    break;
  }
  case PF::PF_TRC_LINEAR: {
    cmsToneCurve *curve = cmsBuildGamma (NULL, 1.00);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    //std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACES-elle-V4-g10.icc";
    //std::cout<<"loading profile "<<wprofname<<std::endl;
    //profile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );
    break;
  }
  case PF::PF_TRC_sRGB: {
    /* sRGB TRC */
    cmsFloat64Number srgb_parameters[5] =
    { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };
    cmsToneCurve *curve = cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
    //cmsToneCurve *curve = cmsBuildTabulatedToneCurve16(NULL, dt_srgb_tone_curve_values_n, dt_srgb_tone_curve_values);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  case PF::PF_TRC_GAMMA_22: {
    cmsToneCurve *curve = cmsBuildGamma (NULL, 2.20);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  case PF::PF_TRC_GAMMA_18: {
    cmsToneCurve *curve = cmsBuildGamma (NULL, 1.80);
    tone_curve[0] = tone_curve[1] = tone_curve[2] = curve;
    break;
  }
  }
  /**/
  profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
  cmsMLU *copyright = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(copyright, "en", "US", "Copyright 2015, Elle Stone (website: http://ninedegreesbelow.com/; email: ellestone@ninedegreesbelow.com). This ICC profile is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License (https://creativecommons.org/licenses/by-sa/3.0/legalcode).");
  cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
  // V4
  cmsMLU *description = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(description, "en", "US", "ACES-elle-V4.icc");
  cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
  //const char* filename = "ACES-elle-V4-rec709.icc";
  //cmsSaveProfileToFile(profile, filename);
  cmsMLUfree(description);
  /**/

  std::cout<<"Initializing ACES profile"<<std::endl;
  set_profile( profile );
}
