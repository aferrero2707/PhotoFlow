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
#define array2D PF::PixelMatrix

#include "../rt/rtengine/alignedbuffer.h"
#include "../rt/rtengine/rescale.h"
#include "../rt/rtengine/boxblur.h"


#if 0
#  define DEBUG_DUMP(arr)                                                 \
    do {                                                                \
        Imagefloat im(arr.width(), arr.height());                      \
        const char *out = "/tmp/" #arr ".tif";                     \
        for (int y = 0; y < im.getHeight(); ++y) {                      \
            for (int x = 0; x < im.getWidth(); ++x) {                   \
                im.r(y, x) = im.g(y, x) = im.b(y, x) = arr[y][x] * 65535.f; \
            }                                                           \
        }                                                               \
        im.saveTIFF(out, 16);                                           \
    } while (false)
#else
#  define DEBUG_DUMP(arr)
#endif

using namespace rtengine;


//#define GF_DEBUG


namespace {


template<class T> void gf_boxblur(PF::PixelMatrix<T> &_src, PF::PixelMatrix<float>& dst, int radius, int border=0)
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

#ifdef GF_DEBUG
  std::cout<<"W="<<W<<"  H="<<H<<std::endl;
#endif

  // create and initialize column sum vector
#ifdef GF_DEBUG
  std::cout<<"Column vectors initialization"<<std::endl;
#endif
  T* csum = new T[W];
  for(int c = border; c < W-border; c++) {
    csum[c] = 0;
    for(int r = border; r < (boxsz+border-1); r++) {
      csum[c] += src[r][c];
#ifdef GF_DEBUG
      std::cout<<"  src["<<r<<"]["<<c<<"]="<<src[r][c]<<"  csum["<<c<<"]="<<csum[c]<<std::endl;
#endif
    }
  }

  // loop on rows, leaving a band of size radius above and below
  for(int r = radius+border; r < (H-radius-border); r++) {

#ifdef GF_DEBUG
    std::cout<<"Processing row="<<r<<std::endl;
#endif
    // add the leftmost boxsz-1 values of row r+radius
    for(int c = border, rr=r+radius; c < (boxsz+border-1); c++) {
      csum[c] += src[rr][c];
#ifdef GF_DEBUG
      std::cout<<"  src["<<rr<<"]["<<c<<"]="<<src[rr][c]<<"  csum["<<c<<"]="<<csum[c]<<std::endl;
#endif
    }

    // initialize the box sum
    T sum = 0;
    for(int c = border; c < (boxsz+border-1); c++) {
      sum += csum[c];
#ifdef GF_DEBUG
      std::cout<<"  csum["<<c<<"]="<<csum[c]<<"  sum="<<sum<<std::endl;
#endif
    }

    // loop on columns, leaving a band of size radius at left and right
    for(int c = radius+border; c < (W-radius-border); c++) {

#ifdef GF_DEBUG
      std::cout<<"Processing column="<<c<<std::endl;
#endif
      // update the column sum at c+radius
      csum[c+radius] += src[r+radius][c+radius];
#ifdef GF_DEBUG
      std::cout<<"  src["<<r+radius<<"]["<<c+radius<<"]="<<src[r+radius][c+radius]<<"  csum["<<c+radius<<"]="<<csum[c+radius]<<std::endl;
#endif
      // add the column at c+radius
      sum += csum[c+radius];
#ifdef GF_DEBUG
      std::cout<<"  csum["<<c+radius<<"]="<<csum[c+radius]<<"  sum="<<sum<<std::endl;
#endif

      // update the destination value
      dst[r][c] = sum / scale;
#ifdef GF_DEBUG
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


int calculate_subsampling(int w, int h, int r)
{
    if (r == 1) {
        return 1;
    }

    //if (max(w, h) <= 600) {
    //    return 1;
    //}

    for (int s = 5; s > 0; --s) {
        if (r % s == 0) {
            return s;
        }
    }

    return LIM(r / 2, 2, 4);
}

} // namespace


void PF::guidedFilter(const PF::PixelMatrix<float> &guide, const PF::PixelMatrix<float> &src,
    PF::PixelMatrix<float> &dst, int r, float epsilon, int subsampling)
{
  bool multithread = false;
  const int W = src.width();
  const int H = src.height();

  //subsampling = 1;
  if (subsampling <= 0) {
      subsampling = calculate_subsampling(W, H, r);
      //std::cout<<"guidedFilter: W="<<W<<"  H="<<H<<"  r="<<r<<"  subsampling="<<subsampling<<std::endl;
  }

  enum Op { MUL, DIVEPSILON, ADD, SUB, ADDMUL, SUBMUL };

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
  const array2D<float> &I = guide;
  const array2D<float> &p = src;
  array2D<float> &q = dst;

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
          gf_boxblur(s, d, rad, border);
      };

  const auto f_subsample =
      [=](array2D<float> &d, const array2D<float> &s) -> void
      {
          rescaleBilinear(s, d, multithread);
      };

  const auto f_mean = f_mean2;

  const auto f_upsample = f_subsample;

  //return;

  const int w = W / subsampling;
  const int h = H / subsampling;

  array2D<float> I1; //(w, h);
  array2D<float> p1; //(w, h);

  if(subsampling > 1) {
    I1.resize(w, h);
    p1.resize(w, h);

    f_subsample(I1, I);
    f_subsample(p1, p);

    //f_upsample(q, p1);
    //std::cout<<"p.width()="<<p.width()<<"  "<<"p1.width()="<<p1.width()<<"  "<<"q.width()="<<q.width()<<"  "<<std::endl;
    //return;
  } else {
    I1 = I;
    p1 = p;
  }
  //return;

  //array2D<float> I1(I);
  //printf("After I1(I): I1=%f  I=%f\n", I1[0][0], I[0][0]); //getchar();
  //array2D<float> p1(p);
  //printf("After p1(p): p1=%f  p=%f\n", p1[0][0], p[0][0]); //getchar();

  DEBUG_DUMP(I);
  DEBUG_DUMP(p);
  DEBUG_DUMP(I1);
  DEBUG_DUMP(p1);

  float r1 = float(r) / subsampling;
  int border = 0;

  array2D<float> meanI(w, h);
  f_mean(meanI, I1, r1, 0);
  DEBUG_DUMP(meanI);
#ifdef GF_DEBUG
  printf("After f_mean(meanI, I1, r1): meanI=%f  I1=%f  r1=%f\n", meanI[0][0], I1[0][0], r1); //getchar();
#endif

  array2D<float> meanp(w, h);
  f_mean(meanp, p1, r1, 0);
  DEBUG_DUMP(meanp);
#ifdef GF_DEBUG
  printf("After f_mean(meanp, p1, r1): p1=%f  r1=%f\n", p1[0][0], r1); //getchar();
#endif
  //q = meanp;
  //return;

  //array2D<float> &corrIp = p1;
  array2D<float> corrIp(p1);
  apply(MUL, 0, corrIp, I1, p1);
#ifdef GF_DEBUG
  printf("After apply(MUL, corrIp, I1, p1): corrIp=%f I1=%f p1=%f\n", corrIp[0][0], I1[0][0], p1[0][0]); //getchar();
#endif
  f_mean(corrIp, corrIp, r1, 0);
#ifdef GF_DEBUG
  printf("After f_mean(corrIp, corrIp, r1): corrIp=%f  r1=%f\n", corrIp[0][0], r1); //getchar();
#endif
  DEBUG_DUMP(corrIp);

  array2D<float> &corrI = I1;
  apply(MUL, 0, corrI, I1, I1);
  f_mean(corrI, corrI, r1, 0);
  DEBUG_DUMP(corrI);
#ifdef GF_DEBUG
  printf("After apply(MUL, corrI, I1, I1): corrI=%f  I1=%f\n", corrI[0][0], I1[0][0]); //getchar();
#endif

  array2D<float> &varI = corrI;
  apply(SUBMUL, r1, varI, meanI, meanI, corrI);
  DEBUG_DUMP(varI);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, varI, meanI, meanI, corrI): varI=%f  meanI=%f  corrI=%f\n",
      varI[0][0], meanI[0][0], corrI[0][0]); //getchar();
#endif

  array2D<float> &covIp = corrIp;
  apply(SUBMUL, r1, covIp, meanI, meanp, corrIp);
  DEBUG_DUMP(covIp);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, covIp, meanI, meanp, corrIp)\n"); //getchar();
#endif

  array2D<float> &a = varI;
  apply(DIVEPSILON, r1, a, covIp, varI);
  DEBUG_DUMP(a);
#ifdef GF_DEBUG
  printf("After apply(DIVEPSILON, a, covIp, varI): a=%f  covIp=%f  varI=%f\n",
      a[0][0], covIp[0][0], varI[0][0]); //getchar();
#endif

  array2D<float> &b = covIp;
  apply(SUBMUL, r1, b, a, meanI, meanp);
  DEBUG_DUMP(b);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, b, a, meanI, meanp): b=%f  a=%f  meanI=%f  meanp=%f\n",
      b[0][0], a[0][0], meanI[0][0], meanp[0][0]); //getchar();
#endif

  array2D<float> &meana = a;
  f_mean(meana, a, r1, r1);
  DEBUG_DUMP(meana);

  array2D<float> &meanb = b;
  f_mean(meanb, b, r1, r1);
  DEBUG_DUMP(meanb);

  if( subsampling > 1 ) {
    array2D<float> meanA(W, H);
    f_upsample(meanA, meana);
    DEBUG_DUMP(meanA);

    array2D<float> meanB(W, H);
    f_upsample(meanB, meanb);
    DEBUG_DUMP(meanB);

    apply(ADDMUL, r*2, q, meanA, I, meanB);
    DEBUG_DUMP(q);
  } else {
    array2D<float>& meanA = meana;
    DEBUG_DUMP(meanA);

    array2D<float>& meanB = meanb;
    DEBUG_DUMP(meanB);

    apply(ADDMUL, r*2, q, meanA, I, meanB);
    DEBUG_DUMP(q);
  }
#ifdef GF_DEBUG
  printf("After apply(ADDMUL, q, meana, I, meanb): q=%f  meana=%f  I=%f  meanb=%f\n\n",
      q[0][0], meana[0][0], I[0][0], meanb[0][0]); //getchar();
#endif
}


void PF::guidedFilter(const PF::PixelMatrix<float> &src,
    PF::PixelMatrix<float> &dst, int r, float epsilon, int subsampling)
{
  bool multithread = false;
  const int W = src.width();
  const int H = src.height();

  //subsampling = 1;
  if (subsampling <= 0) {
      subsampling = calculate_subsampling(W, H, r);
      //std::cout<<"guidedFilter: W="<<W<<"  H="<<H<<"  r="<<r<<"  subsampling="<<subsampling<<std::endl;
  }

  enum Op { MUL, DIVEPSILON, ADD, SUB, ADDMUL, SUBMUL };

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
                      //std::cout<<"["<<x<<"]["<<y<<"]: r = aa / (bb + epsilon): "<<r<<" = "<<aa<<" / ("<<bb<<"+"<<epsilon<<")"<<std::endl;
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
  const array2D<float> &p = src;
  array2D<float> &q = dst;

  AlignedBuffer<float> blur_buf(p.width() * p.height());
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
          gf_boxblur(s, d, rad, border);
      };

  const auto f_subsample =
      [=](array2D<float> &d, const array2D<float> &s) -> void
      {
          rescaleBilinear(s, d, multithread);
      };

  const auto f_mean = f_mean2;

  const auto f_upsample = f_subsample;

  //return;

  const int w = W / subsampling;
  const int h = H / subsampling;

  int r1 = r / subsampling;
  int border = 0;

  array2D<float> p1; //(w, h);

  if(subsampling > 1) {
    p1.resize(w, h);

    f_subsample(p1, p);

    //f_upsample(q, p1);
    //std::cout<<"p.width()="<<p.width()<<"  "<<"p1.width()="<<p1.width()<<"  "<<"q.width()="<<q.width()<<"  "<<std::endl;
    //return;
  } else {
    p1 = p;
  }
  //return;

  //array2D<float> I1(I);
  //printf("After I1(I): I1=%f  I=%f\n", I1[r1*2][r1*2], I[r1*2][r1*2]); //getchar();
  //array2D<float> p1(p);
#ifdef GF_DEBUG
  printf("After p1(p): p1=%f  p=%f\n", p1[r1*2][r1*2], p[r1*2][r1*2]); //getchar();
#endif

  DEBUG_DUMP(p);
  DEBUG_DUMP(p1);

  array2D<float> meanp(w, h);
  f_mean(meanp, p1, r1, 0);
  DEBUG_DUMP(meanp);
#ifdef GF_DEBUG
  printf("After f_mean(meanp, p1, r1): meanp=%f  p1=%f  r1=%d\n", meanp[r1*2][r1*2], p1[r1*2][r1*2], r1); //getchar();
#endif
  //q = meanp;
  //return;

  array2D<float> &corrI = p1;
  apply(MUL, 0, corrI, p1, p1);
  f_mean(corrI, corrI, r1, 0);
  DEBUG_DUMP(corrI);
#ifdef GF_DEBUG
  printf("After apply(MUL, corrI, p1, p1): corrI=%f  p1=%f\n", corrI[r1*2][r1*2], p1[r1*2][r1*2]); //getchar();
#endif

  array2D<float> &varI = corrI;
  apply(SUBMUL, r1, varI, meanp, meanp, corrI);
  DEBUG_DUMP(varI);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, varI, meanp, meanp, corrI): varI=%f  meanp=%f  corrI=%f\n",
      varI[r1*2][r1*2], meanp[r1*2][r1*2], corrI[r1*2][r1*2]); //getchar();
#endif

  array2D<float> &a = varI;
  apply(DIVEPSILON, r1, a, varI, varI);
  DEBUG_DUMP(a);
#ifdef GF_DEBUG
  printf("After apply(DIVEPSILON, a, VarI, varI): a=%f  varI=%f\n",
      a[r1*2][r1*2], varI[r1*2][r1*2]); //getchar();
#endif

  array2D<float> &b = meanp;
  apply(SUBMUL, r1, b, a, meanp, meanp);
  DEBUG_DUMP(b);
#ifdef GF_DEBUG
  printf("After apply(SUBMUL, b, a, meanp, meanp): b=%f  a=%f  meanp=%f\n",
      b[r1*2][r1*2], a[r1*2][r1*2], meanp[r1*2][r1*2]); //getchar();
#endif

  array2D<float> &meana = a;
  f_mean(meana, a, r1, r1);
  DEBUG_DUMP(meana);
#ifdef GF_DEBUG
  printf("After f_mean(meana, a, r1, r1): meana=%f  a=%f  r1=%d\n", meana[r1*2][r1*2], a[r1*2][r1*2], r1); //getchar();
#endif

  array2D<float> &meanb = b;
  f_mean(meanb, b, r1, r1);
  DEBUG_DUMP(meanb);
#ifdef GF_DEBUG
  printf("After f_mean(meanb, b, r1, r1): meanb=%f  b=%f  r1=%d\n", meanb[r1*2][r1*2], b[r1*2][r1*2], r1); //getchar();
#endif

  if( subsampling > 1 ) {
    array2D<float> meanA(W, H);
    f_upsample(meanA, meana);
    DEBUG_DUMP(meanA);

    array2D<float> meanB(W, H);
    f_upsample(meanB, meanb);
    DEBUG_DUMP(meanB);

    apply(ADDMUL, r*2, q, meanA, p, meanB);
    DEBUG_DUMP(q);
#ifdef GF_DEBUG
    printf("After apply(ADDMUL, q, meanA, p, meanB): q=%f  meanA=%f  p=%f  meanB=%f\n\n",
        q[r*2][r*2], meanA[r*2][r*2], p[r*2][r*2], meanB[r*2][r*2]); //getchar();
#endif
  } else {
    array2D<float>& meanA = meana;
    DEBUG_DUMP(meanA);

    array2D<float>& meanB = meanb;
    DEBUG_DUMP(meanB);

    apply(ADDMUL, r*2, q, meanA, p, meanB);
    DEBUG_DUMP(q);
#ifdef GF_DEBUG
    printf("After apply(ADDMUL, q, meanA, p, meanB): q=%f  meanA=%f  p=%f  meanB=%f\n\n",
        q[r*2][r*2], meanA[r*2][r*2], p[r*2][r*2], meanB[r*2][r*2]); //getchar();
#endif
  }
}


PF::GuidedFilterPar::GuidedFilterPar():
PaddedOpPar(),
radius("radius",this,10.0),
threshold("threshold",this,0.075),
subsampling("subsampling",this,1),
convert_to_perceptual("convert_to_perceptual",this,true)
{
  set_type("guided_filter" );

  set_default_name( _("guided filter") );
}


bool PF::GuidedFilterPar::needs_caching()
{
  return false;
}




void PF::GuidedFilterPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  double radius2 = radius.get();
  double subsampling2 = subsampling.get();
  for( unsigned int l = 1; l <= level; l++ ) {
    radius2 /= 2;
    subsampling2 /= 2;
  }
  subsampling_real = (subsampling2 < 1) ? 1 : static_cast<int>(floor(subsampling2));

  int padding = radius2 * 2 + subsampling_real;

  //std::cout<<"GuidedFilterPar::compute_padding: level="<<level
  //    <<"  subsampling="<<subsampling.get()<<"  subsampling_real="<<subsampling_real
  //    <<"  radius="<<radius.get()<<"  radius2="<<radius2<<"  padding="<<padding<<std::endl;

  set_padding( padding, id );
}


VipsImage* PF::GuidedFilterPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  icc_data = NULL;
  if(in.size()>0 && in[0]) icc_data = PF::get_icc_profile( in[0] );

  double radius2 = radius.get();
  double subsampling2 = subsampling.get();
  for( unsigned int l = 1; l <= level; l++ ) {
    radius2 /= 2;
    subsampling2 /= 2;
  }

  radius_real = radius2;
  subsampling_real = (subsampling2 < 1) ? 1 : static_cast<int>(floor(subsampling2));

  int padding = radius_real * 2 + subsampling_real;
  set_padding( padding, 0 );

#ifdef GF_DEBUG
  std::cout<<"GuidedFilterPar::build: radius="<<radius_real<<"  threshold="<<threshold.get()
      <<"  in.size()="<<in.size()<<std::endl;
#endif
  VipsImage* out = PF::PaddedOpPar::build( in, first, imap, omap, level );
  //std::cout<<"GuidedFilterPar::build: out="<<out<<std::endl;
  return out;
}


PF::ProcessorBase* PF::new_guided_filter()
{ return( new PF::Processor<PF::GuidedFilterPar,PF::GuidedFilterProc>() ); }

