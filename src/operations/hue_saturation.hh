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

#ifndef HUE_SATURATION_H
#define HUE_SATURATION_H

#include <string>

#include <glibmm.h>

#include <libraw/libraw.h>

#include "../base/color.hh"
#include "../base/processor.hh"


namespace PF 
{

  class HueSaturationPar: public OpParBase
  {
    Property<float> hue;
    Property<float> saturation;

  public:

    HueSaturationPar();

    float get_hue() { return hue.get(); }
    float get_saturation() { return saturation.get(); }

    bool has_intensity() { return true; }
    bool has_opacity() { return true; }
    bool needs_input() { return true; }
  };

  

  template < OP_TEMPLATE_DEF > 
  class HueSaturation
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
    }
  };




  template < OP_TEMPLATE_DEF_CS_SPEC > 
  class HueSaturation< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_RGB) >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      HueSaturationPar* opar = dynamic_cast<HueSaturationPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      //int width = r->width;
      int height = r->height;

      T* pin;
      T* pout;
      T R, G, B;
      float h, s, v;
      int x, y;

      for( y = 0; y < height; y++ ) {
        pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        for( x = 0; x < line_size; x+=3 ) {
          R = pin[x];
          G = pin[x+1];
          B = pin[x+2];
          rgb2hsv( R, G, B, h, s, v );

          //std::cout<<"in RGB: "<<R<<" "<<G<<" "<<B<<"  HSV: "<<h<<" "<<s<<" "<<v<<std::endl;

          h += opar->get_hue();
          if( h > 360 ) h -= 360;
          else if( h < 0 ) h+= 360;

          s += opar->get_saturation();
          if( s < 0 ) s = 0;
          else if( s > 1 ) s = 1;

          hsv2rgb2( h, s, v, R, G, B );
          //std::cout<<"out RGB: "<<R<<" "<<G<<" "<<B<<"  HSV: "<<h<<" "<<s<<" "<<v<<std::endl;
          pout[x] = R;
          pout[x+1] = G;
          pout[x+2] = B;          
        }
      }
    }
  };



  ProcessorBase* new_hue_saturation();
}

#endif 


