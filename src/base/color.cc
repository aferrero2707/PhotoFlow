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


#include "color.hh"


namespace PF
{


  template<>
  void to_float<float>( const float& in, float& out )
  {
    out = in;
  }

  template<>
  void from_float<float>( const float& in, float& out )
  {
    out = in;
  }
}

void PF::Lab_pf2lcms(float* p)
{
  p[0] = (cmsFloat32Number) (p[0] * 100.0);
  p[1] = (cmsFloat32Number) (p[1]*256.0f - 128.0f);
  p[2] = (cmsFloat32Number) (p[2]*256.0f - 128.0f);

}

void PF::Lab_pf2lcms(float* pin, float* pout)
{
  pout[0] = (cmsFloat32Number) (pin[0] * 100.0);
  pout[1] = (cmsFloat32Number) (pin[1]*256.0f - 128.0f);
  pout[2] = (cmsFloat32Number) (pin[2]*256.0f - 128.0f);

}

float PF::hsl_value( float n1, float n2, float hue)
{
  float val;

  if( hue > 6.0 ) hue -= 6.0f;
  else if( hue < 0 ) hue += 6.0f;

  if( hue < 1.0 ) val = n1 + (n2-n1)*hue;
  else if( hue < 3.0 ) val = n2;
  else if( hue < 4.0 ) val = n1 + (n2-n1)*(4.0f-hue);
  else val = n1;

  return val;
}

