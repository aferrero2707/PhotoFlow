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

#ifndef PF_FORMAT_INFO_H
#define PF_FORMAT_INFO_H


#include "pftypes.hh"

namespace PF
{

  template<class T> 
  struct FormatInfo
  {
    typedef T PROMOTED;
    typedef T SIGNED;
    static T MIN, MAX, HALF;
    static PROMOTED RANGE;
  };

  template<class T>
  T FormatInfo<T>::MIN = 0;

  template<class T>
  T FormatInfo<T>::MAX = 1;

  template<class T>
  T FormatInfo<T>::HALF = 0.5;

  template<class T>
  typename FormatInfo<T>::PROMOTED FormatInfo<T>::RANGE = 1;




  template<> 
  struct FormatInfo<int8_t>
  {
    typedef int16_t PROMOTED;
    typedef int16_t SIGNED;
    static int8_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  template<> 
  struct FormatInfo<uint8_t>
  {
    typedef uint16_t PROMOTED;
    typedef int32_t SIGNED;
    static uint8_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  template<> 
  struct FormatInfo<int16_t>
  {
    typedef int32_t PROMOTED;
    typedef int32_t SIGNED;
    static int16_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  template<> 
  struct FormatInfo<uint16_t>
  {
    typedef uint32_t PROMOTED;
    typedef int64_t SIGNED;
    static uint16_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  template<> 
  struct FormatInfo<int32_t>
  {
    typedef int64_t PROMOTED;
    typedef int32_t SIGNED;
    static int32_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  template<> 
  struct FormatInfo<uint32_t>
  {
    typedef uint64_t PROMOTED;
    typedef int64_t SIGNED;
    static uint32_t MIN, MAX, HALF;
    static PROMOTED RANGE;
  };


  /*
  template < typename T > 
  void clip(const typename FormatInfo<T>::PROMOTED& val, T& clipped)
  {
    if(val > FormatInfo<T>::MAX) clipped = FormatInfo<T>::MAX;
    else if(val < FormatInfo<T>::MIN) clipped = FormatInfo<T>::MIN;
    else clipped = (T)val;
    //std::cout<<"val="<<(int)val<<"  max="<<(int)FormatInfo<T>::MAX<<"  min="<<(int)FormatInfo<T>::MIN<<"  clipped="<<(int)clipped<<std::endl;
  }
  */

  template < typename T > 
  void clip(const typename FormatInfo<T>::SIGNED& val, T& clipped)
  {
    if(val > FormatInfo<T>::MAX) clipped = FormatInfo<T>::MAX;
    else if(val < FormatInfo<T>::MIN) clipped = FormatInfo<T>::MIN;
    else clipped = (T)val;
    //std::cout<<"val="<<(int)val<<"  max="<<(int)FormatInfo<T>::MAX<<"  min="<<(int)FormatInfo<T>::MIN<<"  clipped="<<(int)clipped<<std::endl;
  }


  cmsUInt32Number vips2lcms_pixel_format( VipsBandFormat vipsFmt, cmsHPROFILE pin );
}


#endif
