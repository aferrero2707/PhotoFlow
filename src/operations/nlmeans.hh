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

#ifndef PF_NLMEANS_H
#define PF_NLMEANS_H

#include <string.h>

#include "../base/processor.hh"

namespace PF 
{

class NonLocalMeansPar: public OpParBase
{
  Property<float> radius, strength, luma_frac, chroma_frac;

  int P, K;
  float sharpness;

  ProcessorBase* convert2lab;
  ProcessorBase* convert2input;
  ProcessorBase* nlmeans_algo;

  PF::ICCProfile* in_profile;

public:
  NonLocalMeansPar();

  bool has_intensity() { return false; }
  bool needs_caching() { return false; }

  void set_radius( float r ) { radius.set(r); }
  void set_strength( float s ) { strength.set(s); }
  void set_luma_frac( float f ) { luma_frac.set(f); }
  void set_chroma_frac( float f ) { chroma_frac.set(f); }


  int get_P() {return P; }
  int get_K() { return K; }
  float get_sharpness() { return sharpness; }
  float get_luma_frac() { return luma_frac.get(); }
  float get_chroma_frac() { return chroma_frac.get(); }

  int get_padding() { return P+K; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



class NonLocalMeans_DTAlgo_Par: public OpParBase
{
  int P, K;
  float sharpness, luma_frac, chroma_frac;

public:
  NonLocalMeans_DTAlgo_Par(): OpParBase() {set_type("nlmeans_DT");}

  bool has_intensity() { return false; }
  bool needs_caching() { return false; }

  int get_P() {return P; }
  void set_P(int val) { P = val; }
  int get_K() { return K; }
  void set_K(int val) { K = val; }
  float get_sharpness() { return sharpness; }
  void set_sharpness(float val) { sharpness = val; }
  float get_luma_frac() { return luma_frac; }
  void set_luma_frac(float val) { luma_frac = val; }
  float get_chroma_frac() { return chroma_frac; }
  void set_chroma_frac(float val) { chroma_frac = val; }

  int get_padding() { return P+K; }

  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout)
  {
    int pad = get_padding();
    rout->left = rin->left+pad;
    rout->top = rin->top+pad;
    rout->width = rin->width-pad*2;
    rout->height = rin->height-pad*2;
  }

  /* Function to derive the area to be read from input images,
     based on the requested output area
   */
  virtual void transform_inv(const Rect* rout, Rect* rin)
  {
    int pad = get_padding();
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
  }
};



template < OP_TEMPLATE_DEF >
class NonLocalMeansProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
    std::cout<<"NonLocalMeansProc::render() called."<<std::endl;
  }
};


typedef union floatint_t
{
  float f;
  uint32_t i;
} floatint_t;

/* Non-local means filter, adapted from Darktable
 */
template < OP_TEMPLATE_DEF >
class NonLocalMeans_DTAlgo_Proc
{
  float fast_mexp2f(const float x)
  {
    const float i1 = (float)0x3f800000u; // 2^0
    const float i2 = (float)0x3f000000u; // 2^-1
    const float k0 = i1 + x * (i2 - i1);
    floatint_t k;
    k.i = k0 >= (float)0x800000u ? k0 : 0;
    return k.f;
  }

  float gh(const float f, const float sharpness)
  {
    const float f2 = f * sharpness;
    return fast_mexp2f(f2);
    // return 0.0001f + dt_fast_expf(-fabsf(f)*800.0f);
    // return 1.0f/(1.0f + f*f);
    // make spread bigger: less smoothing
    // const float spread = 100.f;
    // return 1.0f/(1.0f + fabsf(f)*spread);
  }


public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    //std::cout<<"NonLocalMeansProc::render(RGB) called: n="<<n<<std::endl;
    if( n < 1 ) return;
    if( ireg[0] == NULL ) {std::cout<<"NonLocalMeansProc::render(RGB): ireg[0]==NULL"<<std::endl;return;}

    //int iwidth = ireg[0]->valid.width;
    //int iheight = ireg[0]->valid.height;

    //The cleaning algorithm starts here

    NonLocalMeans_DTAlgo_Par* opar = dynamic_cast<NonLocalMeans_DTAlgo_Par*>(par);
    if( !opar ) {
      std::cout<<"NonLocalMeansProc::render(RGB): opar==NULL"<<std::endl;
      return;
    }

    Rect *ir = &ireg[0]->valid;
    VipsRect roi = {ir->left+2, ir->top+2, ir->width-4, ir->height-4};
    Rect *r = &oreg->valid;
    vips_rect_intersectrect( &roi, r, &roi );
    int nch = oreg->im->Bands;
    int line_size = roi.width * oreg->im->Bands;
    int width = ir->width;
    int height = ir->height;
    int dx = r->left - ir->left;
    int dy = r->top - ir->top;
    int padding = r->top - ir->top;

    //std::cout<<"NonLocalMeansProc::render(RGB):"<<std::endl
    //    <<"ir: "<<ir->width<<","<<ir->height<<"+"<<ir->left<<"+"<<ir->top<<std::endl
    //    <<"r:  "<<r->width<<","<<r->height<<"+"<<r->left<<"+"<<r->top<<std::endl;

    //T* pin0;
    ///T* pin1;
    //T* pin2;
    //T* pin20;
    //T* pin21;
    //T* pin22;
    //T* pout;
    //int x, y, oy, x1, y1, pos, ipos, pos1;

    // adjust to zoom size:
    const int P = opar->get_P(); //ceilf(d->radius * fmin(roi_in->scale, 2.0f) / fmax(piece->iscale, 1.0f)); // pixel filter size
    const int K = opar->get_K(); //ceilf(7 * fmin(roi_in->scale, 2.0f) / fmax(piece->iscale, 1.0f));         // nbhood
    const float sharpness = opar->get_sharpness(); //3000.0f / (1.0f + d->strength);
    float luma = opar->get_luma_frac();
    float chroma = opar->get_chroma_frac();

    //int padding = opar->get_padding();

    // adjust to Lab, make L more important
    // float max_L = 100.0f, max_C = 256.0f;
    // float nL = 1.0f/(d->luma*max_L), nC = 1.0f/(d->chroma*max_C);
    //float max_L = 120.0f, max_C = 512.0f;
    float max_L = 1.2f, max_C = 2.0f;
    float nL = 1.0f / max_L, nC = 1.0f / max_C;
    const float norm2[4] = { nL * nL, nC * nC, nC * nC, 1.0f };
    //const float norm2[4] = { 1.0f, 1.0f, .0f, 1.0f };

    /*
    {
      for(int j = 0; j < 10; j++) {
      T* pin = (T*)VIPS_REGION_ADDR( ireg[0], ir->left, ir->top+j );
      for( int i = 0; i < 10*nch; i+=nch ) printf("%f ", pin[i]*100); printf("\n");
      }
    }
    */

    float *Sa = (float*)malloc( sizeof(float) * width);
    float *out_norm = (float*)malloc( sizeof(float) * width * height);
    memset(out_norm, 0x0, sizeof(float) * width * height);

    // we want to sum up weights in col[3], so need to init to 0:
    for( int y = 0; y < r->height; y++ ) {
      T* pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      memset( pout, 0x0, VIPS_REGION_SIZEOF_LINE(oreg) );
    }
    //memset(ovoid, 0x0, (size_t)sizeof(float) * roi_out->width * roi_out->height * 4);

    // for each shift vector
    for(int kj = -K; kj <= K; kj++) {
      for(int ki = -K; ki <= K; ki++) {
        int inited_slide = 0;
        // don't construct summed area tables but use sliding window! (applies to cpu version res < 1k only, or else
        // we will add up errors)
        // do this in parallel with a little threading overhead. could parallelize the outer loops with a bit more
        // memory
//#ifdef _OPENMP
//#pragma omp parallel for schedule(static) default(none) firstprivate(inited_slide) shared(kj, ki, Sa)
//#endif
        for(int j = 0; j < height; j++) {
          if(j + kj < 0 || j + kj >= height) continue;
          float *S = Sa;
          //const float *ins = ((float *)ivoid) + 4 * ((size_t)roi_in->width * (j + kj) + ki);
          const T *ins = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + ki, ir->top + j + kj );
          //float *out = ((float *)ovoid) + 4 * (size_t)roi_out->width * j;

          const int Pm = MIN(MIN(P, j + kj), j);
          const int PM = MIN(MIN(P, height - 1 - j - kj), height - 1 - j);
          // first line of every thread
          // TODO: also every once in a while to assert numerical precision!
          if(!inited_slide) {
            // sum up a line
            memset(Sa, 0x0, sizeof(float) * width);
            for(int jj = -Pm; jj <= PM; jj++) {
              int i = MAX(0, -ki);
              float *s = S + i;
              //const float *inp = ((float *)ivoid) + 4 * i + 4 * (size_t)roi_in->width * (j + jj);
              const T* inp = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i, ir->top + j + jj );
              //printf("inp: left=%d top=%d\n", ir->left + i, ir->top + j + jj);
              //const float *inps = ((float *)ivoid) + 4 * i + 4 * ((size_t)roi_in->width * (j + jj + kj) + ki);
              const T* inps = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i + ki, ir->top + j + jj + kj);
              //printf("inps: left=%d top=%d\n", ir->left + i + ki, ir->top + j + jj + kj);
              const int last = width + MIN(0, -ki);
              for(; i < last; i++, inp += nch, inps += nch, s++)
              {
                for(int k = 0; k < nch; k++) {
                  //float temp = (inp[k] - inps[k]) * (inp[k] - inps[k]) * norm2[k];
                  s[0] += (inp[k] - inps[k]) * (inp[k] - inps[k]) * norm2[k];
                  if( false && /*j < 10 && i < 10 &&*/ k==0)
                    printf("  kj=%d  ki=%d  j=%d  Pm=%d  PM=%d  jj=%d  i=%d  inp[%d]=%f inps[%d]=%f s[0]+=%f\n",
                        kj, ki, j, Pm, PM, jj, i, k, (float)inp[k]*100, k, (float)inps[k]*100,
                        ((inp[k] - inps[k]) * (inp[k] - inps[k]) * norm2[k]));
                }
              }
              if( false && /*j< 10*/ true )
                printf("  kj=%d  ki=%d  j=%d  Pm=%d  PM=%d  jj=%d  i=%d  last=%d  s[0]=%f\n",
                    kj, ki, j, Pm, PM, jj, i, last, s[0]);
            }
            // only reuse this if we had a full stripe
            if(Pm == P && PM == P) inited_slide = 1;
          }
          //continue;
          //std::cout<<"inited_slide="<<inited_slide<<std::endl;

          // sliding window for this line:
          T* out = (T*)VIPS_REGION_ADDR( oreg, ir->left, ir->top + j );
          float* on = out_norm + width*j;
          float *s = S;
          float slide = 0.0f;
          // sum up the first -P..P
          for(int i = 0; i < 2 * P + 1; i++) slide += s[i];
          for(int i = 0; i < width; i++, s++, ins += nch, out += nch, on += 1) {
            if(i - P > 0 && i + P < width)
              slide += s[P] - s[-P - 1];
            if(i >= padding && i < width - padding && i + ki >= 0 && i + ki < width && j >= padding && j < height - padding) {
              const float iv[4] = { static_cast<float>(ins[0]), static_cast<float>(ins[1]), static_cast<float>(ins[2]), 1.0f };
              //#if defined(_OPENMP) && defined(OPENMP_SIMD_)
              //#pragma omp SIMD()
              //#endif
              float gh_val = gh(slide, sharpness);
              for(int c = 0; c < nch; c++)
              {
                //printf("  i=%d    out[0] = %f + %f * gh(%f, %f) = %f + %f * %f = %f\n",
                //    i, out[c], iv[c], slide, sharpness,
                //    out[c], iv[c], gh_val,
                //    out[c] + iv[c] * gh_val);
                out[c] += iv[c] * gh_val;
              }
              //printf("  i=%d    on[0] = %f + %f = %f\n", i, on[0], gh_val, on[0] + gh_val);
              on[0] += gh_val;
            }
          }
          //continue;
          if(inited_slide && j + P + 1 + MAX(0, kj) < height)
          {
            // sliding window in j direction:
            int i = MAX(0, -ki);
            float *s = S + i;
            //const float *inp = ((float *)ivoid) + 4 * i + 4 * (size_t)roi_in->width * (j + P + 1);
            const T* inp = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i, ir->top + j + P + 1 );
            //const float *inps = ((float *)ivoid) + 4 * i + 4 * ((size_t)roi_in->width * (j + P + 1 + kj) + ki);
            const T* inps = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i + ki, ir->top + j + P + 1 + kj );
            //const float *inm = ((float *)ivoid) + 4 * i + 4 * (size_t)roi_in->width * (j - P);
            const T* inm = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i, ir->top + j - P );
            //const float *inms = ((float *)ivoid) + 4 * i + 4 * ((size_t)roi_in->width * (j - P + kj) + ki);
            const T* inms = (T*)VIPS_REGION_ADDR( ireg[0], ir->left + i + ki, ir->top + j - P + kj );
            const int last = width + MIN(0, -ki);
            for(; i < last; i++, inp += nch, inps += nch, inm += nch, inms += nch, s++)
            {
              float stmp = s[0];
              for(int k = 0; k < nch; k++)
                stmp += ((inp[k] - inps[k]) * (inp[k] - inps[k]) - (inm[k] - inms[k]) * (inm[k] - inms[k])) * norm2[k];
              s[0] = stmp;
            }
          }
          else
            inited_slide = 0;
        }
      }
    }

    // normalize and apply chroma/luma blending
    const float weight[4] = { luma, chroma, chroma, 1.0f };
    const float invert[4] = { 1.0f - luma, 1.0f - chroma, 1.0f - chroma, 0.0f };

    for(int j = 0; j < r->height; j++) {
      T* in = ( (T*)VIPS_REGION_ADDR(ireg[0], r->left, r->top + j) );
      T* out = ( (T*)VIPS_REGION_ADDR(oreg, r->left, r->top + j) );
      float* on = out_norm + (j+padding) * width + padding;
      for(int i = 0; i < r->width; i++, in+=nch, out+=nch, on+=1) {
        for(int c = 0; c < nch; c++) {
          //printf("j=%d i=%d  out[c]=%f * %f / %f = %f\n", j, i, out[c], weight[c], on[0], out[c] * weight[c] / on[0]);
          out[c] = in[c] * invert[c] + out[c] * weight[c] / on[0];
        }
        //out[1] = out[2] = 0.5;
      }
    }

    //#ifdef _OPENMP
    //#pragma omp parallel for SIMD() default(none) schedule(static) collapse(2)
    //#endif
    //    for(size_t k = 0; k < (size_t)ch * roi_out->width * roi_out->height; k += ch)
    //    {
    //      for(size_t c = 0; c < 4; c++)
    //      {
    //        out[k + c] = (in[k + c] * invert[c]) + (out[k + c] * (weight[c] / out[k + 3]));
    //      }
    //    }

    // free shared tmp memory:
    free(Sa);
    free(out_norm);
  }
};



ProcessorBase* new_nlmeans();

}

#endif 


