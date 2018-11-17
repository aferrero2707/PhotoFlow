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

#include "color_correction.hh"



PF::ColorCorrectionPar::ColorCorrectionPar():
  OpParBase(),
  r_offs("r_offs",this,0),
  g_offs("g_offs",this,0),
  b_offs("b_offs",this,0),
  r_slope("r_slope",this,1),
  g_slope("g_slope",this,1),
  b_slope("b_slope",this,1),
  r_pow("r_pow",this,1),
  g_pow("g_pow",this,1),
  b_pow("b_pow",this,1),
  saturation("saturation",this,1)
{
  set_type("color_correction" );

  set_default_name( _("color correction") );
  transform = NULL;
}



PF::ColorCorrectionPar::~ColorCorrectionPar()
{
}



VipsImage* PF::ColorCorrectionPar::build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level)
{
  if( in.size() < 1 )
    return NULL;

  icc_data = PF::get_icc_profile( in[0] );

  VipsImage* out = PF::OpParBase::build( in, 0, imap, omap, level );
  return out;
}
