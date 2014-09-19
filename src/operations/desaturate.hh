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

#ifndef VIPS_DESATURATE_H
#define VIPS_DESATURATE_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"

namespace PF 
{

  class DesaturatePar: public OpParBase
  {
  public:
    DesaturatePar();
  };

  

  template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class DesaturateProc
  {
    DesaturatePar* par;
  public:
    DesaturateProc(DesaturatePar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout) {}
  };

  
  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class DesaturateProc<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    DesaturatePar* par;
  public:
    DesaturateProc(DesaturatePar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      T* pp = p[first];
      T val = 0.21f*pp[x] + 0.72f*pp[x+1] + 0.07f*pp[x+2];
      for(int i = CHMIN; i <= CHMAX; i++) {
        pout[x+i] = val;
      }
    }
  };

  
  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class DesaturateProc<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    DesaturatePar* par;
  public:
    DesaturateProc(DesaturatePar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      T* pp = p[first];
      for(int i = CHMIN; i <= CHMAX; i++) {
        pout[x+i] = ( (i==0) ? pp[x] : FormatInfo<T>::HALF );
      }
    }
  };

  

  template < OP_TEMPLATE_DEF > 
  class Desaturate: public OperationPTP< OP_TEMPLATE_IMP, DesaturatePar, DesaturateProc >
  {
  };


  ProcessorBase* new_desaturate();
}

#endif 


