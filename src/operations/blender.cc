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
#include "../base/processor.hh"

//#include "../vips/vips_layer.h"
int
vips_layer( VipsImage **in, int n, VipsImage **out, int first, 
            PF::ProcessorBase* proc,
            VipsImage* imap, VipsImage* omap, 
            VipsDemandStyle demand_hint,
	    int width, int height, int nbands);

VipsImage* PF::BlenderPar::build(std::vector<VipsImage*>& in, int first, 
				 VipsImage* imap, VipsImage* omap, 
				 unsigned int& level)
{
  VipsImage* outnew;
  VipsImage* in1 = NULL;
  VipsImage* in2 = NULL;

  if( in.empty() ) return NULL;
  if( in.size() > 0 ) in1 = in[0];
  if( in.size() > 1 ) in2 = in[1];

  /**/
  // Prepare the blending step between the new image (in invec[1]) and the underlying image
  // if existing (in invec[0]).
  // The blending code will simply force the mode to "passthrough" and copy invec[1] to outnew
  // if invec[0] is NULL
  if( (in1 != NULL) && (in2 != NULL) && (get_blend_mode() != PF_BLEND_PASSTHROUGH) ) {
    std::vector<VipsImage*> in_;
    in_.push_back( in1 );
    in_.push_back( in2 );
    outnew = PF::OpParBase::build( in_, first, NULL, omap, level );
  } else if( (in1 != NULL) && (in2 != NULL) && (get_blend_mode() == PF_BLEND_PASSTHROUGH) ) {
    outnew = in2;
    g_object_ref( outnew );
  } else if( (in1 == NULL) && (in2 != NULL) ) {
    // in1 is NULL, force mode to PASSTHROUGH and copy in2 to output
    outnew = in2;
    g_object_ref( outnew );
  } else if( (in2 == NULL) && (in1 != NULL) ) {
    // in2 is NULL, force mode to PASSTHROUGH and copy in1 to output
    outnew = in1;
    g_object_ref( outnew );
  } else {
    std::cerr<<"PF::BlenderPar::build(): unsupported input pattern and blend mode combination!"<<std::endl;
    return NULL;
  }
  /**/

  //VipsImage* invec[2] = {in1, in2};
  //vips_layer( invec, 2, &outnew, 0, get_processor(), NULL, omap, get_demand_hint() );
#ifndef NDEBUG
  std::cout<<"PF::BlenderPar::build(): input: "<<in1<<" "<<in2<<"   output: "<<outnew<<std::endl;
#endif
  //set_image( outnew );
  return outnew;
}





PF::ProcessorBase* PF::new_blender()
{
  return( new PF::Processor<PF::BlenderPar,PF::BlenderProc>() );
}
