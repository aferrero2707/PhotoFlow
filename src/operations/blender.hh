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

#include <unistd.h>

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

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
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

      if( par->get_blend_mode() == PF_BLEND_PASSTHROUGH ) {
	/* In passthrough mode, we simply attach oreg to ireg[1]; ireg[0] is ignored.
	 */
	Rect *r = &oreg->valid;
	vips_region_region( oreg, ireg[1], r, r->left, r->top );
      } else {
	float opacity = par->get_opacity();
	BLENDER blender( par->get_blend_mode(), opacity );
#ifndef NDEBUG
	usleep(1000);
	//std::cout<<"opacity: "<<opacity<<std::endl;
#endif
	blender.blend( ireg[0], ireg[1], oreg, omap );
      }
    }
  };


  ProcessorBase* new_blender();
}


#endif
