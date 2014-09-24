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


#ifndef PF_COLOR__HH
#define PF_COLOR__HH

#ifndef MIN
#define MIN( a, b ) ((a<b) ? a : b)
#endif
#define MIN3( a, b, c ) MIN(a,MIN(b,c))

#ifndef MAX
#define MAX( a, b ) ((a>b) ? a : b)
#endif
#define MAX3( a, b, c ) MAX(a,MAX(b,c))


#include "format_info.hh"

namespace PF
{


  // Computes the luminance value of a given RGB triplet
  template<class T>
  T luminance(const T& r, const T& g, const T& b)
  {
    return( r*0.3 + g*0.59 + b*0.11 );
  }


  // Clips the RGB values resulting from a luminosity/color blend
  template<class T>
  void clip_lc_blend(const typename FormatInfo<T>::SIGNED& rin, 
                     const typename FormatInfo<T>::SIGNED& gin, 
                     const typename FormatInfo<T>::SIGNED& bin, 
                     T& rout, T& gout, T& bout)
  {
    typename FormatInfo<T>::SIGNED lumi = luminance( rin, gin, bin );
    typename FormatInfo<T>::SIGNED  min = MIN3( rin, gin, bin );
    typename FormatInfo<T>::SIGNED  max = MAX3( rin, gin, bin );
    //std::cout<<"clip_lc_blend(): lumi="<<lumi<<"  min="<<min
    //         <<"  max="<<max<<std::endl;

    rout = rin;
    gout = gin;
    bout = bin;
    if( min < FormatInfo<T>::MIN ) {
      rout = (T)(lumi + (((rin-lumi)*lumi)/(lumi-min)));
      gout = (T)(lumi + (((gin-lumi)*lumi)/(lumi-min)));
      bout = (T)(lumi + (((bin-lumi)*lumi)/(lumi-min)));
    }
    if( max > FormatInfo<T>::MAX ) {
      rout = (T)(lumi + (((rin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
      gout = (T)(lumi + (((gin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
      bout = (T)(lumi + (((bin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
    }
    //std::cout<<"                  input:  "<<rin<<" "<<gin<<" "<<bin<<std::endl
    //         <<"                  output: "<<rout<<" "<<gout<<" "<<bout<<std::endl;
  }


  // Blends a given RGB triplet and a given luminance value.
  // The input RGB values are replaced by the blended ones.
  template<class T>
  void lc_blend(T& red, T& green, T& blue, const T& lumi)
  {
    typename FormatInfo<T>::SIGNED diff = 
      ((typename FormatInfo<T>::SIGNED)lumi) - luminance( red, green, blue );
    //std::cout<<"Luminance top="<<lumi<<"  bottom="<<luminance( red, green, blue )<<"  diff="<<diff<<std::endl;
    typename FormatInfo<T>::SIGNED newred = ((typename FormatInfo<T>::SIGNED)red) + diff;
    typename FormatInfo<T>::SIGNED newgreen = ((typename FormatInfo<T>::SIGNED)green) + diff;
    typename FormatInfo<T>::SIGNED newblue = ((typename FormatInfo<T>::SIGNED)blue) + diff;
    clip_lc_blend( newred, newgreen, newblue, red, green, blue );
    //std::cout<<"red="<<red<<" new="<<newred<<"  green="<<green<<" new="<<newgreen<<"  blue="<<blue<<" new="<<newblue<<std::endl;
  }
}

#endif
