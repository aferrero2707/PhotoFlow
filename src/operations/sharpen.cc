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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "unsharp_mask.hh"
#include "gmic/sharpen_rl.hh"
#include "sharpen.hh"


PF::SharpenPar::SharpenPar(): 
  OpParBase(), 
  method("method",this,PF::SHARPEN_USM,"USM","Unsharp Mask"),
  usm_radius("usm_radius",this,1),
  rl_sigma("rl_sigma",this,1),
  rl_iterations("rl_iterations",this,10)
{
	//method.add_enum_value(PF::SHARPEN_USM,"USM","Unsharp Mask");
#ifndef PF_DISABLE_GMIC
	method.add_enum_value(PF::SHARPEN_DECONV,"DECONV","RL Deconvolution");
#endif
	//method.add_enum_value(PF::SHARPEN_MICRO,"MICRO","Micro Contrast");

  usm = new_unsharp_mask();
#ifndef PF_DISABLE_GMIC
  rl = new_gmic_sharpen_rl();
#endif

  set_type("sharpen" );

  set_default_name( _("sharpening") );
}


VipsImage* PF::SharpenPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  VipsImage* out = NULL;
  switch( method.get_enum_value().first ) {
  case PF::SHARPEN_USM: {
    UnsharpMaskPar* usmpar = dynamic_cast<UnsharpMaskPar*>( usm->get_par() );
    if( usmpar ) {
      usmpar->set_radius( usm_radius.get() );
      usmpar->set_image_hints( in[0] );
      usmpar->set_format( get_format() );
      out = usmpar->build( in, first, imap, omap, level );
    }
    break;
  }
#ifndef PF_DISABLE_GMIC
  case PF::SHARPEN_DECONV: {
    GmicSharpenRLPar* rlpar = dynamic_cast<GmicSharpenRLPar*>( rl->get_par() );
    if( rlpar ) {
      rlpar->set_sigma( rl_sigma.get() );
      rlpar->set_iterations( rl_iterations.get() );
      rlpar->set_image_hints( in[0] );
      rlpar->set_format( get_format() );
      out = rlpar->build( in, first, imap, omap, level );
    }
    break;
  }
#endif
  default:
    break;
  }
  
  return out;
}


PF::ProcessorBase* PF::new_sharpen()
{
  return new PF::Processor<PF::SharpenPar,PF::SharpenProc>();
}
