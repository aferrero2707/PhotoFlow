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

#ifndef VIPS_UNIFORM_H
#define VIPS_UNIFORM_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"

namespace PF 
{

  class UniformPar: public OpParBase
  {
    Property<float> grey, R, G, B, L, a, b, C, M, Y, K;

  public:
    UniformPar();

    Property<float>& get_grey() { return grey; }
    Property<float>& get_R() { return R; }
    Property<float>& get_G() { return G; }
    Property<float>& get_B() { return B; }
    Property<float>& get_L() { return L; }
    Property<float>& get_a() { return a; }
    Property<float>& get_b() { return a; }
    Property<float>& get_C() { return C; }
    Property<float>& get_M() { return M; }
    Property<float>& get_Y() { return Y; }
    Property<float>& get_K() { return K; }
  };

  

  template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UniformProc
  {
    UniformPar* par;
  public:
    UniformProc(UniformPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout) {}
  };

  
  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UniformProc<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    UniformPar* par;
    T val;
  public:
    UniformProc(UniformPar* p): par(p) 
    {
      val = (T)(par->get_grey().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
    }

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      pout[x] = val;
    }
  };

  
  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UniformProc<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    UniformPar* par;
    T val[3];
  public:
    UniformProc(UniformPar* p): par(p) 
    {
      val[0] = (T)(par->get_R().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[1] = (T)(par->get_G().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[2] = (T)(par->get_B().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
    }

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      
      for(int i = CHMIN; i <= CHMAX; i++) {
	pout[x+i] = val[i];
      }
    }
  };

  

  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UniformProc<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    UniformPar* par;
    T val[3];
  public:
    UniformProc(UniformPar* p): par(p) 
    {
      val[0] = (T)(p->get_L().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[1] = (T)(p->get_a().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[2] = (T)(p->get_b().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
    }

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      
      for(int i = CHMIN; i <= CHMAX; i++) {
	pout[x+i] = val[i];
      }
    }
  };

  

  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class UniformProc<T, PF_COLORSPACE_CMYK, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    UniformPar* par;
    T val[4];
  public:
    UniformProc(UniformPar* p): par(p) 
    {
      val[0] = (T)(p->get_C().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[1] = (T)(p->get_M().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[2] = (T)(p->get_Y().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
      val[3] = (T)(p->get_K().get()*FormatInfo<T>::RANGE + FormatInfo<T>::MIN); 
    }

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      
      for(int i = CHMIN; i <= CHMAX; i++) {
	pout[x+i] = val[i];
      }
    }
  };

  

  template < OP_TEMPLATE_DEF > 
  class Uniform: public OperationPTP< OP_TEMPLATE_IMP, UniformPar, UniformProc >
  {
  };


  ProcessorBase* new_uniform();
}

#endif 


