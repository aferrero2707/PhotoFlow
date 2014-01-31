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

#include "blender.hh"

//#include "../vips/vips_layer.h"
int
vips_layer( VipsImage **in, int n, VipsImage **out, int first, 
            PF::ProcessorBase* proc,
            VipsImage* imap, VipsImage* omap, 
            VipsDemandStyle demand_hint);

VipsImage* PF::BlenderPar::build(std::vector<VipsImage*>& in, int first, 
				 VipsImage* imap, VipsImage* omap)
{
  VipsImage* outnew;

  if( in.size() != 2 ) {
    std::cerr<<"PF::BlenderPar::build(): wrong number of input images: "
	     <<in.size()<<" (should be 2)"<<std::endl;
    return NULL;
  } 

  // Prepare the blending step between the new image (in invec[1]) and the underlying image
  // if existing (in invec[0]).
  // The blending code will simply force the mode to "passthrough" and copy invec[1] to outnew
  // if invec[0] is NULL
  if( (in[0] != NULL) && (in[1] != NULL) && (get_blend_mode() != PF_BLEND_PASSTHROUGH) ) {
    VipsImage* invec[2] = {in[0], in[1]};
    vips_layer( invec, 2, &outnew, 0, get_processor(), NULL, omap, get_demand_hint() );
  } else if( (in[0] == NULL) && (in[1] != NULL) ) {
    // in[0] is NULL, force mode to PASSTHROUGH and copy in[1] to output
    vips_copy( in[1], &outnew, NULL );
  } else if( (in[1] != NULL) && (get_blend_mode() == PF_BLEND_PASSTHROUGH) ) {
    // mode is set to PASSTHROUGH, simply copy in[1] to output
    vips_copy( in[1], &outnew, NULL );
  } else {
    std::cerr<<"PF::BlenderPar::build(): unsupported input pattern and blend mode combination!"<<std::endl;
    return NULL;
  }

  //set_image( outnew );
  return outnew;
}
