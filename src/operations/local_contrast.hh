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

#ifndef PF_LOCAL_CONTRAST_H
#define PF_LOCAL_CONTRAST_H

#include "../base/splinecurve.hh"
#include "../base/processor.hh"

namespace PF 
{

  class LocalContrastPar: public OpParBase
  {
    Property<float> amount;
    Property<bool> enable_equalizer;
    Property<float> blacks_amount;
    Property<float> shadows_amount;
    Property<float> midtones_amount;
    Property<float> highlights_amount;
    Property<float> whites_amount;

    ProcessorBase* guided;

    SplineCurve tone_curve;
  public:
    float vec8[UCHAR_MAX+1];
    float vec16[65536/*USHRT_MAX+1*/];

    LocalContrastPar();

    bool has_intensity() { return false; }
    bool needs_caching() {
      return true;
    }

    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    float get_amount() { return amount.get(); }
    bool get_equalizer_enabled() { return enable_equalizer.get(); }
    SplineCurve& get_tone_curve() { return tone_curve; }
      
    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class LocalContrastProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, OpParBase* par) 
    {
      std::cout<<"LocalContrastProc::render() called."<<std::endl;
    }
  };


  template < OP_TEMPLATE_DEF_CS_SPEC >
  class LocalContrastProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      if( n != 2 ) return;
      if( ireg[0] == NULL ) return;
      if( ireg[1] == NULL ) return;

      LocalContrastPar* opar = dynamic_cast<LocalContrastPar*>(par);
      if( !opar ) return;

      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      T* pin1;
      T* pin2;
      T* pout;
      //typename FormatInfo<T>::SIGNED diff;
      float R, original, blurred, out;
      float grey, ngrey, intensity;
      int x, y, pos;
      //float threshold = opar->get_threshold()*FormatInfo<T>::RANGE;

      for( y = 0; y < height; y++ ) {
        pin1 = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pin2 = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x+=3 ) {
          //intensity = 0;
          if( opar->get_equalizer_enabled() ) {
            grey = 0.2126f*pin1[x] + 0.7152f*pin1[x+1] + 0.0722f*pin1[x+2];
            ngrey = (grey+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            intensity = opar->get_tone_curve().get_value( ngrey ) * opar->get_amount();
          } else
            intensity = opar->get_amount();

          pos = x;

          blurred = pin1[pos]; original = pin2[pos];
          if( fabs(blurred) < 1.0E-15 ) R = original*1.E15;
          else R = original / blurred;
          //R = ((R-1.0f)*intensity) + 1.0f;
          //pout[pos] = R * original;
          R = (R<0) ? R : powf(R, intensity);
          pout[pos] = R * blurred;
          //std::cout<<"original: "<<original<<"    R: "<<R<<std::endl;
          //out = R * original;
          //pout[pos] = intensity*out + (1.0f-intensity)*original;
          pos++;

          blurred = pin1[pos]; original = pin2[pos];
          if( fabs(blurred) < 1.0E-15 ) R = original*1.E15;
          else R = original / blurred;
          //R = ((R-1.0f)*intensity) + 1.0f;
          //pout[pos] = R * original;
          R = (R<0) ? R : powf(R, intensity);
          pout[pos] = R * blurred;
          //out = R * original;
          //pout[pos] = intensity*out + (1.0f-intensity)*original;
          pos++;

          blurred = pin1[pos]; original = pin2[pos];
          if( fabs(blurred) < 1.0E-15 ) R = original*1.E15;
          else R = original / blurred;
          //R = ((R-1.0f)*intensity) + 1.0f;
          //pout[pos] = R * original;
          R = (R<0) ? R : powf(R, intensity);
          pout[pos] = R * blurred;
          //out = R * original;
          //pout[pos] = intensity*out + (1.0f-intensity)*original;

          //pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos]; pos++;
          //pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos]; pos++;
          //pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos];
        }
      }
    }
  };

/*
  template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
  class LocalContrastProc<unsigned short,BLENDER,PF_COLORSPACE_RGB,CHMIN,CHMAX,has_imap,has_omap,PREVIEW>
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      if( n != 2 ) return;
      if( ireg[0] == NULL ) return;
      if( ireg[1] == NULL ) return;

      LocalContrastPar* opar = dynamic_cast<LocalContrastPar*>(par);
      if( !opar ) return;

      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      unsigned short* pin1;
      unsigned short* pin2;
      unsigned short* pout;
      unsigned short grey;
      float intensity;
      int x, y, pos;

      for( y = 0; y < height; y++ ) {
        pin1 = (unsigned short*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pin2 = (unsigned short*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        pout = (unsigned short*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x+=3 ) {
          grey = 0.2126f*pin1[x] + 0.7152f*pin1[x+1] + 0.0722f*pin1[x+2];
          if( opar->get_equalizer_enabled() )
            intensity = opar->vec16[ grey ] * opar->get_amount();
          else
            intensity = opar->get_amount();

          //std::cout<<"grey="<<grey<<"    intensity="<<intensity<<std::endl;

          pos = x;
          pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos]; pos++;
          pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos]; pos++;
          pout[pos] = intensity*pin2[pos] + (1.0f-intensity)*pin1[pos];
        }
      }
    }
  };
*/


  ProcessorBase* new_local_contrast();

}

#endif 


