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

void PF::guidedFilter(const PF::PixelMatrix<float> &guide, const PF::PixelMatrix<float> &src,
    PF::PixelMatrix<float> &dst, int r, float epsilon, int subsampling)
{
  bool multithread = false;
  const int W = src.width();
  const int H = src.height();

  //if (subsampling <= 0) {
  //    subsampling = calculate_subsampling(W, H, r);
  //}

  enum Op { MUL, DIVEPSILON, ADD, SUB, ADDMUL, SUBMUL };

  const auto apply =
      [=](Op op, array2D<float> &res, const array2D<float> &a, const array2D<float> &b, const array2D<float> &c=array2D<float>()) -> void
      {
          const int w = res.width();
          const int h = res.height();

#ifdef _OPENMP
          #pragma omp parallel for if (multithread)
#endif
          for (int y = 0; y < h; ++y) {
              for (int x = 0; x < w; ++x) {
                  float r;
                  float aa = a[y][x];
                  float bb = b[y][x];
                  switch (op) {
                  case MUL:
                      r = aa * bb;
                      break;
                  case DIVEPSILON:
                      r = aa / (bb + epsilon);
                      break;
                  case ADD:
                      r = aa + bb;
                      break;
                  case SUB:
                      r = aa - bb;
                      break;
                  case ADDMUL:
                      r = aa * bb + c[y][x];
                      break;
                  case SUBMUL:
                      r = c[y][x] - (aa * bb);
                      break;
                  default:
                      assert(false);
                      r = 0;
                      break;
                  }
                  res[y][x] = r;
              }
          }
      };

  // use the terminology of the paper (Algorithm 2)
  const array2D<float> &I = guide;
  const array2D<float> &p = src;
  array2D<float> &q = dst;

  AlignedBuffer<float> blur_buf(I.width() * I.height());
  const auto f_mean =
      [&](array2D<float> &d, array2D<float> &s, int rad) -> void
      {
          rad = LIM(rad, 0, (min(s.width(), s.height()) - 1) / 2 - 1);
          float **src = s;
          float **dst = d;
          boxblur<float, float>(src, dst, blur_buf.data, rad, rad, s.width(), s.height());
      };

  const auto f_subsample =
      [=](array2D<float> &d, const array2D<float> &s) -> void
      {
          rescaleBilinear(s, d, multithread);
      };

  const auto f_upsample = f_subsample;

  const int w = W / subsampling;
  const int h = H / subsampling;

  array2D<float> I1(w, h);
  array2D<float> p1(w, h);

  f_subsample(I1, I);
  f_subsample(p1, p);

  DEBUG_DUMP(I);
  DEBUG_DUMP(p);
  DEBUG_DUMP(I1);
  DEBUG_DUMP(p1);

  float r1 = float(r) / subsampling;

  array2D<float> meanI(w, h);
  f_mean(meanI, I1, r1);
  DEBUG_DUMP(meanI);

  array2D<float> meanp(w, h);
  f_mean(meanp, p1, r1);
  DEBUG_DUMP(meanp);

  array2D<float> &corrIp = p1;
  apply(MUL, corrIp, I1, p1);
  f_mean(corrIp, corrIp, r1);
  DEBUG_DUMP(corrIp);

  array2D<float> &corrI = I1;
  apply(MUL, corrI, I1, I1);
  f_mean(corrI, corrI, r1);
  DEBUG_DUMP(corrI);

  array2D<float> &varI = corrI;
  apply(SUBMUL, varI, meanI, meanI, corrI);
  DEBUG_DUMP(varI);

  array2D<float> &covIp = corrIp;
  apply(SUBMUL, covIp, meanI, meanp, corrIp);
  DEBUG_DUMP(covIp);

  array2D<float> &a = varI;
  apply(DIVEPSILON, a, covIp, varI);
  DEBUG_DUMP(a);

  array2D<float> &b = covIp;
  apply(SUBMUL, b, a, meanI, meanp);
  DEBUG_DUMP(b);

  array2D<float> &meana = a;
  f_mean(meana, a, r1);
  DEBUG_DUMP(meana);

  array2D<float> &meanb = b;
  f_mean(meanb, b, r1);
  DEBUG_DUMP(meanb);

  array2D<float> meanA(W, H);
  f_upsample(meanA, meana);
  DEBUG_DUMP(meanA);

  array2D<float> meanB(W, H);
  f_upsample(meanB, meanb);
  DEBUG_DUMP(meanB);

  apply(ADDMUL, q, meanA, I, meanB);
  DEBUG_DUMP(q);
}


PF::GuidedFilterPar::GuidedFilterPar():
PaddedOpPar(),
radius("radius",this,10.0),
threshold("threshold",this,0.075)
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
  for( unsigned int l = 1; l <= level; l++ )
    radius2 /= 2;

  int padding = radius2 * 2 + 1;

  set_padding( padding, id );
}


VipsImage* PF::GuidedFilterPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  icc_data = NULL;
  if(in.size()>0 && in[0]) icc_data = PF::get_icc_profile( in[0] );

  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ )
    radius2 /= 2;

  radius_real = radius2;

  std::cout<<"GuidedFilterPar::build: radius="<<radius_real<<"  threshold="<<threshold.get()<<std::endl;
  VipsImage* out = PF::PaddedOpPar::build( in, first, imap, omap, level );
  //std::cout<<"GuidedFilterPar::build: out="<<out<<std::endl;
  return out;
}


PF::ProcessorBase* PF::new_guided_filter()
{ return( new PF::Processor<PF::GuidedFilterPar,PF::GuidedFilterProc>() ); }

