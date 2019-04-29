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

#ifndef PF_THRESHOLD_H
#define PF_THRESHOLD_H

#include <string>

#include "../base/processor.hh"


namespace PF 
{

  class ThresholdPar: public OpParBase
  {
    Property<float> threshold;
    Property<bool> invert;

  public:

    ThresholdPar();

    float get_threshold() { return threshold.get(); }
    bool do_invert() { return invert.get(); }

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
       creation of an intensity map is not allowed
       2. the operation can work without an input image;
       the blending will be set in this case to "passthrough" and the image
       data will be simply linked to the output
    */
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }
  };

  

  template < OP_TEMPLATE_DEF > 
  class Threshold
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      ThresholdPar* opar = dynamic_cast<ThresholdPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      T thr = static_cast<T>( opar->get_threshold()*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );

      T* p;
      T* pin;
      T* pout;
      int x, y;

      if( opar->do_invert() ) {
        for( y = 0; y < height; y++ ) {
          p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x=0; x < line_size; x++ ) {
            if( p[x]<=thr ) pout[x] = PF::FormatInfo<T>::MAX;
            else pout[x] = PF::FormatInfo<T>::MIN;
          }
        }
      } else {
        for( y = 0; y < height; y++ ) {
          p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x=0; x < line_size; x++ ) {
            if( p[x]>thr ) pout[x] = PF::FormatInfo<T>::MAX;
            else pout[x] = PF::FormatInfo<T>::MIN;
          }
        }
      }
    }
  };




  template < OP_TEMPLATE_DEF_CS_SPEC >
  class Threshold< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      ThresholdPar* opar = dynamic_cast<ThresholdPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
      int width = r->width;
      int height = r->height;

      T thr = static_cast<T>( opar->get_threshold()*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );

      T* p;
      T* pin;
      T* pout;
      typename PF::FormatInfo<T>::PROMOTED avg;
      int x, y;

      if( opar->do_invert() ) {
        for( y = 0; y < height; y++ ) {
          p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x=0; x < line_size; x+=3 ) {
            avg = p[x]; avg += p[x+1]; avg += p[x+2]; avg /= 3;
            if( avg<=thr ) pout[x] = pout[x+1] = pout[x+2] = PF::FormatInfo<T>::MAX;
            else pout[x] = pout[x+1] = pout[x+2] = PF::FormatInfo<T>::MIN;
          }
        }
      } else {
        for( y = 0; y < height; y++ ) {
          p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
          pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
          for( x=0; x < line_size; x+=3 ) {
            avg = p[x]; avg += p[x+1]; avg += p[x+2]; avg /= 3;
            if( avg>thr ) pout[x] = pout[x+1] = pout[x+2] = PF::FormatInfo<T>::MAX;
            else pout[x] = pout[x+1] = pout[x+2] = PF::FormatInfo<T>::MIN;
          }
        }
      }
    }
  };




  ProcessorBase* new_threshold();
}

#endif 


