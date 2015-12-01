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

#include <lcms2.h>

namespace PF
{

  class ICCStore
  {
    cmsHPROFILE wprofile;
    cmsToneCurve* perceptual_trc;
    cmsToneCurve* perceptual_trc_inv;

    int perceptual_trc_vec[65536];
    int perceptual_trc_inv_vec[65536];

    cmsCIExyYTRIPLE d50_wp_primaries;

    static ICCStore* instance;
  public:
    ICCStore();

    static ICCStore& Instance();

    cmsHPROFILE get_working_profile() { return wprofile; }

    cmsFloat32Number linear2perceptual( cmsFloat32Number val )
    {
      return cmsEvalToneCurveFloat( perceptual_trc_inv, val );
    }
    cmsFloat32Number perceptual2linear( cmsFloat32Number val )
    {
      return cmsEvalToneCurveFloat( perceptual_trc, val );
    }

    int* get_linear2perceptual_vec() { return perceptual_trc_inv_vec; }
    int* get_perceptual2linear_vec() { return perceptual_trc_vec; }

    float get_luminosity( float R, float G, float B )
    {
      return( d50_wp_primaries.Red.y*R + d50_wp_primaries.Green.y*G + d50_wp_primaries.Blue.y*B );
    }
  };
}


#endif 


