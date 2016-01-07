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
#include "../iccstore.hh"


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

/* ************************** WHITE POINTS ************************** */

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


PF::Rec2020Profile::Rec2020Profile(TRC_type type): ICCProfile( type )
{
  if( type == PF::PF_TRC_STANDARD ) {
    /* Rec 709 TRC *//*
    cmsFloat64Number rec709_parameters[5] =
    { 1.0 / 0.45, 1.099,  0.099, 4.500, 0.018 };
    cmsToneCurve *rec709_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    cmsToneCurve *rec709_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
    rec709_parametic_curve_inv = cmsReverseToneCurve( rec709_parametic_curve_inv );
    init_trc( rec709_parametic_curve, rec709_parametic_curve_inv );*/
    /* sRGB TRC */
    cmsFloat64Number srgb_parameters[5] =
    { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };
    cmsToneCurve *srgb_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
    cmsToneCurve *srgb_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
    srgb_parametic_curve_inv = cmsReverseToneCurve( srgb_parametic_curve_inv );
    init_trc( srgb_parametic_curve, srgb_parametic_curve_inv );
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
  //const char* filename = "Rec2020-elle-V4-rec709.icc";
  //cmsSaveProfileToFile(profile, filename);
  cmsMLUfree(description);

  set_profile( profile );
}
