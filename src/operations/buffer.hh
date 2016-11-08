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

#ifndef VIPS_BUFFER_H
#define VIPS_BUFFER_H

#include <string>

#include "../base/operation.hh"

namespace PF 
{

  class BufferPar: public OpParBase
  {
  public:
    BufferPar(): OpParBase()
    {
      set_type( "buffer" );
    }

    /* Set processing hints:
       1. the intensity parameter makes no sense for a blending operation, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  
  template < OP_TEMPLATE_DEF > 
  class BufferProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      Rect *r = &oreg->valid;
      int width = r->width;
      int height = r->height;
      int line_size = sizeof(T) * width * oreg->im->Bands; //layer->in_all[0]->Bands;
      T* p;
      T* pout;
      int y;

      std::cout<<"BufferProc::render(): region: "<<r->width<<","<<r->height<<"+"<<r->left<<"+"<<r->top<<std::endl;

      for( y = 0; y < height; y++ ) {
        p = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
        memcpy( pout, p, line_size );
      }
    }
  };


  ProcessorBase* new_buffer();
}


#endif
