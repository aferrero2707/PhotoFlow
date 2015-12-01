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

PF::ICCStore::ICCStore()
{
  std::string wprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/Rec2020-elle-V4-g10.icc";
  wprofile = cmsOpenProfileFromFile( wprofname.c_str(), "r" );

  d50_wp_primaries = rec2020_primaries_prequantized;

  /* LAB "L" (perceptually uniform) TRC */
  cmsFloat64Number labl_parameters[5] =
     { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 };
  perceptual_trc = cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
  perceptual_trc_inv = cmsBuildParametricToneCurve(NULL, 4, labl_parameters);
  perceptual_trc_inv = cmsReverseToneCurve( perceptual_trc_inv );

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


PF::ICCStore* PF::ICCStore::instance = NULL;

PF::ICCStore& PF::ICCStore::Instance()
{
  if(!PF::ICCStore::instance)
    PF::ICCStore::instance = new PF::ICCStore();
  return( *instance );
};
