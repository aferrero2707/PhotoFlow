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

#ifndef PF_PIXEL_PROCESSOR__H
#define PF_PIXEL_PROCESSOR__H

#include <iostream>

//#include "processor.hh"
//#include "layer.hh"
#include "operation.hh"
#include "color.hh"

namespace PF
{

#define PIXELPROC_TEMPLATE_DEF                  \
  typename T_, colorspace_t CS_,                  \
    int CHMIN_, int CHMAX_,                       \
    bool PREVIEW_, typename OP_PAR_

#define PIXELPROC_TEMPLATE_IMP                  \
  T, CS, CHMIN, CHMAX, PREVIEW, OP_PAR


#define PIXELPROC_TEMPLATE_DEF_CS_SPEC                  \
  typename T_,                                           \
    int CHMIN_, int CHMAX_,                               \
    bool PREVIEW_, typename OP_PAR_
  
#define PIXELPROC_TEMPLATE_IMP_CS_SPEC(CS_SPEC) \
  T, CS_SPEC, CHMIN, CHMAX, PREVIEW, OP_PAR



  class PixelProcessorPar: public OpParBase
  {
    Property<float> color_blend;

  public:
    PixelProcessorPar():
      OpParBase(), 
      color_blend("color_blend",this,0)
    {
    }
    bool has_imap() { return false; }
    bool has_omap() { return false; }
    bool needs_input() { return true; }

    float get_color_blend() { return( color_blend.get() ); }
  };

  




  template < OP_TEMPLATE_DEF, class OP_PAR, template < PIXELPROC_TEMPLATE_DEF > class PEL_PROC > 
  class PixelProcessor: public IntensityProc<T, has_imap>
  {
    typedef OP_PAR OpParams;
  public: 
    void render(VipsRegion** ir, int n, int in_first,
                VipsRegion* imap, VipsRegion* ,
                VipsRegion* oreg, OP_PAR* par)
    {
      PEL_PROC<PIXELPROC_TEMPLATE_IMP> proc(par);

      float intensity = par->get_intensity();
      float blend = par->get_color_blend();
      float nblend = -1.0f * blend;
      Rect *r = &oreg->valid;
      int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

#ifdef _NDEBUG
      std::cout<<"PixelProcessor::render(): "<<std::endl
               <<"  name: "<<par->get_config_ui()->get_layer()->get_name()<<std::endl
               <<"  input region:  top="<<ir[in_first]->valid.top
               <<" left="<<ir[in_first]->valid.left
               <<" width="<<ir[in_first]->valid.width
               <<" height="<<ir[in_first]->valid.height<<std::endl
               <<"  output region: top="<<oreg->valid.top
               <<" left="<<oreg->valid.left
               <<" width="<<oreg->valid.width
               <<" height="<<oreg->valid.height<<std::endl;
#endif    
    
      const int NMAX = 100;
      T* p[NMAX+1];
      T* pout;
      T* pimap;
    
      if(n > NMAX) n = NMAX;
    
      int x, y, ch, dx=CHMAX-CHMIN+1;//, CHMAXplus1=CHMAX+1;
      int ximap, ni;

      //std::cout<<"PixelProcessor::render(): CHMIN="<<CHMIN<<"  CHMAX="<<CHMAX
      //    <<"PF::ColorspaceInfo<CS>::NCH="<<PF::ColorspaceInfo<CS>::NCH<<std::endl;
    
      for( y = 0; y < r->height; y++ ) {
        
        for( ni = 0; ni < n; ni++) 
          p[ni] = (T*)VIPS_REGION_ADDR( ir[ni], r->left, r->top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
        if(has_imap) pimap = (T*)VIPS_REGION_ADDR( imap, r->left, r->top + y );
        
        for( x=0, ximap=0; x < line_size; ) {
          float intensity_real = this->get_intensity( intensity, pimap, ximap );
          for( ch=0; ch<CHMIN; ch++, x++ ) pout[x] = p[0][x];
          proc.process( p, n, in_first, sz, x, intensity_real/*get_intensity( intensity, pimap, ximap )*/, pout );
          x += dx;
          for( ch=CHMAX+1; ch<PF::ColorspaceInfo<CS>::NCH; ch++, x++ ) pout[x] = p[0][x];
          //for( ch=0; ch<PF::ColorspaceInfo<CS>::NCH; ch++, x++ ) pout[x] = p[0][x];
        }
        if( CS != PF_COLORSPACE_RGB || blend == 0 ) continue;

        if( blend > 0 ) {
          // Blend the output luminance with the input colors
          // Preserves the color information of the input image
          // Equivalent to the "luminosity blend" in photoshop
          double ired, ig, ib;
          double ored, og, ob;
          double ored2, og2, ob2;
          for( x=0, ximap=0; x < line_size; ) {
            ored = (double(pout[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            og = (double(pout[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ob = (double(pout[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            double lumi = PF::luminance( ored, og, ob );
            ired = (double(p[0][x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ig = (double(p[0][x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ib = (double(p[0][x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            //std::cout<<"Input:  "<<ired<<" "<<ig<<" "<<ib<<std::endl;
            //std::cout<<"Output: "<<ored<<" "<<og<<" "<<ob<<std::endl;
            ored2 = ired; og2 = ig; ob2 = ib;
            PF::lc_blend( ored2, og2, ob2, lumi );
            pout[x] = (T)(( (ored2*blend)+(ored*(1.0f-blend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            pout[x+1] = (T)(( (og2*blend)+(og*(1.0f-blend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            pout[x+2] = (T)(( (ob2*blend)+(ob*(1.0f-blend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            //std::cout<<"ired="<<ired<<"  ored="<<ored<<"  (ored*blend)+(ired*(1.0f-blend))="<<(ored*blend)+(ired*(1.0f-blend))<<std::endl;
            //std::cout<<"p[x]="<<p[0][x]<<"  pout[x]="<<pout[x]<<std::endl;
            x += 3;
          }
        } else if( blend < 0 ) {
          // Blend the output color with the input luminance
          // Preserves the luminance information of the input image
          // Equivalent to the "color blend" in photoshop
          double ired, ig, ib;
          double ored, og, ob;
          double ored2, og2, ob2;
          for( x=0, ximap=0; x < line_size; ) {
            ired = (double(p[0][x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ig = (double(p[0][x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ib = (double(p[0][x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ored = (double(pout[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            og = (double(pout[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            ob = (double(pout[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
            double lumi = PF::luminance( ired, ig, ib );
            ored2 = ored; og2 = og; ob2 = ob;
            PF::lc_blend( ored2, og2, ob2, lumi );
            pout[x] = (T)(( (ored2*nblend)+(ored*(1.0f-nblend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            pout[x+1] = (T)(( (og2*nblend)+(og*(1.0f-nblend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            pout[x+2] = (T)(( (ob2*nblend)+(ob*(1.0f-nblend)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
            x += 3;
          }
        }
      }
    }
  };

}

#endif
