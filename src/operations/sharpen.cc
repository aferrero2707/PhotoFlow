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
#include "gmic/sharpen_texture.hh"
#include "sharpen.hh"


PF::SharpenPar::SharpenPar(): 
  OpParBase(), 
  method("method",this,PF::SHARPEN_USM,"USM","Unsharp Mask"),
  usm_radius("usm_radius",this,1),
  rl_sigma("rl_sigma",this,1),
  rl_iterations("rl_iterations",this,10),
  texture_radius("texture_radius",this,4),
  texture_strength("texture_strength",this,1)
{
  //method.add_enum_value(PF::SHARPEN_USM,"USM","Unsharp Mask");
#ifndef PF_DISABLE_GMIC
  method.add_enum_value(PF::SHARPEN_DECONV,"DECONV","RL Deconvolution");
  method.add_enum_value(PF::SHARPEN_TEXTURE,"TEXTURE","texture");
#endif
  //method.add_enum_value(PF::SHARPEN_MICRO,"MICRO","Micro Contrast");

  usm = new_unsharp_mask();
#ifndef PF_DISABLE_GMIC
  rl = new_gmic_sharpen_rl();
  texture = new_gmic_sharpen_texture();
#endif

  set_type("sharpen" );

  set_default_name( _("sharpening") );
}


PF::SharpenPar::~SharpenPar()
{
  std::cout<<"~SharpenPar(): delete usm"<<std::endl;
  delete usm ;
#ifndef PF_DISABLE_GMIC
  std::cout<<"~SharpenPar(): delete rl"<<std::endl;
  delete rl;
  std::cout<<"~SharpenPar(): delete texture"<<std::endl;
  delete texture;
#endif
}


bool PF::SharpenPar::needs_caching()
{
  switch( method.get_enum_value().first ) {
  case PF::SHARPEN_USM:
    return false; break;
#ifndef PF_DISABLE_GMIC
  case PF::SHARPEN_DECONV:
    return true; break;
  case PF::SHARPEN_TEXTURE:
    return true; break;
#endif
  default:
    return false; break;
  }
}


void PF::SharpenPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  std::cout<<"SharpenPar::compute_padding(): method.get_enum_value().first="<<method.get_enum_value().first<<std::endl;
  switch( method.get_enum_value().first ) {
  case PF::SHARPEN_USM:
    g_assert(usm->get_par() != NULL);
    usm->get_par()->compute_padding(full_res, id, level);
    set_padding( usm->get_par()->get_padding(id), id );
    break;
#ifndef PF_DISABLE_GMIC
  case PF::SHARPEN_DECONV:
    g_assert(rl->get_par() != NULL);
    rl->get_par()->compute_padding(full_res, id, level);
    set_padding( rl->get_par()->get_padding(id), id );
    break;
  case PF::SHARPEN_TEXTURE:
    g_assert(texture->get_par() != NULL);
    texture->get_par()->compute_padding(full_res, id, level);
    set_padding( texture->get_par()->get_padding(id), id );
    break;
#endif
  default: break;
  }
}



void PF::SharpenPar::propagate_settings()
{
  UnsharpMaskPar* usmpar = dynamic_cast<UnsharpMaskPar*>( usm->get_par() );
  if( usmpar ) {
    std::cout<<"SharpenPar::propagate_settings(): usm_radius="<<usm_radius.get()<<std::endl;
    usmpar->set_radius( usm_radius.get() );
    usmpar->propagate_settings();
  }

  GmicSharpenRLPar* rlpar = dynamic_cast<GmicSharpenRLPar*>( rl->get_par() );
  if( rlpar ) {
    rlpar->set_sigma( rl_sigma.get() );
    rlpar->set_iterations( rl_iterations.get() );
    rlpar->propagate_settings();
  }

  GmicSharpenTexturePar* tpar = dynamic_cast<GmicSharpenTexturePar*>( texture->get_par() );
  if( tpar ) {
    tpar->set_radius( texture_radius.get() );
    tpar->set_strength( texture_strength.get() );
    tpar->propagate_settings();
  }
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
      rlpar->set_image_hints( in[0] );
      rlpar->set_format( get_format() );
      out = rlpar->build( in, first, imap, omap, level );
    }
    break;
  }
  case PF::SHARPEN_TEXTURE: {
    GmicSharpenTexturePar* par = dynamic_cast<GmicSharpenTexturePar*>( texture->get_par() );
    if( par ) {
      par->set_image_hints( in[0] );
      par->set_format( get_format() );
      out = par->build( in, first, imap, omap, level );
    }
    break;
  }
#endif
  default:
    break;
  }
  
  return out;
}
