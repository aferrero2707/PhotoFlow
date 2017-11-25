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


  template < typename T, int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<T,PF_COLORSPACE_RGB,CHMIN,CHMAX,false,OP_PAR>
  {
    CurvesPar* par;
    int pos;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T* pout)
    {
      //std::cout<<"CurvesProc::process() called in non_preview mode"<<std::endl;
      T* pp = p[first];
      float nin, nout, d1, d2;
      //int id;
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        nin = (float(pp[pos])+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
        d1 = par->scvec[i]->get().get_delta( nin );
        d2 = par->scvec[3]->get().get_delta( nin );
        nout = intensity*(d1+d2) + nin;
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





  template < int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<unsigned char,PF_COLORSPACE_RGB,CHMIN,CHMAX,true,OP_PAR>
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





  template < int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<unsigned short int,PF_COLORSPACE_RGB,CHMIN,CHMAX,true,OP_PAR>
  {
    CurvesPar* par;
    int pos;
    unsigned short int* pp;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(unsigned short int**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, unsigned short int* pout)
    {
      pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        //if(false&&x==0)
          //std::cout<<"CurvesProc::process(): cvec16["<<i<<"]["<<pp[pos]<<"]="<<par->cvec16[i][pp[pos]]<<std::endl;
        pout[pos] = (unsigned short int)(intensity*par->cvec16[i][pp[pos]] + pp[pos]);
        //std::cout<<"pp[pos]="<<pp[pos]<<"  cvec16[i][pp[pos]]="
        //    <<par->cvec16[i][pp[pos]]<<"  pout[pos]="<<pout[pos]<<" ("<<((float)pout[pos])/65535<<")"<<std::endl;
      }
    }
  };





  template < int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<float,PF_COLORSPACE_RGB,CHMIN,CHMAX,false,OP_PAR>
  {
    CurvesPar* par;
    int pos;
    float nin, nout, d1, d2;
    float* pp;
    unsigned short int idx;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(float**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, float* pout)
    {
      //std::cout<<"CurvesProc::process(float) called in non_preview mode"<<std::endl;
      pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        from_float( CLIPF(pp[pos]), idx );
        pout[pos] = (intensity*par->cvec16[i][idx]/65535 + CLIPF(pp[pos]));
        /*
        nin = pp[pos];
        d1 = par->scvec[i]->get().get_delta( nin );
        d2 = par->scvec[3]->get().get_delta( nin );
        nout = intensity*(d1+d2) + nin;
        if( nout > 1 ) nout = 1;
        else if ( nout < 0 ) nout = 0;
        pout[pos] = nout;
        //std::cout<<"pp[pos]="<<pp[pos]<<"  d2="<<d2<<"  pout[pos]="<<pout[pos]<<std::endl;
        */
      }
    }
  };





  template < int CHMIN, int CHMAX, class OP_PAR >
  class CurvesProc<float,PF_COLORSPACE_RGB,CHMIN,CHMAX,true,OP_PAR>
  {
    CurvesPar* par;
    int pos;
    float nin, nout, d1, d2;
    float* pp;
    unsigned short int idx;
  public:
    CurvesProc(CurvesPar* p): par(p) {}

    void process(float**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, float* pout)
    {
      //std::cout<<"CurvesProc::process(float) called in preview mode"<<std::endl;
      pp = p[first];
      pos = x;
      for(int i = CHMIN; i <= CHMAX; i++, pos++) {
        //std::cout<<"CurvesProc::process(): pp[pos]="<<pp[pos]<<std::endl;
        from_float( CLIPF(pp[pos]), idx );
        //std::cout<<"CurvesProc::process(): idx="<<idx<<std::endl;
        //if(false&&x==0)
          //std::cout<<"CurvesProc::process(): cvec16["<<i<<"]["<<idx<<"]="<<par->cvec16[i][idx]/65535<<std::endl;
        pout[pos] = (intensity*par->cvec16[i][idx]/65535 + CLIPF(pp[pos]));
        //std::cout<<"pp[pos]="<<pp[pos]<<"  cvec16[i][pp[pos]]="
        //    <<par->cvec16[i][pp[pos]]<<"  pout[pos]="<<pout[pos]<<" ("<<((float)pout[pos])/65535<<")"<<std::endl;
      }
    }
  };


