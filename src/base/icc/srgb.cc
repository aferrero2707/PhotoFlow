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
#include "../iccstore.hh"


/* ***** Make profile: sRGB, D65, sRGB TRC */
/* http://en.wikipedia.org/wiki/Srgb */
/* Hewlett-Packard and Microsoft designed sRGB to match
 * the color gamut of consumer-grade CRTs from the 1990s
 * and to be the standard color space for the world wide web.
 * When made using the standard sRGB TRC, this sRGB profile
 * can be applied to DCF R03 camera-generated jpegs and
 * is an excellent color space for editing 8-bit images.
 * When made using the linear gamma TRC, the resulting profile
 * should only be used for high bit depth image editing.
 * */
static cmsCIExyYTRIPLE srgb_primaries = {
    {0.6400, 0.3300, 1.0},
    {0.3000, 0.6000, 1.0},
    {0.1500, 0.0600, 1.0}
};

static cmsCIExyYTRIPLE srgb_primaries_pre_quantized = {
    {0.639998686, 0.330010138, 1.0},
    {0.300003784, 0.600003357, 1.0},
    {0.150002046, 0.059997204, 1.0}
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


PF::sRGBProfile::sRGBProfile(TRC_type type): ICCProfile()
{
  set_trc_type( type );

  /*
  if( type == PF::PF_TRC_STANDARD ) {
    // sRGB TRC
    cmsFloat64Number srgb_parameters[5] =
    { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };
    cmsToneCurve *srgb_parametic_curve =
        cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
    cmsToneCurve *srgb_parametic_curve_inv =
        cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
    srgb_parametic_curve_inv = cmsReverseToneCurve( srgb_parametic_curve_inv );
    //init_trc( srgb_parametic_curve, srgb_parametic_curve_inv );
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

  /* ***** Make profile: sRGB, D65, sRGB TRC */
  /*
   * */
  cmsCIExyYTRIPLE primaries = srgb_primaries_pre_quantized;
  cmsCIExyY whitepoint = d65_srgb_adobe_specs;
  cmsToneCurve* tone_curve[3];
  switch( type ) {
  case PF::PF_TRC_STANDARD: {
    /* sRGB TRC */
    cmsFloat64Number srgb_parameters[5] =
    { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };
    cmsToneCurve *curve = cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
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
  cmsMLUsetASCII(description, "en", "US", "sRGB-elle-V4.icc");
  cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
  //const char* filename = "sRGB-elle-V4-rec709.icc";
  //cmsSaveProfileToFile(profile, filename);
  cmsMLUfree(description);

  std::cout<<"Initializing sRGB profile"<<std::endl;
  set_profile( profile );
}
