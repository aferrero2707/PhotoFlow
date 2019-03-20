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

#ifndef PF_DYNAMIC_RANGE_COMPRESSOR_V2_H
#define PF_DYNAMIC_RANGE_COMPRESSOR_V2_H

//#ifdef __SSE2__
//#include "../rt/rtengine/sleefsseavx.c"
//#else
#include "../rt/rtengine/sleef.c"
//#endif

#include "../base/splinecurve.hh"
#include "../base/processor.hh"

namespace PF 
{

  class DynamicRangeCompressorV2Par: public OpParBase
  {
    Property<float> amount;
    Property<bool> enable_equalizer;
    Property<float> blacks_amount;
    Property<float> shadows_amount;
    Property<float> midtones_amount;
    Property<float> highlights_amount;
    Property<float> whites_amount;

    Property<int> bilateral_iterations;
    Property<float> bilateral_sigma_s;
    Property<float> bilateral_sigma_r;

    Property<float> guided_radius, guided_threshold;

    Property<float> strength_s, strength_h, anchor;
    Property<float> local_contrast;

    Property<float> lumi_blend;

    Property<bool> show_residual;
    bool show_residual_;

    ProcessorBase* loglumi;
    ProcessorBase* to_log;
    ProcessorBase* bilateral;
    ProcessorBase* guided;

    PF::ICCProfile* profile;

    SplineCurve tone_curve;
    bool caching;
  public:
    float vec8[UCHAR_MAX+1];
    float vec16[65536/*USHRT_MAX+1*/];

    DynamicRangeCompressorV2Par();

    bool has_intensity() { return false; }
    bool needs_caching() {
      return caching;
    }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    PF::ICCProfile* get_profile() { return profile; }

    float get_strength_s() { return strength_s.get(); }
    float get_strength_h() { return strength_h.get(); }
    float get_anchor() { return anchor.get(); }
    float get_local_contrast() { return local_contrast.get(); }
    float get_lumi_blend() { return lumi_blend.get(); }
    bool get_show_residual() { return show_residual_; }

    float get_amount() { return amount.get(); }
    bool get_equalizer_enabled() { return enable_equalizer.get(); }
    SplineCurve& get_tone_curve() { return tone_curve; }

    VipsImage* build_old(std::vector<VipsImage*>& in, int first,
         VipsImage* imap, VipsImage* omap,
         unsigned int& level);

    VipsImage* build_new(std::vector<VipsImage*>& in, int first,
         VipsImage* imap, VipsImage* omap,
         unsigned int& level);

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
         VipsImage* imap, VipsImage* omap,
         unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class DynamicRangeCompressorV2Proc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, OpParBase* par) 
    {
      std::cout<<"DynamicRangeCompressorV2Proc::render() called."<<std::endl;
    }
  };


  template < OP_TEMPLATE_DEF_CS_SPEC >
  class DynamicRangeCompressorV2Proc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public:
    void render_old(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      static const double inv_log_base = 1.0 / log(10.0);
      static const double min_value = -2.5, max_value = 2;
      //static const double gamma = log(50) * inv_log_base / (max_value - min_value);
      //if( n != 3 ) return;
      if( ireg[0] == NULL ) return;
      //if( ireg[1] == NULL ) return;
      //if( ireg[2] == NULL ) return;

      DynamicRangeCompressorV2Par* opar = dynamic_cast<DynamicRangeCompressorV2Par*>(par);
      if( !opar ) return;
      PF::ICCProfile* profile = opar->get_profile();
      if( !profile ) return;

      const float gamma_s = 1.0/(1.0 + 0.09*opar->get_strength_s());
      const float gamma_h = 1.0/(1.0 + 0.09*opar->get_strength_h());
      const float lc = opar->get_local_contrast() + 1;
      const float lcm = 1.0f-lc;
      //std::cout<<"DynamicRangeCompressorV2Proc::render(): lc="<<lc<<"  lcm="<<lcm<<std::endl;

      Rect *r = &oreg->valid;
      int width = r->width;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      if( false && r->left<10000 && r->top<10000 )
        std::cout<<"DynamicRangeCompressorV2Proc::render(): region="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;

      T* psmooth;
      T* plog;
      T* pin;
      T* pout;
      //typename FormatInfo<T>::SIGNED diff;
      float diff, out, lout;
      float grey, ngrey, intensity, L, lL;
      int x, y, pos;
      //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;
      float bias = profile->perceptual2linear(0.5);
      //float lbias = log(bias) * inv_log_base;

      for( y = 0; y < height; y++ ) {
        psmooth = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        plog = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        pin = (T*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < width; x++, pin+=3, pout+=3, psmooth++, plog++ ) {
          //pout[0] = pin[0]; pout[1] = pin[1]; pout[2] = pin[2]; continue;

          float l = ( plog[0]*(6*2+1) ) - 6;
          float s0 = ( psmooth[0]*(6*2+1) ) - 6;
          float s = s0;
          //float s = (lc*s0 + lcm*l);
          //float s = (0.1*(plog[0]+psmooth[0])) - 6;

          diff = l - s;
          float gamma1 = (s>0) ? gamma_h : gamma_s;
          float gamma2 = (l>0) ? gamma_h : gamma_s;
          //double exp = s;
          //double exp = ((s * gamma) + lc*diff);
          double exp = opar->get_show_residual() ? s : ( lc * ((s * gamma1) + diff) + lcm * l * gamma2 );
          //double exp = opar->get_show_residual() ? s : ( lc * (powf(s, gamma1) + diff) + lcm * powf(l, gamma2) );
          //double exp = opar->get_show_residual() ? s0 : powf(s, gamma1) + diff;

          //out = pow( 10.0, l * gamma );
          //out = pow( 10.0, exp );
          //out = psmooth[0];
          out = xexp10( exp );

          out *= bias;

          //out = pow( 10.0, (0.1*psmooth[0]) - 6 + diff/** gamma + diff*/ );
          //out = (((psmooth[0]-50) * gamma) + 50 + diff)/100;
          //out = (((plog[0]-50) * gamma) + 50)/100;

          //if( profile->is_linear() )
          //  lout = profile->perceptual2linear(out);
          //else lout = out;

          //pout[0] = pout[1] = pout[2] = out;

          /**/
          //intensity = 0;
          L = profile->get_lightness(pin[0], pin[1], pin[2]);
          if( opar->get_equalizer_enabled() ) {
            ngrey = (L+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            intensity = opar->get_tone_curve().get_value( ngrey ) * opar->get_amount();
          } else
            intensity = opar->get_amount();
          const float ratio = (L>1.0e-12) ? out / L : 0;
          pout[0] = pin[0] * ratio; pout[1] = pin[1] * ratio; pout[2] = pin[2] * ratio;
          if(false)
            //if(false && x==(width-1) && y==(r->height-1) && r->left==0 && r->top==0)
            std::cout<<"L="<<L<<"  plog[0]="<<*plog<<"  psmooth[0]="<<*psmooth
            <<"  l="<<l<<"  s="<<s<<"  gamma_s="<<gamma_s<<"  diff="<<diff<<"  out="<<out
            <<"  ratio="<<ratio<<std::endl;

          /**/
        }
      }
    }


    void render_new(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      static const double inv_log_base = 1.0 / log(10.0);
      static const double min_value = -2.5, max_value = 2;
      //static const double gamma = log(50) * inv_log_base / (max_value - min_value);
      //if( n != 3 ) return;
      if( ireg[0] == NULL ) return;
      //if( ireg[1] == NULL ) return;
      //if( ireg[2] == NULL ) return;

      DynamicRangeCompressorV2Par* opar = dynamic_cast<DynamicRangeCompressorV2Par*>(par);
      if( !opar ) return;
      PF::ICCProfile* profile = opar->get_profile();
      if( !profile ) return;

      const float gamma_s = 1.0/(1.0 + 0.09*opar->get_strength_s());
      //const float gamma_h = 1.0/(1.0 + 0.09*opar->get_strength_h());
      const float gamma_h = gamma_s;
      const float lc = opar->get_local_contrast() + 1;
      const float lcm = 1.0f-lc;

      Rect *r = &oreg->valid;
      int width = r->width;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      if( false && r->left<10000 && r->top<10000 ) {
        std::cout<<"DynamicRangeCompressorV2Proc::render(): region="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;
        std::cout<<"DynamicRangeCompressorV2Proc::render(): show_residual="<<opar->get_show_residual()<<std::endl;
      }

      T* psmooth;
      T* plog;
      T* pin;
      T* pout;
      //typename FormatInfo<T>::SIGNED diff;
      float diff, ratio, out, lout;
      float grey, ngrey, intensity, L, lL;
      int x, y, pos, ch;
      float bias = profile->perceptual2linear(opar->get_anchor());
      //float lbias = log(bias) * inv_log_base;

      for( y = 0; y < height; y++ ) {
        psmooth = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        plog = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        pin = (T*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        float out[4];
        for( x = 0; x < width; x++, pin+=3, pout+=3, psmooth+=4, plog+=4 ) {
          //pout[0] = pin[0]; pout[1] = pin[1]; pout[2] = pin[2]; continue;

          float l[4] = {plog[0], plog[1], plog[2], plog[3]};
          float s0[4] = {psmooth[0], psmooth[1], psmooth[2], psmooth[3]};
          float s[4] = {lc*s0[0] + lcm*l[0], lc*s0[1] + lcm*l[1], lc*s0[2] + lcm*l[2], lc*s0[3] + lcm*l[3]};

          for( int ch = 0; ch < 4; ch++ ) {
            diff = l[ch] - s[ch];
            ratio = (fabs(s[ch]) > 1.0e-15) ? l[ch] / s[ch] : l[ch] / 1.0e-15;
            float gamma1 = (s[ch]>0) ? gamma_h : gamma_s;
            float gamma2 = (l[ch]>0) ? gamma_h : gamma_s;
            //double expo = opar->get_show_residual() ? s[ch] :
            //    ( lc * ((s[ch] * gamma1) + diff) + lcm * (l[ch] * gamma2) );
            double expo = opar->get_show_residual() ? s0[ch] : ( (s[ch] * gamma1) + diff );

            out[ch] = xexp10( expo ) * bias;
            //std::cout<<"l[ch]="<<l[ch]<<"  s[ch]="<<s[ch]<<"  ratio="<<ratio
            //    <<"  gamma1="<<gamma1<<"  gamma2="<<gamma2
            //    <<"  expo="<<expo<<"  out[ch]"<<out[ch]<<std::endl;
          }

          /**/
          //intensity = 0;
          /*
          L = profile->get_lightness(pin[0], pin[1], pin[2]);
          if( opar->get_equalizer_enabled() ) {
            ngrey = (L+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            intensity = opar->get_tone_curve().get_value( ngrey ) * opar->get_amount();
          } else
            intensity = opar->get_amount();
          const float Lratio = (L>1.0e-12) ? out / L : 0;
          pout[0] = pin[0] * ratio; pout[1] = pin[1] * ratio; pout[2] = pin[2] * ratio;
           */
          float Lblend = opar->get_lumi_blend();
          float iLblend = 1.0f - Lblend;
          if(opar->get_lumi_blend() > 0 && !opar->get_show_residual()) {
            float Lin = profile->get_lightness(pin[0], pin[1], pin[2]);
            //float Lout = profile->get_lightness(out[0], out[1], out[2]);
            float Lout = out[3];
            const float Lratio = (Lin>1.0e-12) ? Lout / Lin : Lout/1.0e-12;
            pout[0] = Lblend * pin[0] * Lratio + iLblend * out[0];
            pout[1] = Lblend * pin[1] * Lratio + iLblend * out[1];
            pout[2] = Lblend * pin[2] * Lratio + iLblend * out[2];
          } else
            pout[0] = out[0]; pout[1] = out[1]; pout[2] = out[2];
          //pout[0] = pin[0]; pout[1] = pin[1]; pout[2] = pin[2];
          if(false && x==(width-1) && y==(r->height-1) && r->left==0 && r->top==0)
            std::cout<<"L="<<L<<"  plog[0]="<<*plog<<"  psmooth[0]="<<*psmooth
            <<"  l="<<l<<"  s="<<s<<"  gamma_s="<<gamma_s<<"  diff="<<diff<<"  out="<<out
            <<"  ratio="<<ratio<<std::endl;

          /**/
        }
      }
    }

    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      render_old( ireg, n, in_first, imap, omap, oreg, par );
    }
  };


  ProcessorBase* new_dynamic_range_compressor_v2();

}

#endif 


