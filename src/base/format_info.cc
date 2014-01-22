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

#include "format_info.hh"


namespace PF {

  int8_t FormatInfo<int8_t>::MIN = SCHAR_MIN;
  int8_t FormatInfo<int8_t>::MAX = SCHAR_MAX;
  int8_t FormatInfo<int8_t>::HALF = 0;
  FormatInfo<int8_t>::PROMOTED FormatInfo<int8_t>::RANGE = 
    (FormatInfo<int8_t>::PROMOTED)FormatInfo<int8_t>::MAX - FormatInfo<int8_t>::MIN;


  uint8_t FormatInfo<uint8_t>::MIN = 0;
  uint8_t FormatInfo<uint8_t>::MAX = UCHAR_MAX;
  uint8_t FormatInfo<uint8_t>::HALF = UCHAR_MAX/2;
  FormatInfo<uint8_t>::PROMOTED FormatInfo<uint8_t>::RANGE = FormatInfo<uint8_t>::MAX;

  // 16-bit base types
#if (USHRT_MAX == 65535U)
#define PF_USHRT_MAX USHRT_MAX
#define PF_SHRT_MIN SHRT_MIN
#define PF_SHRT_MAX SHRT_MAX
#elif (UINT_MAX == 65535U)
#define PF_USHRT_MAX UINT_MAX
#define PF_SHRT_MIN INT_MIN
#define PF_SHRT_MAX INT_MAX
#else
#  error "Unable to find 16 bits unsigned type, unsupported compiler"
#endif

  int16_t FormatInfo<int16_t>::MIN = PF_SHRT_MIN;
  int16_t FormatInfo<int16_t>::MAX = PF_SHRT_MAX;
  int16_t FormatInfo<int16_t>::HALF = 0;
  FormatInfo<int16_t>::PROMOTED FormatInfo<int16_t>::RANGE = 
    (FormatInfo<int16_t>::PROMOTED)FormatInfo<int16_t>::MAX - FormatInfo<int16_t>::MIN;


  uint16_t FormatInfo<uint16_t>::MIN = 0;
  uint16_t FormatInfo<uint16_t>::MAX = PF_USHRT_MAX;
  uint16_t FormatInfo<uint16_t>::HALF = PF_USHRT_MAX/2;
  FormatInfo<uint16_t>::PROMOTED FormatInfo<uint16_t>::RANGE = FormatInfo<uint16_t>::MAX;


  // 16-bit base types
#if (UINT_MAX == 4294967295U)
#define PF_UINT_MAX UINT_MAX
#define PF_INT_MIN INT_MIN
#define PF_INT_MAX INT_MAX
#elif (ULONG_MAX == 65535U)
#define PF_UINT_MAX ULONG_MAX
#define PF_INT_MIN LONG_MIN
#define PF_INT_MAX LONG_MAX
#else
#  error "Unable to find 16 bits unsigned type, unsupported compiler"
#endif

  int32_t FormatInfo<int32_t>::MIN = PF_INT_MIN;
  int32_t FormatInfo<int32_t>::MAX = PF_INT_MAX;
  int32_t FormatInfo<int32_t>::HALF = 0;
  FormatInfo<int32_t>::PROMOTED FormatInfo<int32_t>::RANGE = 
    (FormatInfo<int32_t>::PROMOTED)FormatInfo<int32_t>::MAX - FormatInfo<int32_t>::MIN;


  uint32_t FormatInfo<uint32_t>::MIN = 0;
  uint32_t FormatInfo<uint32_t>::MAX = PF_UINT_MAX;
  uint32_t FormatInfo<uint32_t>::HALF = PF_UINT_MAX/2;
  FormatInfo<uint32_t>::PROMOTED FormatInfo<uint32_t>::RANGE = FormatInfo<uint32_t>::MAX;


}
