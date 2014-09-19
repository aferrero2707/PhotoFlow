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


PF::BlenderPar::BlenderPar(): 
  OpParBase(),
  blend_mode("blend_mode",this),
  opacity("opacity",this,1)
{
  blend_mode.set_enum_value( PF_BLEND_NORMAL );
  set_type( "blender" );
  set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
}


VipsImage* PF::BlenderPar::build(std::vector<VipsImage*>& in, int first, 
                                 VipsImage* imap, VipsImage* omap, 
                                 unsigned int& level)
{
  VipsImage* outnew;
  VipsImage* in1 = NULL;
  VipsImage* in2 = NULL;
  void *data;
  size_t data_length;
  cmsHPROFILE profile_in;

  if( in.empty() ) return NULL;
  if( in.size() > 0 ) in1 = in[0];
  if( in.size() > 1 ) in2 = in[1];

  if( in1 ) {
    if( !vips_image_get_blob( in1, VIPS_META_ICC_NAME, 
                              &data, &data_length ) ) {
    
      profile_in = cmsOpenProfileFromMem( data, data_length );
      if( profile_in ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<"BlenderPar::build(): Input profile: "<<tstr<<std::endl;
        cmsCloseProfile( profile_in );
      }
    }  
  }

  //std::cout<<"BlenderPar::build(): opacity="<<get_opacity()<<std::endl;

  /**/
  // Prepare the blending step between the new image (in invec[1]) and the underlying image
  // if existing (in invec[0]).
  // The blending code will simply force the mode to "passthrough" and copy invec[1] to outnew
  // if invec[0] is NULL
  // The mode will be forced to passthrough also when the blending mode is set to "normal",
  // the opacity is 100% and the opacity map is NULL, since in this case the underlying
  // image is useless. This improves performance for the default layer settings.
  bool is_passthrough = false;
  if( get_blend_mode() == PF_BLEND_PASSTHROUGH ) is_passthrough = true;
  if( (get_blend_mode() == PF_BLEND_NORMAL) && 
      (get_opacity() > 0.999f) &&
      (omap == NULL) ) is_passthrough = true;

  // If both images are not NULL and the blending mode is not "passthrough-equivalent",
  // we activate the blending code.
  // In all other cases, one of the input images is copied to the output
  // without further processing (and without any performance overhead)
  if( (in1 != NULL) && (in2 != NULL) && (!is_passthrough) ) {
    std::vector<VipsImage*> in_;
    in_.push_back( in1 );
    in_.push_back( in2 );
    outnew = PF::OpParBase::build( in_, first, NULL, omap, level );
  } else if( (in1 != NULL) && (in2 != NULL) && is_passthrough ) {
    outnew = in2;
    PF_REF( outnew, "BlenderPar::build() in2 ref" );
  } else if( (in1 == NULL) && (in2 != NULL) ) {
    // in1 is NULL, force mode to PASSTHROUGH and copy in2 to output
    outnew = in2;
    PF_REF( outnew, "BlenderPar::build() in2 ref" );
  } else if( (in2 == NULL) && (in1 != NULL) ) {
    // in2 is NULL, force mode to PASSTHROUGH and copy in1 to output
    outnew = in1;
    PF_REF( outnew, "BlenderPar::build() in1 ref" );
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
  if( outnew ) {
    if( !vips_image_get_blob( outnew, VIPS_META_ICC_NAME, 
                              &data, &data_length ) ) {
    
      profile_in = cmsOpenProfileFromMem( data, data_length );
      if( profile_in ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<"BlenderPar::build(): Output profile: "<<tstr<<std::endl;
        cmsCloseProfile( profile_in );
      }
    }  
  }

  return outnew;
}





PF::ProcessorBase* PF::new_blender()
{
  return( new PF::Processor<PF::BlenderPar,PF::BlenderProc>() );
}
