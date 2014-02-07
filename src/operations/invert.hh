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

#ifndef VIPS_INVERT_H
#define VIPS_INVERT_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"

namespace PF 
{

  class InvertPar: public OpParBase
  {
  public:
    InvertPar(): OpParBase() {}
  };

  
  template < typename T, colorspace_t CS, bool PREVIEW, class OP_PAR >
  class InvertProc
  {
    InvertPar* par;
  public:
    InvertProc(InvertPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout) {}
  };

  
  template < typename T, bool PREVIEW, class OP_PAR >
  class InvertProc<T, PF_COLORSPACE_GRAYSCALE, PREVIEW, OP_PAR>
  {
    InvertPar* par;
  public:
    InvertProc(InvertPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      pout[x] = (T)(FormatInfo<T>::RANGE - p[first][x]); 
    }
  };

  
  template < typename T, bool PREVIEW, class OP_PAR >
  class InvertProc<T, PF_COLORSPACE_RGB, PREVIEW, OP_PAR>
  {
    InvertPar* par;
  public:
    InvertProc(InvertPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      /**/
      int i = x;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      /**/
      /*
      T* pp = p[first];
      for(int i = 0; i < 3; i++) {
	pout[x+i] = (T)(FormatInfo<T>::RANGE - pp[x+i]); 
      }
      */
    }
  };

  
  template < typename T, bool PREVIEW, class OP_PAR >
  class InvertProc<T, PF_COLORSPACE_LAB, PREVIEW, OP_PAR>
  {
    InvertPar* par;
  public:
    InvertProc(InvertPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      int i = x;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
    }
  };

  
  template < typename T, bool PREVIEW, class OP_PAR >
  class InvertProc<T, PF_COLORSPACE_CMYK, PREVIEW, OP_PAR>
  {
    InvertPar* par;
  public:
    InvertProc(InvertPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      int i = x;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
      i += 1;
      pout[i] = (T)(FormatInfo<T>::RANGE - p[first][i]); 
    }
  };

  

  template < OP_TEMPLATE_DEF > 
  class Invert: public OperationPTP< OP_TEMPLATE_IMP, 
				     InvertProc<T,CS,PREVIEW,InvertPar>, 
				     InvertPar >
  {
  };

}

#endif 


