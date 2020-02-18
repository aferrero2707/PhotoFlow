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
#include "rcd_demosaic.hh"
#include "no_demosaic.hh"
#include "xtrans_demosaic.hh"
#include "fast_demosaic.hh"
#include "fast_demosaic_xtrans.hh"
#include "false_color_correction.hh"
#include "lensfun.hh"
#include "raw_hl_reco.hh"
#include "raw_output.hh"
#include "hotpixels.hh"

#include "raw_developer.hh"


PF::RawDeveloperPar::RawDeveloperPar(): 
  OpParBase(), output_format( VIPS_FORMAT_NOTSET ),
  lf_prop_camera_maker( "lf_camera_maker", this ),
  lf_prop_camera_model( "lf_camera_model", this ),
  lf_prop_lens( "lf_lens", this ),
  enable_distortion( "lf_enable_distortion", this, false ),
  enable_tca( "lf_enable_tca", this, false ),
  enable_vignetting( "lf_enable_vignetting", this, false ),
  enable_all( "lf_enable_all", this, false ),
  auto_crop( "lf_auto_crop", this, false ),
  tca_method("tca_method",this,PF::PF_TCA_CORR_AUTO,"TCA_CORR_AUTO",_("auto")),
  demo_method("demo_method",this,PF::PF_DEMO_AMAZE,"AMAZE","Amaze"),
	fcs_steps("fcs_steps",this,0),
  hlreco_mode("hlreco_mode",this,PF::HLRECO_CLIP,"HLRECO_CLIP",_("clip")),
	caching_enabled( true )
{
  tca_method.add_enum_value(PF::PF_TCA_CORR_AUTO,"TCA_CORR_AUTO",_("auto"));
  tca_method.add_enum_value(PF::PF_TCA_CORR_PROFILED,"TCA_CORR_PROFILED",_("profiled"));
  tca_method.add_enum_value(PF::PF_TCA_CORR_PROFILED_AUTO,"TCA_CORR_PROFILED_AUTO",_("profiled + auto"));
  //tca_method.add_enum_value(PF::PF_TCA_CORR_MANUAL,"TCA_CORR_MANUAL",_("manual"));

  demo_method.add_enum_value(PF::PF_DEMO_AMAZE,"AMAZE","Amaze");
  demo_method.add_enum_value(PF::PF_DEMO_RCD,"RCD","RCD");
  //demo_method.add_enum_value(PF::PF_DEMO_FAST,"FAST","Fast");
  demo_method.add_enum_value(PF::PF_DEMO_LMMSE,"LMMSE","LMMSE");
  demo_method.add_enum_value(PF::PF_DEMO_IGV,"IGV","Igv");
  //demo_method.add_enum_value(PF::PF_DEMO_NONE,"NONE","RAW");

  hlreco_mode.add_enum_value(PF::HLRECO_BLEND,"HLRECO_BLEND",_("blend"));
  hlreco_mode.add_enum_value(PF::HLRECO_NONE,"HLRECO_NONE",_("none"));

  amaze_demosaic = new_amaze_demosaic();
  lmmse_demosaic = new_lmmse_demosaic();
  igv_demosaic = new_igv_demosaic();
  rcd_demosaic = new_rcd_demosaic();
  no_demosaic = new_no_demosaic();
  xtrans_demosaic = new_xtrans_demosaic();
  fast_demosaic = new_fast_demosaic();
  fast_demosaic_xtrans = new_fast_demosaic_xtrans();
  FastDemosaicXTransPar* xtrans_par =
      dynamic_cast<FastDemosaicXTransPar*>(fast_demosaic_xtrans->get_par());
  if( xtrans_par ) xtrans_par->set_normalize( true );
  raw_preprocessor = new_raw_preprocessor();
  ca_correct = new_ca_correct();
  hl_reco = new_raw_hl_reco();
  lensfun = new_lensfun();
  raw_output = new_raw_output();
  hotpixels = new_hotpixels();
  convert_format = new_convert_format();
	for(int ifcs = 0; ifcs < 4; ifcs++) 
		fcs[ifcs] = new_false_color_correction();

  map_properties( raw_preprocessor->get_par()->get_properties() );
  map_properties( ca_correct->get_par()->get_properties() );
  map_properties( raw_output->get_par()->get_properties() );
  map_properties( hotpixels->get_par()->get_properties() );
  map_properties( lensfun->get_par()->get_properties() );

  set_type("raw_developer_v2" );

  set_default_name( _("RAW developer") );
}


bool PF::RawDeveloperPar::is_editing_locked()
{
  PF::wb_mode_t wbmode = get_wb_mode();
  if( (wbmode == WB_SPOT) || (wbmode == WB_COLOR_SPOT) || (wbmode == WB_AREA_SPOT) )
    return true;
  return false;
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


void PF::RawDeveloperPar::add_wb_area(std::vector<int>& area)
{
  PF::RawPreprocessorPar* par = dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
  g_assert( par != NULL );
  return par->add_wb_area(area);
}


std::vector< std::vector<int> >& PF::RawDeveloperPar::get_wb_areas()
{
  PF::RawPreprocessorPar* par = dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
  g_assert( par != NULL );
  return par->get_wb_areas();
}


int PF::RawDeveloperPar::get_hotp_fixed()
{
  int result = 0;
  PF::HotPixelsPar* par = dynamic_cast<PF::HotPixelsPar*>( hotpixels->get_par() );
  if( par ) result = par->get_pixels_fixed();
  return result;
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


std::string PF::RawDeveloperPar::get_lf_maker()
{
  std::string result;
  LensFunPar* lfpar = dynamic_cast<LensFunPar*>( lensfun->get_par() );
  if( !lfpar ) {
    std::cout<<"RawDeveloperPar::build(): could not get LensFunPar object."<<std::endl;
    return result;
  }
  return lfpar->camera_maker();
}


std::string PF::RawDeveloperPar::get_lf_model()
{
  std::string result;
  LensFunPar* lfpar = dynamic_cast<LensFunPar*>( lensfun->get_par() );
  if( !lfpar ) {
    std::cout<<"RawDeveloperPar::build(): could not get LensFunPar object."<<std::endl;
    return result;
  }
  return lfpar->camera_model();
}


std::string PF::RawDeveloperPar::get_lf_lens()
{
  std::string result;
  LensFunPar* lfpar = dynamic_cast<LensFunPar*>( lensfun->get_par() );
  if( !lfpar ) {
    std::cout<<"RawDeveloperPar::build(): could not get LensFunPar object."<<std::endl;
    return result;
  }
  return lfpar->lens();
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
  if( PF_VIPS_IMAGE_GET_BLOB( in[0], "raw_image_data", &image_data, &blobsz ) ) {
    std::cout<<"RawDeveloperPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
	//std::cout<<"RawDeveloperPar::build(): blobsz="<<blobsz<<std::endl;
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"RawDeveloperPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }
  
  
  LensFunPar* lfpar = dynamic_cast<LensFunPar*>( lensfun->get_par() );
  if( !lfpar ) {
    std::cout<<"RawDeveloperPar::build(): could not get LensFunPar object."<<std::endl;
    return NULL;
  }
  int lf_modflags = lfpar->get_flags( in[0] );

  VipsImage* input_img = in[0];
  //std::cout<<"RawDeveloperPar::build(): input_img->Bands="<<input_img->Bands<<std::endl;
  if( input_img->Bands != 3 ) {
    raw_preprocessor->get_par()->set_image_hints( in[0] );
    raw_preprocessor->get_par()->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* image = raw_preprocessor->get_par()->build( in, 0, NULL, NULL, level );
    //VipsImage* image = in[0]; PF_REF( image, "");
    if( !image )
      return NULL;

    VipsImage* out_ca = NULL;
    PF::ProcessorBase* demo = NULL;
    if( PF::check_xtrans(image_data->idata.filters) ) {
      out_ca = image;
      //PF_REF( out_ca, "RawDeveloperPar::build(): in[0] ref for xtrans");
      //demo = fast_demosaic_xtrans;
      demo = xtrans_demosaic;
    } else {
      in2.push_back( image );
      hotpixels->get_par()->set_image_hints( image );
      hotpixels->get_par()->set_format( VIPS_FORMAT_FLOAT );
      HotPixelsPar* hppar = dynamic_cast<HotPixelsPar*>( hotpixels->get_par() );
      hppar->set_pixels_fixed( 0 );
      VipsImage* out_hotp = hotpixels->get_par()->build( in2, 0, NULL, NULL, level );
      g_object_unref( image );
      //VipsImage* out_hotp = image;

      CACorrectPar* capar = dynamic_cast<CACorrectPar*>( ca_correct->get_par() );
      if( !capar ) {
        std::cout<<"RawDeveloperPar::build(): could not get CACorrectPar object."<<std::endl;
        return NULL;
      }

      capar->set_enable_ca( false );
      capar->set_auto_ca( false );
      if( enable_tca.get() || enable_all.get() ) {
        switch(tca_method.get_enum_value().first) {
        case PF::PF_TCA_CORR_AUTO:
          capar->set_enable_ca( true );
          capar->set_auto_ca( true );
          break;
        case PF::PF_TCA_CORR_PROFILED_AUTO:
          if( (lf_modflags & LF_MODIFY_TCA) == 0) {
            capar->set_enable_ca( true );
            capar->set_auto_ca( true );
          }
          break;
        case PF::PF_TCA_CORR_MANUAL:
          capar->set_enable_ca( true );
          capar->set_auto_ca( false );
          break;
        default:
          break;
        }
      }

      in2.clear(); in2.push_back( out_hotp );
      ca_correct->get_par()->set_image_hints( out_hotp );
      ca_correct->get_par()->set_format( VIPS_FORMAT_FLOAT );
      out_ca = ca_correct->get_par()->build( in2, 0, NULL, NULL, level );
      g_object_unref( out_hotp );
      //VipsImage* out_ca = out_hotp;

      switch( demo_method.get_enum_value().first ) {
      case PF::PF_DEMO_FAST: demo = fast_demosaic; break;
      case PF::PF_DEMO_AMAZE: demo = amaze_demosaic; break;
      case PF::PF_DEMO_LMMSE: demo = lmmse_demosaic; break;
      case PF::PF_DEMO_IGV: demo = igv_demosaic; break;
      case PF::PF_DEMO_RCD: demo = rcd_demosaic; break;
      case PF::PF_DEMO_NONE: demo = no_demosaic; break;
      default: break;
      }
      //PF::ProcessorBase* demo = amaze_demosaic;
      //PF::ProcessorBase* demo = igv_demosaic;
      //PF::ProcessorBase* demo = fast_demosaic;
    }
    if( !demo ) return NULL;
    in2.clear(); in2.push_back( out_ca );
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



  RawPreprocessorPar* rppar = dynamic_cast<RawPreprocessorPar*>( raw_preprocessor->get_par() );

  RawHLRecoPar* hlpar = dynamic_cast<RawHLRecoPar*>( hl_reco->get_par() );
  if( !hlpar ) {
    std::cout<<"RawDeveloperPar::build(): could not get RawHLReco object."<<std::endl;
    return NULL;
  }
  if( rppar && hlpar ) {
    switch( rppar->get_wb_mode() ) {
    case WB_SPOT:
    case WB_COLOR_SPOT:
      hlpar->set_wb( rppar->get_wb_red(),
          rppar->get_wb_green(),
          rppar->get_wb_blue() );
      break;
    default:
      hlpar->set_wb( rppar->get_wb_red()*rppar->get_camwb_corr_red(),
          rppar->get_wb_green()*rppar->get_camwb_corr_green(),
          rppar->get_wb_blue()*rppar->get_camwb_corr_blue() );
      break;
    }
  }
  hlpar->set_hlreco_mode((hlreco_mode_t)hlreco_mode.get_enum_value().first);
  hlpar->set_image_hints(out_demo);
  hlpar->set_format( VIPS_FORMAT_FLOAT );
  in2.clear(); in2.push_back( out_demo );
  VipsImage* out_hl = hlpar->build( in2, 0, NULL, NULL, level );
  g_object_unref( out_demo );


  /**/
  lensfun->get_par()->set_image_hints( out_hl );
  lensfun->get_par()->set_format( VIPS_FORMAT_FLOAT );
  lfpar->set_auto_crop_enabled( auto_crop.get() );
  lfpar->set_vignetting_enabled( enable_vignetting.get() || enable_all.get() );
  lfpar->set_distortion_enabled( enable_distortion.get() || enable_all.get() );
  lfpar->set_tca_enabled( false );
  if( enable_tca.get() || enable_all.get() ) {
    switch(tca_method.get_enum_value().first) {
    case PF::PF_TCA_CORR_PROFILED:
      lfpar->set_tca_enabled( true );
      break;
    case PF::PF_TCA_CORR_PROFILED_AUTO:
      if( lf_modflags & LF_MODIFY_TCA )
        lfpar->set_tca_enabled( true );
      break;
    default:
      break;
    }
  }
  in2.clear(); in2.push_back( out_hl );
  VipsImage* out_lf = lensfun->get_par()->build( in2, 0, NULL, NULL, level );
  g_object_unref( out_hl );


  raw_output->get_par()->set_image_hints( out_lf );
  raw_output->get_par()->set_format( VIPS_FORMAT_FLOAT );
  RawOutputPar* ropar = dynamic_cast<RawOutputPar*>( raw_output->get_par() );
  if( rppar && ropar ) {
    switch( rppar->get_wb_mode() ) {
    case WB_SPOT:
    case WB_COLOR_SPOT:
      ropar->set_wb( rppar->get_wb_red(),
          rppar->get_wb_green(),
          rppar->get_wb_blue() );
      break;
    default:
      ropar->set_wb( rppar->get_wb_red()*rppar->get_camwb_corr_red(),
          rppar->get_wb_green()*rppar->get_camwb_corr_green(),
          rppar->get_wb_blue()*rppar->get_camwb_corr_blue() );
      break;
    }
    rppar->set_hlreco_mode( (hlreco_mode_t)hlreco_mode.get_enum_value().first );
    ropar->set_hlreco_mode( (hlreco_mode_t)hlreco_mode.get_enum_value().first );
  }
  //ropar->set_output_gain(lfpar->get_gain_vignetting());

  in2.clear(); in2.push_back( out_lf );
  VipsImage* out = raw_output->get_par()->build( in2, 0, NULL, NULL, level );
  g_object_unref( out_lf );
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


  //std::cout<<"RawDeveloperPar::build(): before vips_resize()"<<std::endl;
  VipsImage* scaled = out2;
  if( false && level > 0 ) {
    float scale = 1;
    for(unsigned int l = 0; l < level; l++) scale /= 2;
    VipsKernel kernel = VIPS_KERNEL_CUBIC;
    if( vips_resize(out2, &scaled, scale, "kernel", kernel, NULL) ) {
      std::cout<<"RawDeveloperPar::build(): vips_resize() failed."<<std::endl;
      //PF_UNREF( interpolate, "ScalePar::build(): interpolate unref" );
      return NULL;
    }
    PF_UNREF(out2, "RawDeveloperPar::build(): out2 unref after vips_resize()");
  }
  //std::cout<<"RawDeveloperPar::build(): after vips_resize()"<<std::endl;


  set_image_hints( scaled );
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
  return scaled;
}
