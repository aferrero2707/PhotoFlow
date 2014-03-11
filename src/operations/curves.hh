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

#ifndef CURVES_OP_H
#define CURVES_OP_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"
#include "../base/splinecurve.hh"

namespace PF 
{

  class CurvesPar: public OpParBase
  {
    Property<SplineCurve> grey_curve;
    Property<SplineCurve> RGB_curve;
    Property<SplineCurve> R_curve;
    Property<SplineCurve> G_curve;
    Property<SplineCurve> B_curve;
    Property<SplineCurve> L_curve;
    Property<SplineCurve> a_curve;
    Property<SplineCurve> b_curve;
    Property<SplineCurve> C_curve;
    Property<SplineCurve> M_curve;
    Property<SplineCurve> Y_curve;
    Property<SplineCurve> K_curve;
    PropertyBase RGB_active_curve;
    PropertyBase Lab_active_curve;
    PropertyBase CMYK_active_curve;

  public:
    std::vector< std::pair<float,float> > Greyvec;
    std::vector< std::pair<float,float> > RGBvec[4];
    std::vector< std::pair<float,float> > Labvec[3];
    std::vector< std::pair<float,float> > CMYKvec[4];
    std::vector< std::pair<float,float> >* cvec;

    CurvesPar();

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap);
  };

  

  template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class CurvesProc
  {
    CurvesPar* par;
    int pos;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout)
    {
      T* pp = p[first];
      float nin, nout;
      int id;
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
	nin = (float(pp[pos])+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
	id = (int)(nin*1000);
	if( id<0 || id > 1000 )
	  continue;
	nout = intensity*par->cvec[i][id].second + par->cvec[i][id].first;
	if( nout > 1 ) nout = 1;
	else if ( nout < 0 ) nout = 0;
	pout[pos] = T(nout*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
	/*
	std::cout<<"RGBvec["<<i<<"]["<<id<<"].second="
		 <<par->RGBvec[i][id].second
		 <<"  RGBvec["<<i<<"]["<<id<<"].first="
		 <<par->RGBvec[i][id].first
		 <<"    nout="<<nout<<std::endl;
	std::cout<<"i="<<i<<"  pp[pos]="<<(int)pp[pos]<<" nin="<<nin<<"  id="<<id<<"  nout="<<nout<<"  pout[pos]="<<(int)pout[pos]<<std::endl;
	*/
      }
    }
  };

  
  template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class CurvesProc<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, PREVIEW, OP_PAR>
  {
    CurvesPar* par;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T* pout) 
    {
      pout[x] = (T)(FormatInfo<T>::RANGE - p[first][x]); 
    }
  };

  

  template < OP_TEMPLATE_DEF > 
  class Curves: public OperationPTP< OP_TEMPLATE_IMP, CurvesPar, CurvesProc >
  {
  };

}

#endif 


