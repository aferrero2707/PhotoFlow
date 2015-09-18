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

#ifndef PF_CLIP_WARNING_H
#define PF_CLIP_WARNING_H

#include <string>

#include "../base/processor.hh"


namespace PF 
{

class ClippingWarningPar: public OpParBase
{
  bool highlights_warning_enabled, shadows_warning_enabled;
public:
  ClippingWarningPar();

  void set_highlights_warning( bool flag ) { highlights_warning_enabled = flag; }
  bool get_highlights_warning() { return highlights_warning_enabled; }
  void set_shadows_warning( bool flag ) { shadows_warning_enabled = flag; }
  bool get_shadows_warning() { return shadows_warning_enabled; }

  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_input() { return true; }
};



template < OP_TEMPLATE_DEF >
class ClippingWarning
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};


template < OP_TEMPLATE_DEF_CS_SPEC >
class ClippingWarning< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;

    ClippingWarningPar* opar = dynamic_cast<ClippingWarningPar*>( par );
    if( !opar ) return;

    T max = static_cast<T>( 0.999f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );
    T min = static_cast<T>( 0.001f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );

    T* p;
    T* pin;
    T* pout;
    typename PF::FormatInfo<T>::PROMOTED avg;
    int x, y;

    for( y = 0; y < height; y++ ) {
      p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      for( x=0; x < line_size; x+=3 ) {
        avg = p[x]; avg += p[x+1]; avg += p[x+2]; avg /= 3;
        if( avg<=min && opar->get_shadows_warning() ) {
          pout[x] = static_cast<T>( 0.2f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );
          pout[x+1] = static_cast<T>( 0.8f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );
          pout[x+2] = static_cast<T>( 1.0f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );
        } else if( avg>=max && opar->get_highlights_warning() ) {
          pout[x] = static_cast<T>( 1.0f*PF::FormatInfo<T>::RANGE + PF::FormatInfo<T>::MIN );
          pout[x+1] = 0;
          pout[x+2] = 0;
        } else {
          pout[x] = p[x];
          pout[x+1] = p[x+1];
          pout[x+2] = p[x+2];
        }
      }
    }
  }
};





ProcessorBase* new_clipping_warning();
}

#endif 


