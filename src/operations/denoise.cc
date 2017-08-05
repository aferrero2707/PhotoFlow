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

#include <vips/cimg_funcs.h>

#include "denoise.hh"
#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "../operations/impulse_nr.hh"
#include "../operations/nlmeans.hh"


PF::DenoisePar::DenoisePar(): 
  OpParBase(),
  impulse_nr_enable("impulse_nr_enable",this,false),
  impulse_nr_threshold("impulse_nr_threshold",this,50),
  nlmeans_enable("nlmeans_enable",this,false),
  nlmeans_radius("nlmeans_radius",this,2),
  nlmeans_strength("nlmeans_strength",this,50),
  nlmeans_luma_frac("nlmeans_luma_frac",this,0.5),
  nlmeans_chroma_frac("nlmeans_chroma_frac",this,1),
  iterations("iterations",this,1),
  amplitude("amplitude",this,100),
  sharpness("sharpness",this,0.9), 
  anisotropy("anisotropy",this,0.15),
  alpha("alpha",this,0.6),
  sigma("sigma",this,1.1),
	nr_mode("nr_mode",this,PF_NR_ANIBLUR,"ANIBLUR","Anisotropic Blur (G'Mic)"),
	in_profile( NULL )
{	
  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert2input = new_icc_transform();
  impulse_nr = PF::new_impulse_nr();
  nlmeans = PF::new_nlmeans();
  set_type( "denoise" );

  set_default_name( _("noise reduction") );
}



VipsImage* PF::DenoisePar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;

  PF_REF( out, "PF::DenoisePar::build(): out ref" );
  if( (get_render_mode() == PF_RENDER_PREVIEW && level>0) ) {
    return out;
  }

  if( !impulse_nr_enable.get() && !nlmeans_enable.get() ) {
    return out;
  }

  void *data;
  size_t data_length;

  if( in_profile ) cmsCloseProfile( in_profile );

  in_profile = NULL;
  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
                           &data, &data_length ) ) {
    in_profile = cmsOpenProfileFromMem( data, data_length );
  }

  std::vector<VipsImage*> in2;

  convert2lab->get_par()->set_image_hints( out );
  convert2lab->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( out );
  VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
  if( !labimg ) {
    std::cout<<"DenoisePar::build(): null Lab image"<<std::endl;
    return out;
  }
  PF_UNREF( out, "DenoisePar::build(): out unref after Lab conversion" );
  out = labimg;

  if( impulse_nr_enable.get() && impulse_nr && impulse_nr->get_par() ) {
    PF::ImpulseNRPar* imnrpar = dynamic_cast<PF::ImpulseNRPar*>(impulse_nr->get_par());
    if( !imnrpar ) {
      std::cout<<"DenoisePar::build(): failed to cast to ImpulseNRPar*"<<std::endl;
      //PF_REF( out, "PF::DenoisePar::build(): out ref" );
      return out;
    }
    imnrpar->set_threshold( impulse_nr_threshold.get() );
    imnrpar->set_image_hints( out );
    imnrpar->set_format( get_format() );
    in2.clear(); in2.push_back( out );
    VipsImage* imnrimg = imnrpar->build( in2, 0, NULL, NULL, level );
    PF_UNREF( out, "DenoisePar::build(): out unref after imnrpar->build()" );
    out = imnrimg;
  }

  std::cout<<"DenoisePar::build(): nlmeans_enable.get()="<<nlmeans_enable.get()<<std::endl;
  if( nlmeans_enable.get() && nlmeans && nlmeans->get_par() ) {
    PF::NonLocalMeansPar* nrpar = dynamic_cast<PF::NonLocalMeansPar*>(nlmeans->get_par());
    if( !nrpar ) {
      std::cout<<"DenoisePar::build(): failed to cast to NonLocalMeansPar*"<<std::endl;
      //PF_REF( out, "PF::DenoisePar::build(): out ref" );
      return out;
    }
    nrpar->set_radius( nlmeans_radius.get() );
    nrpar->set_strength( nlmeans_strength.get() );
    nrpar->set_luma_frac( nlmeans_luma_frac.get() );
    nrpar->set_chroma_frac( nlmeans_chroma_frac.get() );
    nrpar->set_image_hints( out );
    nrpar->set_format( get_format() );
    in2.clear(); in2.push_back( out );
    VipsImage* nrimg = nrpar->build( in2, 0, NULL, NULL, level );
    PF_UNREF( out, "DenoisePar::build(): out unref after nrpar->build()" );
    out = nrimg;
  }


  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2input->get_par() );
  //std::cout<<"ImageArea::update(): icc_par="<<icc_par<<std::endl;
  if( icc_par ) {
    //std::cout<<"ImageArea::update(): setting display profile: "<<current_display_profile<<std::endl;
    icc_par->set_out_profile( in_profile );
  }
  convert2input->get_par()->set_image_hints( out );
  convert2input->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( out );
  std::cout<<"DenoisePar::build(): calling convert2input->get_par()->build()"<<std::endl;
  VipsImage* out2 = convert2input->get_par()->build(in2, 0, NULL, NULL, level );
  PF_UNREF( out, "DenoisePar::update() out unref" );


	/*
  int fast_approx = 0;
	if( (get_render_mode() == PF_RENDER_PREVIEW && level>0) ) 
    fast_approx = 1;
  out = vips_image_new();
  if( im_greyc_mask( srcimg, out, NULL, 
                     iterations.get(), amplitude.get(),
                     sharpness.get(), anisotropy.get(),
                     alpha.get(), sigma.get(), 0.8, 30, 2, 1, fast_approx ) )
    return NULL;
  */

	return out2;
}


PF::ProcessorBase* PF::new_denoise()
{
  return( new PF::Processor<PF::DenoisePar,PF::DenoiseProc>() );
}
