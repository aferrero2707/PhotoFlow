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

}


void PF::eusm_gf(const PF::PixelMatrix<float> &src, PF::PixelMatrix<float> &dst_a,
    PF::PixelMatrix<float> &dst_mean, int r, float epsilon)
{
  bool multithread = false;
  const int W = src.width();
  const int H = src.height();

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

  array2D<float>& meanI = dst_mean;
  f_mean(meanI, I1, r1, 0);
  DEBUG_DUMP(meanI);
  //printf("After f_mean(meanI, I1, r1): meanI=%f  I1=%f  r1=%f\n", meanI[0][0], I1[0][0], r1); //getchar();

  array2D<float> &corrI = I1;
  apply(MUL, 0, corrI, I1, I1);
  f_mean(corrI, corrI, r1, 0);
  DEBUG_DUMP(corrI);
  //printf("After apply(MUL, corrI, I1, I1): corrI=%f  I1=%f\n", corrI[0][0], I1[0][0]); //getchar();

  array2D<float> &varI = corrI;
  apply(SUBMUL, r1, varI, meanI, meanI, corrI);
  DEBUG_DUMP(varI);
  //printf("After apply(SUBMUL, varI, meanI, meanI, corrI): varI=%f  meanI=%f  corrI=%f\n",
  //    varI[0][0], meanI[0][0], corrI[0][0]); //getchar();

  array2D<float> &a = varI;
  apply(DIVEPSILON, r1, a, varI, varI);
  DEBUG_DUMP(a);
  //printf("After apply(DIVEPSILON, a, covIp, varI): a=%f  covIp=%f  varI=%f\n",
  //    a[0][0], covIp[0][0], varI[0][0]); //getchar();

  array2D<float> &meana = dst_a;
  f_mean(meana, a, r1, r1);
  DEBUG_DUMP(meana);
}

class EUSMLogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
  bool linear;
public:
  EUSMLogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5), linear(false) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }

  bool do_linear() { return linear; }
  void set_linear(bool l) { linear = l; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("EUSMLogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("EUSMLogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("EUSMLogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("EUSMLogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class EUSMLogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class EUSMLogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    EUSMLogLumiPar* opar = dynamic_cast<EUSMLogLumiPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;
    bool linear = opar->do_linear();

    const float bias = 1.0f/profile->perceptual2linear(opar->get_anchor());

    float L, pL;
    float* pin;
    float* pout;
    int x, y, R;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < width; x++, pin+=3, pout++ ) {
        L = profile->get_lightness(pin[0], pin[1], pin[2]);
        //L *= bias;
        if( !linear ) {
          pL = (L>1.0e-16) ? xlogf( L ) : xlogf( 1.0e-16 );
          pout[0] = pL;
        } else {
          L /= 0.05;
          pL = (L > 1) ? xlogf( L ) : (L - 1);
          pout[0] = pL;
        }
      }
    }
  }
};





PF::EnhancedUnsharpMaskPar::EnhancedUnsharpMaskPar():
  PaddedOpPar(),
  do_sum("do_sum",this,true),
  linear("linear",this,false),
  amount("amount",this,1),
  radius("radius",this,3),
  threshold_l("threshold_l",this,0.005),
  threshold_h("threshold_h",this,0.02)
{
  loglumi = new PF::Processor<EUSMLogLumiPar,EUSMLogLumiProc>();
  guided[0] = new_guided_filter();
  guided[1] = new_guided_filter();

  set_type("enhanced_usm" );

  set_default_name( _("enhanced usm") );
}



void PF::EnhancedUnsharpMaskPar::propagate_settings()
{
  for(int gi = 0; gi < 2; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    guidedpar->propagate_settings();
  }
}



void PF::EnhancedUnsharpMaskPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  double radius2 = radius.get();
  for( unsigned int l = 1; l <= level; l++ ) {
    radius2 /= 2;
  }
  int padding = radius2 * 2;

  //std::cout<<"GuidedFilterPar::compute_padding: level="<<level
  //    <<"  subsampling="<<subsampling.get()<<"  subsampling_real="<<subsampling_real
  //    <<"  radius="<<radius.get()<<"  radius2="<<radius2<<"  padding="<<padding<<std::endl;

  /*
  threshold_real_l = threshold_l.get();
  threshold_real_h = threshold_h.get();
  if( get_linear() ) {
    threshold_real_l /= 1;
    threshold_real_h /= 1;
  }

  int padding = 0;
  int rv[2] = { radius.get(), radius.get() };
  float tv[2] = { threshold_real_l, threshold_real_h };

  for(int gi = 0; gi < 1; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
#ifndef NDEBUG
    std::cout<<"EnhancedUnsharpMaskPar::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
#endif
    if(rv[gi] == 0) break;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold( tv[gi] );
    int subsampling = 1;
    guidedpar->set_subsampling(subsampling);
    guidedpar->propagate_settings();
    guidedpar->compute_padding(full_res, id, level);

    padding = guidedpar->get_padding(id);
  }
  */
  set_padding( padding, id );
}



VipsImage* PF::EnhancedUnsharpMaskPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;


  std::vector<VipsImage*> in2;
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

  int padding = radius_real * 2;
  set_padding( padding, 0 );

#ifdef GF_DEBUG
  std::cout<<"GuidedFilterPar::build: radius="<<radius_real<<"  threshold="<<threshold.get()
      <<"  in.size()="<<in.size()<<std::endl;
#endif
  VipsImage* out2 = PF::PaddedOpPar::build( in, first, imap, omap, level );
  //std::cout<<"GuidedFilterPar::build: out="<<out<<std::endl;
  return out2;


  EUSMLogLumiPar* logpar = dynamic_cast<EUSMLogLumiPar*>( loglumi->get_par() );
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( 0.5 );
    logpar->set_linear( get_linear() );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  } else {
    logimg = in[0];
    PF_REF(logimg, "EnhancedUnsharpMaskPar::build: in[0] ref");
  }

#ifndef NDEBUG
  std::cout<<"EnhancedUnsharpMaskPar::build(): logimg="<<logimg<<std::endl;
#endif

  if( !logimg ) return NULL;




  int rv[2] = { radius.get(), radius.get() };
  float tv[2] = { threshold_real_l, threshold_real_h };

  //VipsImage* timg = logimg;
  VipsImage* smoothed[2] = { NULL };
#ifndef NDEBUG
  std::cout<<"EnhancedUnsharpMaskPar::build(): radius="<<radius.get()<<std::endl;
#endif

  for(int gi = 0; gi < 2; gi++) {
    std::cout<<"EnhancedUnsharpMaskPar::build(): gi="<<gi<<"  radius="<<rv[gi]<<std::endl;
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( logimg );
    guidedpar->set_format( get_format() );
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(tv[gi]);
    int subsampling = 1;
    guidedpar->set_subsampling(subsampling);
    guidedpar->set_convert_to_perceptual( false );
    //guidedpar->propagate_settings();
    //guidedpar->compute_padding(logimg, 0, level);
    VipsImage* tmpimg = logimg;
    PF_REF(tmpimg, "EnhancedUnsharpMaskPar::build(): tmpimg ref");
    in2.clear();
    in2.push_back( tmpimg );
    smoothed[gi] = guidedpar->build( in2, first, NULL, NULL, level );
    PF_UNREF(tmpimg, "EnhancedUnsharpMaskPar::build(): tmpimg unref");

    //#ifndef NDEBUG
    std::cout<<"EnhancedUnsharpMaskPar::build(): gi="<<gi<<"  radius="
        <<rv[gi]<<"  padding="<<guidedpar->get_padding(0)
        <<"  logimg="<<logimg<<"  smoothed="<<smoothed[gi]<<std::endl;
    //#endif

    //if( smoothed[gi] ) {
    //  PF_UNREF(logimg, "EnhancedUnsharpMaskPar::build(): timg unref");
    //}
  }


  if( !smoothed[0] || !smoothed[1] ) {
    std::cout<<"EnhancedUnsharpMaskPar::build(): null smoothed image"<<std::endl;
    PF_REF( in[0], "EnhancedUnsharpMaskPar::build(): null smoothed image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed[0]);
  in2.push_back(smoothed[1]);
  in2.push_back(logimg);
  in2.push_back(in[0]);
  VipsImage* out = PF::PaddedOpPar::build( in2, 0, imap, omap, level );
  PF_UNREF( smoothed[0], "EnhancedUnsharpMaskPar::build() smoothed[0] unref" );
  PF_UNREF( smoothed[1], "EnhancedUnsharpMaskPar::build() smoothed[1] unref" );
  PF_UNREF( logimg, "EnhancedUnsharpMaskPar::build() logimg unref" );
  set_image_hints( in[0] );

  return out;
}


PF::ProcessorBase* PF::new_enhanced_usm()
{ return new PF::Processor<PF::EnhancedUnsharpMaskPar,PF::EnhancedUnsharpMaskProc>(); }
