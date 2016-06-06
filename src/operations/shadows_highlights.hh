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

#ifndef PF_SHAHI_H
#define PF_SHAHI_H


#define UNBOUND_L 1
#define UNBOUND_A 2
#define UNBOUND_B 4
#define UNBOUND_SHADOWS_L UNBOUND_L
#define UNBOUND_SHADOWS_A UNBOUND_A
#define UNBOUND_SHADOWS_B UNBOUND_B
#define UNBOUND_HIGHLIGHTS_L (UNBOUND_L << 3) /* 8 */
#define UNBOUND_HIGHLIGHTS_A (UNBOUND_A << 3) /* 16 */
#define UNBOUND_HIGHLIGHTS_B (UNBOUND_B << 3) /* 32 */
#define UNBOUND_GAUSSIAN 64
#define UNBOUND_BILATERAL 128 /* not implemented yet */
#define UNBOUND_DEFAULT                                                                                      \
    (UNBOUND_SHADOWS_L | UNBOUND_SHADOWS_A | UNBOUND_SHADOWS_B | UNBOUND_HIGHLIGHTS_L | UNBOUND_HIGHLIGHTS_A   \
        | UNBOUND_HIGHLIGHTS_B | UNBOUND_GAUSSIAN)

#define CLAMPF(a, mn, mx) ((a) < (mn) ? (mn) : ((a) > (mx) ? (mx) : (a)))
#define CLAMP_RANGE(x, y, z) (CLAMP(x, y, z))
#define MMCLAMPPS(a, mn, mx) (_mm_min_ps((mx), _mm_max_ps((a), (mn))))

namespace PF 
{

enum shahi_method_t
{
  SHAHI_GAUSSIAN,
  SHAHI_BILATERAL,
};


inline float sign(float x)
{
  return (x < 0 ? -1.0f : 1.0f);
}



class ShadowsHighlightsPar: public OpParBase
{
  PropertyBase method;
  Property<float> shadows, highlights, wp_adjustment, radius, compress, sh_color_adjustment, hi_color_adjustment;

  ProcessorBase* gauss;
  ProcessorBase* convert2lab;
  ProcessorBase* convert2input;

  cmsHPROFILE in_profile;

public:
  ShadowsHighlightsPar();

  bool has_intensity() { return false; }
  bool needs_caching();

  float get_shadows() { return shadows.get(); }
  float get_highlights() { return highlights.get(); }
  float get_wp_adjustment() { return wp_adjustment.get(); }
  float get_compress() { return compress.get(); }
  float get_sh_color_adjustment() { return sh_color_adjustment.get(); }
  float get_hi_color_adjustment() { return hi_color_adjustment.get(); }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class ShadowsHighlightsProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class ShadowsHighlightsProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( n != 2 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;

    ShadowsHighlightsPar* opar = dynamic_cast<ShadowsHighlightsPar*>(par);
    if( !opar ) return;

    //const int order = data->order;
    const float sigma = 100;//fmaxf(0.1f, opar->get_radius());
    //const float sigma = radius * roi_in->scale / piece->iscale;
    const float shadows = 2.0f * fmin(fmax(-1.0, (opar->get_shadows() / 100.0f)), 1.0f);
    const float highlights = 2.0f * fmin(fmax(-1.0, (opar->get_highlights() / 100.0f)), 1.0f);
    const float whitepoint = fmax(1.0f - opar->get_wp_adjustment() / 100.0f, 0.01f);
    const float compress
    = fmin(fmax(0, (opar->get_compress() / 100.0f)), 0.99f); // upper limit 0.99f to avoid division by zero later
    const float shadows_ccorrect = (fmin(fmax(0.0f, (opar->get_sh_color_adjustment() / 100.0f)), 1.0f) - 0.5f)
                                             * sign(shadows) + 0.5f;
    const float highlights_ccorrect = (fmin(fmax(0.0f, (opar->get_hi_color_adjustment() / 100.0f)), 1.0f) - 0.5f)
                                                * sign(-highlights) + 0.5f;
    const unsigned int flags = UNBOUND_DEFAULT/*data->flags*/;
    const int unbound_mask = true;/*((data->shadhi_algo == SHADHI_ALGO_BILATERAL) && (flags & UNBOUND_BILATERAL))
                               || ((data->shadhi_algo == SHADHI_ALGO_GAUSSIAN) && (flags & UNBOUND_GAUSSIAN));*/
    const float low_approximation = 0.01f;//data->low_approximation;

    const float max[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float min[4] = { 0.0f, -1.0f, -1.0f, 0.0f };
    const float lmin = 0.0f;
    const float lmax = max[0] + fabs(min[0]);
    const float halfmax = lmax / 2.0;
    const float doublemax = lmax * 2.0;

    float highlights2 = highlights * highlights;

    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    float* pin1;
    float* pin2;
    float* pout;
    //typename FormatInfo<T>::SIGNED diff;
    float diff, out;
    int x, y, pos;
    //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;

    for( y = 0; y < height; y++ ) {
      pin1 = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {

        // invert and desaturate
        pout[x] = 1.0f - pin2[x];
        pout[x+1] = pout[x+2] = 0.0f;

        float ta[3] = {pin1[x],(pin1[x+1]-0.5f)*2,(pin1[x+2]-0.5f)*2}, tb[3] = {pout[x],0.0f,0.0f};
        /*
        if( y == 0 && x == 0 )
          printf("in: %f,%f,%f    out: %f,%f,%f\n",pin1[x],pin1[x+1],pin1[x+2],tb[0],tb[1],tb[2]);
        if( y == 0 && x == 0 )
          printf("in2: %f,%f,%f    out: %f,%f,%f\n",ta[0],ta[1],ta[2],tb[0],tb[1],tb[2]);
        *//*
        pout[x] = pin1[x];
        pout[x+1] = pin1[x+1];
        pout[x+2] = pin1[x+2];
        continue;
        */

        ta[0] = ta[0] > 0.0f ? ta[0] / whitepoint : ta[0];
        tb[0] = tb[0] > 0.0f ? tb[0] / whitepoint : tb[0];

        // overlay highlights
        float highlights_xform = CLAMP_RANGE(1.0f - tb[0] / (1.0f - compress), 0.0f, 1.0f);

        while(highlights2 > 0.0f)
        {
          float la = (flags & UNBOUND_HIGHLIGHTS_L) ? ta[0] : CLAMP_RANGE(ta[0], lmin, lmax);
          float lb = (tb[0] - halfmax) * sign(-highlights) * sign(lmax - la) + halfmax;
          lb = unbound_mask ? lb : CLAMP_RANGE(lb, lmin, lmax);
          float lref = copysignf(fabs(la) > low_approximation ? 1.0f / fabs(la) : 1.0f / low_approximation, la);
          float href = copysignf(
              fabs(1.0f - la) > low_approximation ? 1.0f / fabs(1.0f - la) : 1.0f / low_approximation, 1.0f - la);

          float chunk = highlights2 > 1.0f ? 1.0f : highlights2;
          float optrans = chunk * highlights_xform;
          highlights2 -= 1.0f;

          ta[0] = la * (1.0 - optrans)
                          + (la > halfmax ? lmax - (lmax - doublemax * (la - halfmax)) * (lmax - lb) : doublemax * la
                              * lb) * optrans;

          ta[0] = (flags & UNBOUND_HIGHLIGHTS_L) ? ta[0] : CLAMP_RANGE(ta[0], lmin, lmax);

          ta[1] = ta[1] * (1.0f - optrans)
                          + (ta[1] + tb[1]) * (ta[0] * lref * (1.0f - highlights_ccorrect)
                              + (1.0f - ta[0]) * href * highlights_ccorrect) * optrans;

          ta[1] = (flags & UNBOUND_HIGHLIGHTS_A) ? ta[1] : CLAMP_RANGE(ta[1], min[1], max[1]);

          ta[2] = ta[2] * (1.0f - optrans)
                          + (ta[2] + tb[2]) * (ta[0] * lref * (1.0f - highlights_ccorrect)
                              + (1.0f - ta[0]) * href * highlights_ccorrect) * optrans;

          ta[2] = (flags & UNBOUND_HIGHLIGHTS_B) ? ta[2] : CLAMP_RANGE(ta[2], min[2], max[2]);
        }

        // overlay shadows
        float shadows2 = shadows * shadows;
        float shadows_xform = CLAMP_RANGE(tb[0] / (1.0f - compress) - compress / (1.0f - compress), 0.0f, 1.0f);

        while(shadows2 > 0.0f)
        {
          float la = (flags & UNBOUND_HIGHLIGHTS_L) ? ta[0] : CLAMP_RANGE(ta[0], lmin, lmax);
          float lb = (tb[0] - halfmax) * sign(shadows) * sign(lmax - la) + halfmax;
          lb = unbound_mask ? lb : CLAMP_RANGE(lb, lmin, lmax);
          float lref = copysignf(fabs(la) > low_approximation ? 1.0f / fabs(la) : 1.0f / low_approximation, la);
          float href = copysignf(
              fabs(1.0f - la) > low_approximation ? 1.0f / fabs(1.0f - la) : 1.0f / low_approximation, 1.0f - la);


          float chunk = shadows2 > 1.0f ? 1.0f : shadows2;
          float optrans = chunk * shadows_xform;
          shadows2 -= 1.0f;

          ta[0] = la * (1.0 - optrans)
                          + (la > halfmax ? lmax - (lmax - doublemax * (la - halfmax)) * (lmax - lb) : doublemax * la
                              * lb) * optrans;

          ta[0] = (flags & UNBOUND_SHADOWS_L) ? ta[0] : CLAMP_RANGE(ta[0], lmin, lmax);

          ta[1] = ta[1] * (1.0f - optrans)
                          + (ta[1] + tb[1]) * (ta[0] * lref * shadows_ccorrect
                              + (1.0f - ta[0]) * href * (1.0f - shadows_ccorrect)) * optrans;

          ta[1] = (flags & UNBOUND_SHADOWS_A) ? ta[1] : CLAMP_RANGE(ta[1], min[1], max[1]);

          ta[2] = ta[2] * (1.0f - optrans)
                          + (ta[2] + tb[2]) * (ta[0] * lref * shadows_ccorrect
                              + (1.0f - ta[0]) * href * (1.0f - shadows_ccorrect)) * optrans;

          ta[2] = (flags & UNBOUND_SHADOWS_B) ? ta[2] : CLAMP_RANGE(ta[2], min[2], max[2]);
        }

        //_Lab_rescale(ta, &out[j]);
        pout[x] = ta[0];
        pout[x+1] = ta[1] / 2.f + 0.5f;
        pout[x+2] = ta[2] / 2.f + 0.5f;
      }
    }
  }
};


ProcessorBase* new_shadows_highlights();

}

#endif 


