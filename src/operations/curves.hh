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
#include "../base/pixel_processor.hh"
#include "../base/splinecurve.hh"

namespace PF 
{

  class CurvesPar: public PixelProcessorPar
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

    void update_curve( Property<SplineCurve>& grey_curve,
                       short int* vec8, int* vec16 );

  public:
    std::vector< std::pair<float,float> > Greyvec;
    std::vector< std::pair<float,float> > RGBvec[4];
    std::vector< std::pair<float,float> > Labvec[3];
    std::vector< std::pair<float,float> > CMYKvec[4];
    std::vector< std::pair<float,float> >* cvec;

    Property<SplineCurve>* scvec[4];

    short int Greyvec8[UCHAR_MAX+1];
    short int RGBvec8[4][UCHAR_MAX+1];
    short int Labvec8[3][UCHAR_MAX+1];
    short int CMYKvec8[4][UCHAR_MAX+1];
    short int * cvec8[4];

    int Greyvec16[65536/*USHRT_MAX+1*/];
    int RGBvec16[4][65536/*USHRT_MAX+1*/];
    int Labvec16[3][65536/*USHRT_MAX+1*/];
    int CMYKvec16[4][65536/*USHRT_MAX+1*/];
    int * cvec16[4];

    CurvesPar();

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
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
      //std::cout<<"CurvesProc::process() called in non_preview mode"<<std::endl;
      T* pp = p[first];
      float nin, nout;
      //int id;
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        nin = (float(pp[pos])+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
        nout = intensity*par->scvec[i]->get().get_delta( nin ) + nin;
        if( nout > 1 ) nout = 1;
        else if ( nout < 0 ) nout = 0;
        pout[pos] = T(nout*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
        /*
        id = (int)(nin*1000);
        if( id<0 || id > 1000 )
          continue;
        nout = intensity*par->cvec[i][id].second + par->cvec[i][id].first;
        if( nout > 1 ) nout = 1;
        else if ( nout < 0 ) nout = 0;
        pout[pos] = T(nout*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
        */
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





  template < colorspace_t CS, int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<unsigned char,CS,CHMIN,CHMAX,true,OP_PAR>
  {
    CurvesPar* par;
    int pos;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(unsigned char**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, unsigned char* pout)
    {
      unsigned char* pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        pout[pos] = (unsigned char)(intensity*par->cvec8[i][pp[pos]] + pp[pos]);
      }
    }
  };





  template < colorspace_t CS, int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<unsigned short int,CS,CHMIN,CHMAX,true,OP_PAR>
  {
    CurvesPar* par;
    int pos;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(unsigned short int**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, unsigned short int* pout)
    {
      unsigned short int* pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        pout[pos] = (unsigned short int)(intensity*par->cvec16[i][pp[pos]] + pp[pos]);
        //if(x==128) std::cout<<"pp[pos]="<<pp[pos]<<"  cvec16[i][pp[pos]]="<<par->cvec16[i][pp[pos]]<<"  pout[pos]="<<pout[pos]<<std::endl;
      }
    }
  };


#include "curves_rgb.hh"

  /*
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
  */
  

  template < OP_TEMPLATE_DEF > 
  class Curves: public PixelProcessor< OP_TEMPLATE_IMP, CurvesPar, CurvesProc >
  {
  };


  ProcessorBase* new_curves();
}

#endif 


