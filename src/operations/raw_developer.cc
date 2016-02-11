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

#include "convertformat.hh"
#include "raw_preprocessor.hh"
#include "ca_correct.hh"
#include "amaze_demosaic.hh"
#include "lmmse_demosaic.hh"
#include "igv_demosaic.hh"
#include "fast_demosaic.hh"
#include "false_color_correction.hh"
#include "raw_output.hh"

#include "raw_developer.hh"


PF::RawDeveloperPar::RawDeveloperPar(): 
  OpParBase(), output_format( VIPS_FORMAT_NOTSET ),
  demo_method("demo_method",this,PF::PF_DEMO_AMAZE,"AMAZE","Amaze"),
	fcs_steps("fcs_steps",this,1),
	caching_enabled( true )
{
	demo_method.add_enum_value(PF::PF_DEMO_AMAZE,"AMAZE","Amaze");
	demo_method.add_enum_value(PF::PF_DEMO_FAST,"FAST","Fast");
  demo_method.add_enum_value(PF::PF_DEMO_LMMSE,"LMMSE","LMMSE");
  demo_method.add_enum_value(PF::PF_DEMO_IGV,"IGV","Igv");

  amaze_demosaic = new_amaze_demosaic();
  lmmse_demosaic = new_lmmse_demosaic();
  igv_demosaic = new_igv_demosaic();
  fast_demosaic = new_fast_demosaic();
  raw_preprocessor = new_raw_preprocessor();
  ca_correct = new_ca_correct();
  raw_output = new_raw_output();
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
	for(int ifcs = 0; ifcs < 4; ifcs++) 
		fcs[ifcs] = new_false_color_correction();

  map_properties( raw_preprocessor->get_par()->get_properties() );
  map_properties( ca_correct->get_par()->get_properties() );
  map_properties( raw_output->get_par()->get_properties() );

  set_type("raw_developer" );

  set_default_name( _("RAW developer") );
}



PF::wb_mode_t PF::RawDeveloperPar::get_wb_mode()
{
  PF::wb_mode_t result = PF::WB_CAMERA;
  PF::RawPreprocessorPar* par = dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
  if( par ) result = par->get_wb_mode();
  return result;
}


void PF::RawDeveloperPar::set_wb(float r, float g, float b)
{
  PF::RawPreprocessorPar* par = dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
  if( par ) par->set_wb(r,g,b);
}


void PF::RawDeveloperPar::get_wb(float* mul)
{
  PF::RawPreprocessorPar* par = dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
  if( par ) {
    mul[0] = par->get_wb_red();
    mul[1] = par->get_wb_green();
    mul[2] = par->get_wb_blue();
  }
}


VipsImage* PF::RawDeveloperPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;
  
  VipsImage* out_demo;
  std::vector<VipsImage*> in2;

  size_t blobsz;
  if( vips_image_get_blob( in[0], "raw_image_data",
			   (void**)&image_data, 
			   &blobsz ) ) {
    std::cout<<"RawDeveloperPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
	//std::cout<<"RawDeveloperPar::build(): blobsz="<<blobsz<<std::endl;
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"RawDeveloperPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }
  
  
  VipsImage* input_img = in[0];
	//std::cout<<"RawDeveloperPar::build(): input_img->Bands="<<input_img->Bands<<std::endl;
  if( input_img->Bands != 3 ) {
    raw_preprocessor->get_par()->set_image_hints( in[0] );
    raw_preprocessor->get_par()->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* image = raw_preprocessor->get_par()->build( in, 0, NULL, NULL, level );
    if( !image )
      return NULL;
  
    in2.push_back( image );
    ca_correct->get_par()->set_image_hints( image );
    ca_correct->get_par()->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* out_ca = ca_correct->get_par()->build( in2, 0, NULL, NULL, level );
    g_object_unref( image );
    //VipsImage* out_ca = image;

    in2.clear(); in2.push_back( out_ca );
		PF::ProcessorBase* demo = NULL;
		switch( demo_method.get_enum_value().first ) {
		case PF::PF_DEMO_FAST: demo = fast_demosaic; break;
		case PF::PF_DEMO_AMAZE: demo = amaze_demosaic; break;
    case PF::PF_DEMO_LMMSE: demo = lmmse_demosaic; break;
    case PF::PF_DEMO_IGV: demo = igv_demosaic; break;
		default: break;
		}
		//PF::ProcessorBase* demo = amaze_demosaic;
		//PF::ProcessorBase* demo = igv_demosaic;
		//PF::ProcessorBase* demo = fast_demosaic;
		if( !demo ) return NULL;
    demo->get_par()->set_image_hints( out_ca );
    demo->get_par()->set_format( VIPS_FORMAT_FLOAT );
    out_demo = demo->get_par()->build( in2, 0, NULL, NULL, level );
    g_object_unref( out_ca );

		for(int ifcs = 0; ifcs < VIPS_MIN(fcs_steps.get(),4); ifcs++) {
			VipsImage* temp = out_demo;
			in2.clear(); in2.push_back( temp );
			fcs[ifcs]->get_par()->set_image_hints( temp );
			fcs[ifcs]->get_par()->set_format( VIPS_FORMAT_FLOAT );
			out_demo = fcs[ifcs]->get_par()->build( in2, 0, NULL, NULL, level );
			PF_UNREF( temp, "RawDeveloperPar::build(): temp unref");
		}
  } else {
    raw_preprocessor->get_par()->set_image_hints( in[0] );
    raw_preprocessor->get_par()->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* image = raw_preprocessor->get_par()->build( in, 0, NULL, NULL, level );
    if( !image )
      return NULL;
  
    out_demo = image;
  }

  /**/
  raw_output->get_par()->set_image_hints( out_demo );
  raw_output->get_par()->set_format( VIPS_FORMAT_FLOAT );
  RawPreprocessorPar* rppar = dynamic_cast<RawPreprocessorPar*>( raw_preprocessor->get_par() );
  RawOutputPar* ropar = dynamic_cast<RawOutputPar*>( raw_output->get_par() );
  if( rppar && ropar ) {
    switch( rppar->get_wb_mode() ) {
    case WB_SPOT:
    case WB_COLOR_SPOT:
      ropar->set_wb( rppar->get_wb_red(), rppar->get_wb_green(), rppar->get_wb_blue() );
      break;
    default:
      ropar->set_wb( rppar->get_wb_red()*rppar->get_camwb_corr_red(),
          rppar->get_wb_green()*rppar->get_camwb_corr_green(),
          rppar->get_wb_blue()*rppar->get_camwb_corr_blue() );
      break;
    }
  }

  in2.clear(); in2.push_back( out_demo );
  VipsImage* out = raw_output->get_par()->build( in2, 0, NULL, NULL, level );
  g_object_unref( out_demo );
  /**/

  //VipsImage* gamma_in = out_demo;
  VipsImage* gamma_in = out;
  VipsImage* gamma = gamma_in;
  /*
  if( vips_gamma(gamma_in, &gamma, "exponent", (float)(2.2), NULL) ) {
    g_object_unref(gamma_in);
    return NULL;
  }
  */

  /*
  void *data;
  size_t data_length;
  cmsHPROFILE profile_in;
  if( gamma ) {
    if( !vips_image_get_blob( gamma, VIPS_META_ICC_NAME, 
                              &data, &data_length ) ) {
    
      profile_in = cmsOpenProfileFromMem( data, data_length );
      if( profile_in ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<"RawDeveloper::build(): convert_format input profile: "<<tstr<<std::endl;
        cmsCloseProfile( profile_in );
      }
    }  
  }
  */

  VipsImage* out2;
  std::vector<VipsImage*> in3;
  in3.push_back( gamma );
  convert_format->get_par()->set_image_hints( gamma );
  convert_format->get_par()->set_format( get_format() );
  out2 = convert_format->get_par()->build( in3, 0, NULL, NULL, level );
  g_object_unref( gamma );
  /*
  if( out2 ) {
    if( !vips_image_get_blob( out2, VIPS_META_ICC_NAME, 
                              &data, &data_length ) ) {
    
      profile_in = cmsOpenProfileFromMem( data, data_length );
      if( profile_in ) {
        char tstr[1024];
        cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
        std::cout<<"RawDeveloper::build(): convert_format output profile: "<<tstr<<std::endl;
        cmsCloseProfile( profile_in );
      }
    }  
  }
  */

//std::cout<<"RawDeveloper::build(): output image: "<<out2<<std::endl;
  return out2;
}


PF::ProcessorBase* PF::new_raw_developer()
{
  return new PF::Processor<PF::RawDeveloperPar,PF::RawDeveloper>();
}
