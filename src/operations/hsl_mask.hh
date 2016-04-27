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

#ifndef HSL_MASK_H
#define HSL_MASK_H

#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../base/color.hh"
#include "../base/processor.hh"
#include "../base/splinecurve.hh"


namespace PF 
{

  class HSLMaskPar: public OpParBase
  {
    Property<bool> invert;
    Property<SplineCurve> H_curve;
    Property<SplineCurve> S_curve;
    Property<SplineCurve> L_curve;
    Property<bool> H_curve_enabled;
    Property<bool> S_curve_enabled;
    Property<bool> L_curve_enabled;

    Property<SplineCurve>* eq_vec[3];

    void update_curve( Property<SplineCurve>* grey_curve, float* vec );

  public:

    float vec[3][65536];
    bool eq_enabled[3];

    HSLMaskPar();

    Property<SplineCurve>& get_H_curve() { return H_curve; }
    Property<SplineCurve>& get_S_curve() { return S_curve; }
    Property<SplineCurve>& get_L_curve() { return L_curve; }

    void set_H_curve_enabled( bool en ) { H_curve_enabled.set( en ); }
    void set_S_curve_enabled( bool en ) { S_curve_enabled.set( en ); }
    void set_L_curve_enabled( bool en ) { L_curve_enabled.set( en ); }

    void set_invert( bool flag) { invert.update(flag); }
    bool get_invert() { return invert.get(); }

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first,
                     VipsImage* imap, VipsImage* omap,
                     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class HSLMask
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };



  template<class T>
  T RGB2HSLMask( T* RGB, HSLMaskPar* opar, bool inv )
  {
    float h_in, s_in, v_in, l_in;
    rgb2hsv( RGB[0], RGB[1], RGB[2], h_in, s_in, l_in );
    //rgb2hsl( RGB[0], RGB[1], RGB[2], h_in, s_in, l_in );
    //std::cout<<"in RGB: "<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<"  HSL: "<<h_in<<" "<<s_in<<" "<<l_in<<std::endl;

    if( s_in<0 ) s_in = 0; if( s_in>1 ) s_in = 1;
    if( l_in<0 ) l_in = 0; if( l_in>1 ) l_in = 1;

    unsigned short int hid = static_cast<unsigned short int>( h_in*65535/360 );
    unsigned short int sid = static_cast<unsigned short int>( s_in*65535 );
    unsigned short int lid = static_cast<unsigned short int>( l_in*65535 );


    float h_eq1 = opar->eq_enabled[0] ? opar->vec[0][hid] : 1;
    float h_eq2 = opar->eq_enabled[1] ? opar->vec[1][sid] : 1;
    float h_eq3 = opar->eq_enabled[2] ? opar->vec[2][lid] : 1;

    //std::cout<<"opar->vec[2][65535]="<<opar->vec[2][65535]<<std::endl;

    float h_eq = MIN3( h_eq1, h_eq2, h_eq3 );
    if( inv ) h_eq = 1.0f - h_eq;


    T val = FormatInfo<T>::RANGE*h_eq + FormatInfo<T>::MIN;
    //std::cout<<"  lid: "<<lid<<"   h_eq3: "<<h_eq3<<"   h_eq: "<<h_eq<<"   val: "<<val<<std::endl;
    return val;
  }



  template < OP_TEMPLATE_DEF_CS_SPEC >
  class HSLMask< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_GRAYSCALE) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      HSLMaskPar* opar = dynamic_cast<HSLMaskPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;
      bool ok = true;
      if( n<1 ) ok = false;

      bool inv = opar->get_invert();

      // We take the last input image as the mask source
      int in_id = n-1;

      if( ok && ireg[in_id]->im->Bands != 3 )
        ok = false;

      T* pin;
      T* pout;
      T RGB[3];
      float h, s, v, l;
      int xin, xout, y, k;

      if( ok && (opar->eq_enabled[0] || opar->eq_enabled[1] || opar->eq_enabled[2]) ) {
        for( y = 0; y < height; y++ ) {
          pin = (T*)VIPS_REGION_ADDR( ireg[in_id], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

          for( xin=0, xout=0; xout < line_size; xin+=3, xout++ ) {
            RGB[0] = pin[xin];
            RGB[1] = pin[xin+1];
            RGB[2] = pin[xin+2];
            T val = RGB2HSLMask( RGB, opar, inv );

            pout[xout] = val;
          }
        }
      } else {
        for( y = 0; y < height; y++ ) {
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( xout = 0; xout < line_size; xout++ ) {
            pout[xout] = FormatInfo<T>::MAX;
          }
        }
      }
    }
  };




  template < OP_TEMPLATE_DEF_CS_SPEC >
  class HSLMask< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      HSLMaskPar* opar = dynamic_cast<HSLMaskPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;
      if( n<1 ) return;
      int in_id = 0;
      if( n>1 ) in_id = 1;

      if( ireg[in_id]->im->Bands != 3 )
        return;

      bool inv = opar->get_invert();

      T* pin;
      T* pout;
      T RGB[3];
      float h, s, v, l;
      int x, y, k;

      if( opar->eq_enabled[0] || opar->eq_enabled[1] || opar->eq_enabled[2] ) {
        for( y = 0; y < height; y++ ) {
          pin = (T*)VIPS_REGION_ADDR( ireg[in_id], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

          for( x = 0; x < line_size; x+=3 ) {
            RGB[0] = pin[x];
            RGB[1] = pin[x+1];
            RGB[2] = pin[x+2];
            T val = RGB2HSLMask( RGB, opar, inv );

            pout[x] = val;
            pout[x+1] = val;
            pout[x+2] = val;
          }
        }
      } else {
        for( y = 0; y < height; y++ ) {
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x = 0; x < line_size; x+=3 ) {
            pout[x] = pout[x+1] = pout[x+2] = FormatInfo<T>::MAX;
          }
        }
      }
    }
  };




  template < OP_TEMPLATE_DEF_CS_SPEC >
  class HSLMask< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      HSLMaskPar* opar = dynamic_cast<HSLMaskPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      T* pin;
      T* pout;
      typename PF::FormatInfo<T>::SIGNED a, b;
      int x, y;

      for( y = 0; y < height; y++ ) {
        pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      }
    }
  };



  ProcessorBase* new_hsl_mask();
}

#endif 


