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

#ifndef PF_ICCSTORE_H
#define PF_ICCSTORE_H

#include <string.h>
#include <string>
#include <lcms2.h>
#include <vips/vips.h>

namespace PF
{

enum TRC_type
{
  PF_TRC_STANDARD=0,
  PF_TRC_PERCEPTUAL=1,
  PF_TRC_LINEAR=2
};


enum profile_type_t {
  OUT_PROF_NONE,
  OUT_PROF_EMBEDDED,
  OUT_PROF_sRGB,
  OUT_PROF_ADOBE,
  OUT_PROF_PROPHOTO,
  OUT_PROF_REC2020,
  OUT_PROF_ACES,
  OUT_PROF_ACESCG,
  OUT_PROF_LAB,
  OUT_PROF_CUSTOM
};



extern cmsToneCurve* Lstar_trc;
extern cmsToneCurve* iLstar_trc;


struct ICCProfileData
{
  cmsToneCurve* perceptual_trc;
  cmsToneCurve* perceptual_trc_inv;
  TRC_type trc_type;

  float perceptual_trc_vec[65536];
  float perceptual_trc_inv_vec[65536];

  float Y_R, Y_G, Y_B;
};


ICCProfileData* get_icc_profile_data( VipsImage* img );
void free_icc_profile_data( ICCProfileData* data );
cmsFloat32Number linear2perceptual( ICCProfileData* data, cmsFloat32Number val );
cmsFloat32Number perceptual2linear( ICCProfileData* data, cmsFloat32Number val );


class ICCProfile
{
  cmsHPROFILE profile;
  cmsToneCurve* perceptual_trc;
  cmsToneCurve* perceptual_trc_inv;
  TRC_type trc_type;

  float perceptual_trc_vec[65536];
  float perceptual_trc_inv_vec[65536];

  double colorants[9];
  float Y_R, Y_G, Y_B;

public:
  ICCProfile( TRC_type type );
  virtual ~ICCProfile();

  void init_colorants();
  void init_trc( cmsToneCurve* trc, cmsToneCurve* trc_inv );

  double* get_colorants() { return colorants; }

  TRC_type get_trc_type() { return trc_type; }
  bool is_linear() { return( get_trc_type() == PF_TRC_LINEAR ); }
  bool is_perceptual() { return( get_trc_type() == PF_TRC_PERCEPTUAL ); }
  bool is_standard() { return( get_trc_type() == PF_TRC_STANDARD ); }

  void set_profile( cmsHPROFILE p ) { profile = p; init_colorants(); }
  cmsHPROFILE get_profile(); //{ return profile; }

  cmsFloat32Number linear2perceptual( cmsFloat32Number val )
  {
    return cmsEvalToneCurveFloat( perceptual_trc_inv, val );
  }
  cmsFloat32Number perceptual2linear( cmsFloat32Number val )
  {
    return cmsEvalToneCurveFloat( perceptual_trc, val );
  }

  float* get_linear2perceptual_vec() { return perceptual_trc_inv_vec; }
  float* get_perceptual2linear_vec() { return perceptual_trc_vec; }

  float get_lightness( float R, float G, float B );
  void get_lightness( float* RGBv, float* Lv, size_t size );

  ICCProfileData* get_data()
  {
    ICCProfileData* data = new ICCProfileData;
    data->trc_type = trc_type;
    memcpy( data->perceptual_trc_vec, perceptual_trc_vec, sizeof(perceptual_trc_vec) );
    memcpy( data->perceptual_trc_inv_vec, perceptual_trc_inv_vec, sizeof(perceptual_trc_inv_vec) );
    data->perceptual_trc =  cmsDupToneCurve( perceptual_trc );
    data->perceptual_trc_inv =  cmsDupToneCurve( perceptual_trc_inv );
    data->Y_R = Y_R;
    data->Y_G = Y_G;
    data->Y_B = Y_B;

    return data;
  }
};


class ACESProfile: public ICCProfile
{
public:
  ACESProfile(TRC_type type);
};


class Rec2020Profile: public ICCProfile
{
public:
  Rec2020Profile(TRC_type type);
};


class sRGBProfile: public ICCProfile
{
public:
  sRGBProfile(TRC_type type);
};


class ICCStore
{
  ICCProfile* srgb_profiles[3];
  ICCProfile* rec2020_profiles[3];
  ICCProfile* aces_profiles[3];
  static ICCStore* instance;
public:
  ICCStore();

  static ICCStore& Instance();

  ICCProfile* get_srgb_profile(TRC_type type) { return srgb_profiles[type]; }
  ICCProfile* get_profile(profile_type_t ptype, TRC_type trc_type);

  cmsToneCurve* get_Lstar_trc() {return Lstar_trc; }
  cmsToneCurve* get_iLstar_trc() {return iLstar_trc; }
};
}


#endif 


