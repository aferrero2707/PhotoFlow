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

#ifndef PF_SMART_MASK__HH
#define PF_SMART_MASK__HH

#include <iostream>

#include "../base/format_info.hh"
#include "../base/processor.hh"

namespace PF 
{

  class SmartMaskPar: public OpParBase
  {
    Property<float> L_ref;
    Property<float> L_range;
    Property<float> L_slope;

    Property<float> a_ref;
    Property<float> a_range;
    Property<float> a_slope;

    Property<float> b_ref;
    Property<float> b_range;
    Property<float> b_slope;

  public:
    float L_min_start, L_min_end, L_max_start, L_max_end;
    float a_min_start, a_min_end, a_max_start, a_max_end;
    float b_min_start, b_min_end, b_max_start, b_max_end;

    SmartMaskPar();


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, 
                     unsigned int& level);
  };


  template < OP_TEMPLATE_DEF > 
  class GaussBlurProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      if( n < 2 ) return;
      if( ireg[1] == NULL ) return;
      SmartMaskPar* par2 = dynamic_cast<SmartMaskPar*>( par );
      if( !par2 ) return;
      float min_start[3] = {
        par2->L_min_start, par2->a_min_start, par2->b_min_start
      };
      float min_end[3] = {
        par2->L_min_end, par2->a_min_end, par2->b_min_end
      };
      float max_start[3] = {
        par2->L_max_start, par2->a_max_start, par2->b_max_start
      };
      float max_end[3] = {
        par2->L_max_end, par2->a_max_end, par2->b_max_end
      };

      Rect *r = &oreg->valid;
      int sz = oreg->im->Bands;
      if( sz != 3 ) return;
      int line_size = r->width * sz;

      T* p;
      T* pout;
      int x, y, ch;
      float val[3];
      float out[3];

      for( y = 0; y < r->height; y++ ) {
        
        p = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        for( x=0; x < line_size; x += 3) {
          val[0] = ((float)p[x] + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
          val[1] = ((float)p[x+1] + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
          val[2] = ((float)p[x+2] + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

          for( ch=0; ch < 3; ch++ ) {
            out[ch] = 0;
            if( val[ch] >= min_end[ch] && val[ch] <= max_start[ch] )
              out[ch] = 1;
            else if( val[ch] >= min_start[ch] && val[ch] < min_end[ch] )
              out[ch] = (val[ch]-min_start[ch])/(min_end[ch]-min_start[ch]);
            else if( val[ch] >= max_start[ch] && val[ch] < max_end[ch] )
              out[ch] = (val[ch]-min_start[ch])/(min_end[ch]-min_start[ch]);
        }
    }
};


  template < OP_TEMPLATE_DEF_CS_SPEC > 
  class GaussBlurProc<OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_GRAYSCALE)>
  {
  };


}
