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

//#include <vips/cimg_funcs.h>

#include "../convertformat.hh"
#include "gmic.hh"

int vips_gmic(VipsImage **in, VipsImage** out, int n, int padding, double x_scale, double y_scale, const char* command, ...);


PF::GMicPar::GMicPar(): 
  OpParBase(),
  iterations("iterations",this,1),
  command("command",this,""),
  post_command("post_command",this,""),
  padding("padding",this,0),
  x_scale("x_scale",this,1), 
  y_scale("y_scale",this,1),
  cache_tiles( false )
{
  convert_format = PF::new_convert_format();	
  convert_format2 = PF::new_convert_format();	
  set_type( "gmic" );
}



VipsImage* PF::GMicPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) {
    std::cout<<"GMicPar::build(): null input image"<<std::endl;
    return NULL;
  }

  if( command.get().empty() ) {
    PF_REF( srcimg, "GMicPar::build(): empty command string" );
    return srcimg;
  }

  /*
	if( (get_render_mode() == PF_RENDER_PREVIEW && level>0) ) {
		PF_REF( out, "PF::GMicPar::build(): out ref" );
		return out;
	}
  */

  int target_ch = 0;
  PF::colorspace_t cs = PF::convert_colorspace( get_interpretation() );
  switch( cs ) {
  case PF_COLORSPACE_GRAYSCALE: break;
  case PF_COLORSPACE_RGB:
    target_ch = get_rgb_target_channel();
    break;
  case PF_COLORSPACE_LAB:
    target_ch = get_lab_target_channel();
    break;
  case PF_COLORSPACE_CMYK:
    target_ch = get_cmyk_target_channel();
    break;
  }

  VipsImage* srcimg2 = srcimg;
  if( (target_ch>=0) && (target_ch<srcimg->Bands) ) {
    if( vips_extract_band( srcimg, &srcimg2, target_ch, NULL ) ) {
      std::cout<<"GMicPar::build(): vips_extract_band() failed"<<std::endl;
      return NULL;
    }
    vips_image_init_fields( srcimg2,
                            get_xsize(), get_ysize(), 
                            1, get_format(),
                            get_coding(),
                            get_interpretation(),
                            1.0, 1.0);
  } else {
    PF_REF( srcimg2, "GMicPar::build(): srcimg2 ref" );
  }

  /**/
  std::vector<VipsImage*> in2;
  in2.push_back( srcimg2 );
  convert_format->get_par()->set_image_hints( srcimg2 );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  VipsImage* convimg = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
  if( !convimg ) {
    std::cout<<"GMicPar::build(): null convimg"<<std::endl;
    return NULL;
  }
  PF_UNREF( srcimg2, "GMicPar::build(): srcimg2 unref" );
  /**/
  //convimg = srcimg;
  //

  //return convimg;

  int tw = 128, th = 128, nt = 1000;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  bool threaded = true, persistent = false;
  /**/
  VipsImage* iter_in = convimg;
  VipsImage* iter_out = NULL;
  //std::cout<<"G'MIC command: "<<command.get()<<std::endl;
  std::string cmd = std::string("-verbose - ")+command.get();
  for( int i = 0; i < iterations.get(); i++ ) {
    // Memory caching of the padded image
    VipsImage* cached = iter_in;
    if( cache_tiles ) {
      if( vips_tilecache(iter_in, &cached,
          "tile_width", tw, "tile_height", th, "max_tiles", nt,
          "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
        std::cout<<"GMicPar::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( iter_in, "GaussBlurPar::build(): iter_in unref" );
    }

    VipsImage* inv[2] = { cached, NULL };
    if( vips_gmic( inv, &iter_out, 1,
                   padding.get(), x_scale.get(),
                   y_scale.get(),  
                   cmd.c_str(), NULL ) ) {
      std::cout<<"vips_gmic() failed!!!!!!!"<<std::endl;
      PF_UNREF( cached, "GMicPar::build(): vips_gmic() failed, iter_in unref" );
      return NULL;
    }
    PF_UNREF( cached, "GMicPar::build(): iter_in unref" );
    iter_in = iter_out;
  }
  /**/
  //return iter_out;

  //out = convimg;
  in2.clear();
  in2.push_back( iter_out );
  convert_format2->get_par()->set_image_hints( iter_out );
  convert_format2->get_par()->set_format( get_format() );
  VipsImage* out = convert_format2->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( iter_out, "GMicPar::build(): iter_out unref" );

  VipsImage* out2 = out;
  //std::cout<<"target_ch="<<target_ch<<"  srcimg->Bands="<<srcimg->Bands<<std::endl;
  if( (target_ch>=0) && (target_ch<(srcimg->Bands)) ) {
    //std::cout<<"Joining bands..."<<std::endl;
    VipsImage* bandv[4] = {NULL, NULL, NULL, NULL};
    bandv[target_ch] = out;
    for( int i = 0; i < srcimg->Bands; i++ ) {
      //std::cout<<"Processing band #"<<i<<std::endl;
      if( i==target_ch ) continue;
      VipsImage* band;
      if( vips_extract_band( srcimg, &band, i, NULL ) ) {
        std::cout<<"vips_extract_band( srcimg, &band, "<<i<<", NULL ) failed"<<std::endl;
        return NULL;
      }
      vips_image_init_fields( band,
                              get_xsize(), get_ysize(), 
                              1, get_format(),
                              get_coding(),
                              get_interpretation(),
                              1.0, 1.0);
      //std::cout<<"Extracted band #"<<i<<std::endl;
      bandv[i] = band;
    }
    if( vips_bandjoin( bandv, &out2, srcimg->Bands, NULL ) ) {
      //PF_UNREF( band, "ClonePar::rgb2rgb(): band unref after bandjoin failure" );
      return NULL;
    }
    for( int i = 0; i < srcimg->Bands; i++ ) {
      PF_UNREF( bandv[i], "GMicPar::build(): bandv[i] unref after bandjoin" );
    }
  }

	return out2;
}


PF::ProcessorBase* PF::new_gmic()
{
  return( new PF::Processor<PF::GMicPar,PF::GMicProc>() );
}
