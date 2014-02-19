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

#ifndef VIPS_BLENDER_H
#define VIPS_BLENDER_H

#include <string>

#include "../base/operation.hh"

namespace PF 
{

  class BlenderPar: public OpParBase
  {
  public:
    BlenderPar(): OpParBase()
    {
      set_type( "blender" );
      set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
    }

    /* Set processing hints:
       1. the intensity parameter makes no sense for a blending operation, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, VipsImage* imap, VipsImage* omap);
  };

  
  template < OP_TEMPLATE_DEF > 
  class BlenderProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
      if( (n != 2) || (ireg[0] == NULL) || (ireg[1] == NULL) ) {
	std::cerr<<"BlenderProc::render(): wrong number of input images"<<std::endl;
	return;
      }

      BLENDER blender;
      float opacity = par->get_opacity();
#ifndef NDEBUG
      usleep(10000);
      //std::cout<<"opacity: "<<opacity<<std::endl;
#endif
    
      Rect *r = &oreg->valid;
      int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
      int line_size = r->width * ireg[0]->im->Bands;

#ifndef NDEBUG
      std::cout<<std::endl<<std::endl<<"BlenderProc::render(): blending region "
	       <<r->left<<","<<r->top<<" x "<<r->width<<","<<r->height<<std::endl;
#endif

      T* p[2];    
      T* pout;
      int x, xomap, y;

      for( y = 0; y < r->height; y++ ) {
      
	p[0] = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y ); 
	p[1] = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y ); 
	pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	blender.init_line( omap, r->left, r->top + y );
      
	for( x=0, xomap=0; x < line_size; ) {
	  blender.blend( opacity, p[0], p[1], pout, x, xomap );
	  x += sz;
	}
      }
    }
  };


  template < OP_TEMPLATE_DEF_BLENDER_SPEC > 
  class BlenderProc< OP_TEMPLATE_IMP_BLENDER_SPEC(BlendPassthrough) >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
      if( (n != 2) || (ireg[1] == NULL) ) {
	std::cerr<<"BlenderProc<BlenderPassthrough>::render(): wrong number of input images"<<std::endl;
	return;
      }

      Rect *r = &oreg->valid;

      /* In passthrough mode, we simply attach oreg to ireg[1]; ireg[0] is ignored.
       */
      vips_region_region( oreg, ireg[1], r, r->left, r->top );
    }
  };



}


#endif
