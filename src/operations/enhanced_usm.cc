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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/processor_imp.hh"
#include "guided_filter.hh"
#include "enhanced_usm.hh"

#define array2D PF::PixelMatrix
#define DEBUG_DUMP(arr)

#include "../rt/rtengine/alignedbuffer.h"
#include "../rt/rtengine/rescale.h"
#include "../rt/rtengine/boxblur.h"


using namespace rtengine;

namespace {


template<class T> void eusm_gf_boxblur(PF::PixelMatrix<T> &_src, PF::PixelMatrix<float>& dst, int radius, int border=0)
{
  const int W = _src.width();
  const int H = _src.height();
  const int boxsz = radius*2 + 1;
  const float scale = boxsz*boxsz;

  PF::PixelMatrix<T>* ppm = &_src;
  PF::PixelMatrix<T> tbuf;
  if( _src.get_rows() == dst.get_rows() ) {
    tbuf = _src;
    ppm = &tbuf;
  }
  PF::PixelMatrix<T>& src = *ppm;

#ifdef GF_DEBUG2
  std::cout<<"W="<<W<<"  H="<<H<<std::endl;
#endif

  // create and initialize column sum vector
#ifdef GF_DEBUG2
  std::cout<<"Column vectors initialization"<<std::endl;
#endif
  T* csum = new T[W];
  for(int c = border; c < W-border; c++) {
    csum[c] = 0;
    for(int r = border; r < (boxsz+border-1); r++) {
      csum[c] += src[r][c];
#ifdef GF_DEBUG2
      std::cout<<"  src["<<r<<"]["<<c<<"]="<<src[r][c]<<"  csum["<<c<<"]="<<csum[c]<<std::endl;
#endif
    }
  }

  // loop on rows, leaving a band of size radius above and below
  for(int r = radius+border; r < (H-radius-border); r++) {

#ifdef GF_DEBUG2
    std::cout<<"Processing row="<<r<<std::endl;
#endif
    // add the leftmost boxsz-1 values of row r+radius
    for(int c = border, rr=r+radius; c < (boxsz+border-1); c++) {
      csum[c] += src[rr][c];
#ifdef GF_DEBUG2
      std::cout<<"  src["<<rr<<"]["<<c<<"]="<<src[rr][c]<<"  csum["<<c<<"]="<<csum[c]<<std::endl;
#endif
    }

    // initialize the box sum
    T sum = 0;
    for(int c = border; c < (boxsz+border-1); c++) {
      sum += csum[c];
#ifdef GF_DEBUG2
      std::cout<<"  csum["<<c<<"]="<<csum[c]<<"  sum="<<sum<<std::endl;
#endif
    }

    // loop on columns, leaving a band of size radius at left and right
    for(int c = radius+border; c < (W-radius-border); c++) {

#ifdef GF_DEBUG2
      std::cout<<"Processing column="<<c<<std::endl;
#endif
      // update the column sum at c+radius
      csum[c+radius] += src[r+radius][c+radius];
#ifdef GF_DEBUG2
      std::cout<<"  src["<<r+radius<<"]["<<c+radius<<"]="<<src[r+radius][c+radius]<<"  csum["<<c+radius<<"]="<<csum[c+radius]<<std::endl;
#endif
      // add the column at c+radius
      sum += csum[c+radius];
#ifdef GF_DEBUG2
      std::cout<<"  csum["<<c+radius<<"]="<<csum[c+radius]<<"  sum="<<sum<<std::endl;
#endif

      // update the destination value
      dst[r][c] = sum / scale;
#ifdef GF_DEBUG2
      std::cout<<"dst["<<r<<"]["<<c<<"] = "<<sum<<" / "<<scale<<std::endl;
#endif

      // remove the column at c-radius from the sum
      sum -= csum[c-radius];

      // remove the value at (r-radius, c-radius) from the column a c-radius
      csum[c-radius] -= src[r-radius][c-radius];
    }

    // loop on columns finished.
    // remove the values at r-radius from the csums from W-boxsz+1 to W-1
    for(int c = (W-border-boxsz+1); c < W-border; c++) {
      csum[c] -= src[r-radius][c];
    }
  }

  delete[] csum;
}

}

void PF::eusm_gf(const PF::PixelMatrix<float> &src, PF::PixelMatrix<float> &dst_a, PF::PixelMatrix<float> &dst_b,
    PF::PixelMatrix<float> &dst_mean, int r, float epsilon, bool invert)
{
  bool multithread = false;
  const int W = src.width();
  const int H = src.height();

  enum Op { MUL, DIVEPSILON, DIVEPSILONINV, ADD, SUB, ADDMUL, SUBMUL };

  const auto apply =
      [=](Op op, int border, array2D<float> &res, const array2D<float> &a, const array2D<float> &b, const array2D<float> &c=array2D<float>()) -> void
      {
          const int w = res.width()-border;
          const int h = res.height()-border;

#ifdef _OPENMP
          #pragma omp parallel for if (multithread)
#endif
          for (int y = border; y < h; ++y) {
              for (int x = border; x < w; ++x) {
                  float r;
                  float aa = a[y][x];
                  float bb = b[y][x];
                  switch (op) {
                  case MUL:
                      r = aa * bb;
                      //std::cout<<"r = aa * bb: "<<r<<" = "<<aa<<" * "<<bb<<std::endl;
                      break;
                  case DIVEPSILON:
                      r = aa / (bb + epsilon);
                      //std::cout<<"r = aa / (bb + epsilon): "<<r<<" = "<<aa<<" / "<<bb+epsilon<<std::endl;
                      break;
                  case DIVEPSILONINV:
                      r = 1.0f - (aa / (bb + epsilon));
                      //std::cout<<"r = aa / (bb + epsilon): "<<r<<" = "<<aa<<" / "<<bb+epsilon<<std::endl;
                      break;
                  case ADD:
                      r = aa + bb;
                      //std::cout<<"r = aa + bb: "<<r<<" = "<<aa<<" + "<<bb<<std::endl;
                      break;
                  case SUB:
                      r = aa - bb;
                      //std::cout<<"r = aa - bb: "<<r<<" = "<<aa<<" - "<<bb<<std::endl;
                      break;
                  case ADDMUL:
                      r = aa * bb + c[y][x];
                      //std::cout<<"r = aa * bb + c[y][x]: "<<r<<" = "<<aa<<" * "<<bb<<" + "<<c[y][x]<<std::endl;
                      break;
                  case SUBMUL:
                      r = c[y][x] - (aa * bb);
                      //std::cout<<"r = c[y][x] - (aa * bb): "<<r<<" = "<<c[y][x]<<" - "<<aa<<" * "<<bb<<std::endl;
                      break;
                  default:
                      assert(false);
                      r = 0;
                      break;
                  }
                  res[y][x] = r;
                  //if( op==ADDMUL && r<-100 ) getchar();
              }
          }
      };

  // use the terminology of the paper (Algorithm 2)
  const array2D<float> &I = src;

  AlignedBuffer<float> blur_buf(I.width() * I.height());
  const auto f_mean1 =
      [&](array2D<float> &d, array2D<float> &s, int rad, int border=0) -> void
      {
          rad = LIM(rad, 0, (min(s.width(), s.height()) - 1) / 2 - 1);
          float **src = s;
          float **dst = d;
          boxblur<float, float>(src, dst, blur_buf.data, rad, rad, s.width(), s.height());
      };

  const auto f_mean2 =
      [&](array2D<float> &d, array2D<float> &s, int rad, int border=0) -> void
      {
          eusm_gf_boxblur(s, d, rad, border);
      };

  const auto f_subsample =
      [=](array2D<float> &d, const array2D<float> &s) -> void
      {
          rescaleBilinear(s, d, multithread);
      };

  const auto f_mean = f_mean2;

  const auto f_upsample = f_subsample;

  //return;

  const int w = W;
  const int h = H;

  array2D<float> I1; //(w, h);
  I1 = I;

  float r1 = float(r);
  int border = 0;

#ifdef GF_DEBUG
  printf("epsilon=%f\n", epsilon);

  printf("I1:\n");
  for (int x = r1; x < w-r1; ++x) {
    printf("%f ", I1[r1][x]);
  }
  printf("\n");
#endif

  array2D<float>& meanI = dst_mean;
  f_mean(meanI, I1, r1, 0);
  DEBUG_DUMP(meanI);
#ifdef GF_DEBUG
  printf("After f_mean(meanI, I1, r1)\n");
  for (int x = r1; x < w-r1; ++x) {
    printf("%f ", meanI[r1][x]);
  }
  printf("\n");
#endif

  array2D<float> &corrI = I1;
  apply(MUL, 0, corrI, I1, I1);
  f_mean(corrI, corrI, r1, 0);
  DEBUG_DUMP(corrI);
  //printf("After apply(MUL, corrI, I1, I1): corrI=%f  I1=%f\n", corrI[0][0], I1[0][0]); //getchar();

  array2D<float> &varI = corrI;
  apply(SUBMUL, r1, varI, meanI, meanI, corrI);
  DEBUG_DUMP(varI);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, varI, meanI, meanI, corrI)\n");
  for (int x = r1; x < w-r1; ++x) {
    printf("%f ", varI[r1][x]);
  }
  printf("\n");
#endif

  array2D<float> &a = varI;
  if(invert)
    apply(DIVEPSILONINV, r1, a, varI, varI);
  else
    apply(DIVEPSILON, r1, a, varI, varI);
  DEBUG_DUMP(a);
#ifdef GF_DEBUG
  printf("After apply(DIVEPSILON, a, covIp, varI):\n");
  for (int x = r1; x < w-r1; ++x) {
    printf("%f ", a[r1][x]);
  }
  printf("\n");
#endif

  array2D<float> &b = meanI;
  apply(SUBMUL, r1, b, a, meanI, meanI);
  DEBUG_DUMP(b);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, b, a, meanI, meanI):\n");
  for (int x = r1; x < w-r1; ++x) {
    printf("%f ", b[r1][x]);
  }
  printf("\n");
#endif

  array2D<float> &meana = dst_a;
  f_mean(meana, a, r1, r1);
#ifdef GF_DEBUG
  printf("After f_mean(meana, a, r1, r1):\n");
  for (int x = r1*2; x < w-r1*2; ++x) {
    printf("%f ", meana[r1*2][x]);
  }
  printf("\n");
#endif
  DEBUG_DUMP(meana);

  array2D<float> &meanb = dst_b;
  f_mean(meanb, b, r1, r1);
#ifdef GF_DEBUG
  printf("After f_mean(meanb, b, r1, r1):\n");
  for (int x = r1*2; x < w-r1*2; ++x) {
    printf("%f ", meanb[r1*2][x]);
  }
  printf("\n");
#endif
  DEBUG_DUMP(meanb);

  array2D<float> q(w-r*4, h-r*4);
  //apply(ADDMUL, r*2, q, meana, I, meanb);
#ifdef GF_DEBUG
  printf("After apply(ADDMUL, r*2, q, meana, I, meanb):\n");
  for (int x = r*2; x < w-r*2; ++x) {
    printf("%f ", meana[r*2][x]*I[r*2][x] + meanb[r*2][x]);
  }
  printf("\n");
  printf("\n\n");
#endif
  DEBUG_DUMP(q);
}



PF::EnhancedUnsharpMaskPar::EnhancedUnsharpMaskPar():
  PaddedOpPar(),
  show_mask("show_mask",this,true),
  linear("linear",this,false),
  amount("amount",this,1),
  radius("radius",this,3),
  threshold_l("threshold_l",this,0.005),
  threshold_h("threshold_h",this,0.02),
  nscales("nscales",this,1)
{
  set_type("enhanced_usm" );

  set_default_name( _("enhanced usm") );
}



void PF::EnhancedUnsharpMaskPar::propagate_settings()
{
}



void PF::EnhancedUnsharpMaskPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  double radius2 = radius.get();
  for( unsigned int s = 1; s < nscales.get(); s++ ) {
    radius2 *= 2;
  }
  for( unsigned int l = 1; l <= level; l++ ) {
    radius2 /= 2;
  }
  int padding = radius2 * 4;
  set_padding( padding, id );
}



VipsImage* PF::EnhancedUnsharpMaskPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;


  in_profile = PF::get_icc_profile( in[0] );

  threshold_real_l = threshold_l.get();
  threshold_real_h = threshold_h.get();
  if( get_linear() ) {
    threshold_real_l /= 1;
    threshold_real_h /= 1;
  }

  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ ) {
    radius2 /= 2;
  }
  radius_real = radius2;
  for( unsigned int s = 1; s < nscales.get(); s++ ) {
    radius2 *= 2;
  }


  int padding = radius2 * 4;
  set_padding( padding, 0 );

  VipsImage* out = PF::PaddedOpPar::build( in, first, imap, omap, level );
  return out;
}


PF::ProcessorBase* PF::new_enhanced_usm()
{ return new PF::Processor<PF::EnhancedUnsharpMaskPar,PF::EnhancedUnsharpMaskProc>(); }
