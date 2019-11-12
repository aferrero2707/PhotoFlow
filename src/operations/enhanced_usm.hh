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

#ifndef PF_ENHANCED_USM_H
#define PF_ENHANCED_USM_H

#include "../rt/rtengine/sleef.c"
#include "../base/processor.hh"
#include "padded_op.hh"

namespace PF 
{

class EnhancedUnsharpMaskPar: public PaddedOpPar
{
  Property<bool> show_mask;
  Property<bool> linear;
  Property<float> amount;
  Property<float> radius, threshold_l, threshold_h;
  Property<int> nscales;
  float radius_real, threshold_real_l, threshold_real_h;

  PF::ICCProfile* in_profile;
public:
  EnhancedUnsharpMaskPar();

  bool has_intensity() { return false; }
  bool needs_caching() {
    return true;
  }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
  void propagate_settings();

  PF::ICCProfile* get_profile() { return in_profile; }
  void set_amount( float a ) { amount.set( a ); }
  float get_amount() { return amount.get(); }
  void set_radius( float r ) { radius.set( r ); }
  float get_radius() { return radius_real; }
  void set_threshold_l( float t ) { threshold_l.set( t ); }
  float get_threshold_l() { return threshold_real_l; }
  void set_threshold_h( float t ) { threshold_h.set( t ); }
  float get_threshold_h() { return threshold_real_h; }
  void set_nscales( int s ) { nscales.set( s ); }
  int get_nscales() { return nscales.get(); }

  void set_show_mask(bool b) { show_mask.set(b); }
  bool get_show_mask() { return show_mask.get(); }

  void set_linear(bool b) { linear.set(b); }
  bool get_linear() { return linear.get(); }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};


void eusm_gf(const PixelMatrix<float> &src, PixelMatrix<float> &dst_a, PixelMatrix<float> &dst_b,
    PixelMatrix<float> &dst_mean, int r, float epsilon, bool invert);



template < OP_TEMPLATE_DEF >
class EnhancedUnsharpMaskProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
    std::cout<<"EnhancedUnsharpMaskProc::render() called."<<std::endl;
  }
};



//#define GF_DEBUG 1


template < OP_TEMPLATE_DEF_CS_SPEC >
class EnhancedUnsharpMaskProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
{
  ICCProfile* profile;
public:

  void fill_L_matrix(int rw, int rh, PixelMatrix<float>& rgbin, PixelMatrix<float>& Lout, bool log_conv)
  {
    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* L = Lout[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        float Lin = (profile != NULL) ? profile->get_lightness(row[0], row[1], row[2]) : ((row[0]+row[1]+row[2])/3.0f);
        if( log_conv && profile && profile->is_linear() ) {
          *L = (Lin>1.0e-16) ? log10(Lin) : -16;
        } else {
          *L = Lin;
        }
        //*Lg = *L;

        row += 3;
        L += 1;
      }
    }
  }

  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 1 ) return;
    if( ireg[0] == NULL ) return;

    EnhancedUnsharpMaskPar* opar = dynamic_cast<EnhancedUnsharpMaskPar*>(par);
    if( !opar ) return;

    const int RGBIN_INDEX = 0;

    profile = opar->get_profile();
    if( !profile ) return;

    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    T* pin;
    T* pL;
    T* pbh;
    T* pbl;
    T* pout;
    int x, x0, y;
    float scale = opar->get_amount();
    bool show_mask = opar->get_show_mask();
    bool linear = opar->get_linear();
    float thrlow = opar->get_threshold_l();
    float thrhigh = opar->get_threshold_h();
    int nscales = opar->get_nscales();

    float radius = opar->get_radius();
    for( unsigned int s = 1; s < nscales; s++ ) {
      radius *= 2;
    }

    int offsx = 0;
    int offsy = 0;
    int rw = ireg[RGBIN_INDEX]->valid.width;
    int rh = ireg[RGBIN_INDEX]->valid.height;
    int ileft = ireg[RGBIN_INDEX]->valid.left;
    int itop = ireg[RGBIN_INDEX]->valid.top;
    float* p = (float*)VIPS_REGION_ADDR( ireg[RGBIN_INDEX], ileft, itop );
    int rowstride = VIPS_REGION_LSKIP(ireg[RGBIN_INDEX]) / sizeof(float);
    PixelMatrix<float> rgbin(p, rw, rh, rowstride, offsy, offsx);

    // input luminance image
    PixelMatrix<float> Lin(rw, rh, offsy, offsx);
    fill_L_matrix( rw, rh, rgbin, Lin, false );

    // input log-luminance image
    PixelMatrix<float> logLin(rw, rh, offsy, offsx);
    fill_L_matrix( rw, rh, rgbin, logLin, (linear==false) );

    // output a and mean coefficients, low threshold
    PixelMatrix<float> a_thrlow(rw, rh, offsy, offsx);
    PixelMatrix<float> b_thrlow(rw, rh, offsy, offsx);
    PixelMatrix<float> mean_thrlow(rw, rh, offsy, offsx);
    // output a and mean coefficients, high threshold
    PixelMatrix<float> a_thrhigh(rw, rh, offsy, offsx);
    PixelMatrix<float> b_thrhigh(rw, rh, offsy, offsx);
    PixelMatrix<float> mean_thrhigh(rw, rh, offsy, offsx);

    int dx = oreg->valid.left - ireg[0]->valid.left;
    int dy = oreg->valid.top - ireg[0]->valid.top;

    // process and blend each scale
    for( unsigned int s = 0; s < nscales; s++ ) {

      eusm_gf(Lin, a_thrlow, b_thrlow, mean_thrlow, radius, thrlow, true);
      eusm_gf(logLin, a_thrhigh, b_thrhigh, mean_thrhigh, radius, thrhigh, false);

      for( y = 0; y < height; y++ ) {
        // original image
        pin = (float*)VIPS_REGION_ADDR( ireg[RGBIN_INDEX], r->left, r->top + y );
        // output image
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        float* pLin = Lin[y+dy]; pLin += dx;
        float* plogLin = logLin[y+dy]; plogLin += dx;
        float* pa1 = a_thrlow[y+dy]; pa1 += dx;
        float* pa2 = a_thrhigh[y+dy]; pa2 += dx;
        float* pb1 = b_thrlow[y+dy]; pb1 += dx;
        float* pb2 = b_thrhigh[y+dy]; pb2 += dx;
        float* pmean1 = mean_thrlow[y+dy]; pmean1 += dx;
        float* pmean2 = mean_thrhigh[y+dy]; pmean2 += dx;
        //float* pmean = mean_thrhigh[y+dy]; pmean += dx;

        for( x0 = 0, x = 0; x < line_size; x+=3, x0++ ) {
          // inverted low-threshold coefficients
          float a1 = pa1[x0];
          float b1 = pb1[x0];
          // high-threshold coefficients
          float a2 = pa2[x0];
          float b2 = pb2[x0];
          //printf("a1 = %f  a2 = %f\n", a1, a2);
          // compute the guided filer output for the two thresholds
          float gf1 = a1 * pLin[x0] + b1;
          float gf2 = a2 * plogLin[x0] + b2;
          if( !linear ) gf2 = xexp10(gf2);
#ifdef GF_DEBUG
          printf("Lin=%f  a1 = %f  a2 = %f  b1=%f  b2=%f (%f)  gf1=%f  gf2=%f\n",
              pLin[x0], a1, a2, b1, b2, xexp10(b2), gf1, gf2);
#endif
          // input luminance
          float L = pLin[x0];

          // compute the difference between the input and blurred luminances
          float delta1 = L - gf1;
          float delta2 = L - gf2;
          //float delta = L - gf;
          int gfid = (std::fabs(delta1) < std::fabs(delta2)) ? 1 : 2;
          float delta = (gfid == 1) ? delta1 : delta2;
          // add back the difference to the input luminance
          float Lout = L + (delta * scale); if(Lout < 0) Lout = 0;

          float R = (L < 1.0e-10) ? 1.0f : Lout / L;

          if( s == 0 ) {
            pout[x] = pin[x] * R;
            pout[x+1] = pin[x+1] * R;
            pout[x+2] = pin[x+2] * R;
          } else {
            R *= 0.6f;
            pout[x]   = pin[x]   * R + 0.4f * pout[x];
            pout[x+1] = pin[x+1] * R + 0.4f * pout[x+1];
            pout[x+2] = pin[x+2] * R + 0.4f * pout[x+2];
          }

          if( show_mask ) {
            float gf = (gfid == 1) ? gf1 : gf2;
            float a = (gfid == 1) ? a1 : a2;
            //float val = gf;
            float val = a;
            pout[x] = val;
            pout[x+1] = val;
            pout[x+2] = val;
          }
        }
      }
      radius /= 2;
      if( radius < 1 ) break;
    }
  }
};


ProcessorBase* new_enhanced_usm();

}

#endif 


