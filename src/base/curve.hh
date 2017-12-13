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


#ifndef CURVE_H
#define CURVE_H

#include <map>
#include <vector>


namespace PF
{

  class Curve
  {
  public:
    Curve();
    virtual ~Curve();

    // Get the output value corresponding to an input value x (normalized to the [0,1] range)
    virtual float get_value( float x ) = 0;

    // Fill a vector of equally-spaced npoints with input-output value pairs
    virtual void get_values( std::vector< std::pair<float,float> >& vec ) = 0;
  };

}


#endif
