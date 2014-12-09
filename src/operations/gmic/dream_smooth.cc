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


#include "../base/pf_mkstemp.hh"

#include "gmic.hh"
#include "dream_smooth.hh"



PF::GmicDreamSmoothPar::GmicDreamSmoothPar(): 
  OpParBase(),
  iterations("iterations",this,1),
  prop_interations("smooth_interations",this,1),
  prop_equalize("equalize",this,0),
  prop_merging_option("merging_option", this, 1, "alpha", "alpha"),
  prop_opacity("opacity",this,0.8),
  prop_reverse("reverse",this,0),
  prop_smoothness("smoothness",this,0.8),
  padding("padding",this,0),
  do_update( true ),
  raster_image( NULL )
{	
  convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  convert_format2 = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();

  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
  snprintf( fname, 499, "%s\gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  struct stat buffer;   
  int stat_result = stat( fname, &buffer );
  if( stat_result != 0 ) {
    fname[0] = 0;
  }
#else
  if( getenv("HOME") ) {
    //snprintf( fname, 499, "%s/.photoflow/gmic_update.gmic", getenv("HOME") );
    snprintf( fname, 499, "%s/share/photoflow/gmic_def.gmic", INSTALL_PREFIX );
    std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
    struct stat buffer;   
    int stat_result = stat( fname, &buffer );
    if( stat_result != 0 ) {
      //snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
      //stat_result = stat( fname, &buffer );
      //if( stat_result != 0 ) {
      fname[0] = 0;
      //}
    }
  }
#endif
  if( strlen( fname ) > 0 ) {
    std::ifstream t;
    int length;
    t.open(fname);      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    custom_gmic_commands = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(custom_gmic_commands, length);       // read the whole file into the buffer
    t.close();                    // close file handle
    std::cout<<"G'MIC custom commands loaded"<<std::endl;
  }

  /* Make a gmic for this thread.
   */
  gmic_instance = new gmic( 0, custom_gmic_commands, false, 0, 0 ); 


  //gmic = PF::new_gmic();
  prop_merging_option.add_enum_value( 0, "add", "add" );
  prop_merging_option.add_enum_value( 2, "and", "and" );
  prop_merging_option.add_enum_value( 3, "average", "average" );
  prop_merging_option.add_enum_value( 4, "blue", "blue" );
  prop_merging_option.add_enum_value( 5, "burn", "burn" );
  prop_merging_option.add_enum_value( 6, "darken", "darken" );
  prop_merging_option.add_enum_value( 7, "difference", "difference" );
  prop_merging_option.add_enum_value( 8, "divide", "divide" );
  prop_merging_option.add_enum_value( 9, "dodge", "dodge" );
  prop_merging_option.add_enum_value( 10, "exclusion", "exclusion" );
  prop_merging_option.add_enum_value( 11, "freeze", "freeze" );
  prop_merging_option.add_enum_value( 12, "grainextract", "grainextract" );
  prop_merging_option.add_enum_value( 13, "grainmerge", "grainmerge" );
  prop_merging_option.add_enum_value( 14, "green", "green" );
  prop_merging_option.add_enum_value( 15, "hardlight", "hardlight" );
  prop_merging_option.add_enum_value( 16, "hardmix", "hardmix" );
  prop_merging_option.add_enum_value( 17, "hue", "hue" );
  prop_merging_option.add_enum_value( 18, "interpolation", "interpolation" );
  prop_merging_option.add_enum_value( 19, "lighten", "lighten" );
  prop_merging_option.add_enum_value( 20, "lightness", "lightness" );
  prop_merging_option.add_enum_value( 21, "linearburn", "linearburn" );
  prop_merging_option.add_enum_value( 22, "linearlight", "linearlight" );
  prop_merging_option.add_enum_value( 23, "luminance", "luminance" );
  prop_merging_option.add_enum_value( 24, "multiply", "multiply" );
  prop_merging_option.add_enum_value( 25, "negation", "negation" );
  prop_merging_option.add_enum_value( 26, "or", "or" );
  prop_merging_option.add_enum_value( 27, "overlay", "overlay" );
  prop_merging_option.add_enum_value( 28, "pinlight", "pinlight" );
  prop_merging_option.add_enum_value( 29, "red", "red" );
  prop_merging_option.add_enum_value( 30, "reflect", "reflect" );
  prop_merging_option.add_enum_value( 31, "saturation", "saturation" );
  prop_merging_option.add_enum_value( 32, "screen", "screen" );
  prop_merging_option.add_enum_value( 33, "shapeaverage", "shapeaverage" );
  prop_merging_option.add_enum_value( 34, "softburn", "softburn" );
  prop_merging_option.add_enum_value( 35, "softdodge", "softdodge" );
  prop_merging_option.add_enum_value( 36, "softlight", "softlight" );
  prop_merging_option.add_enum_value( 37, "stamp", "stamp" );
  prop_merging_option.add_enum_value( 38, "subtract", "subtract" );
  prop_merging_option.add_enum_value( 39, "value", "value" );
  prop_merging_option.add_enum_value( 40, "vividlight", "vividlight" );
  prop_merging_option.add_enum_value( 41, "xor", "xor" );
  prop_merging_option.add_enum_value( 42, "edges", "edges" );
  set_type( "gmic_dream_smooth" );
}


PF::GmicDreamSmoothPar::~GmicDreamSmoothPar()
{
	std::cout<<"GmicDreamSmoothPar::~GmicDreamSmoothPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"GmicDreamSmoothPar::~GmicDreamSmoothPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"GmicDreamSmoothPar::~GmicDreamSmoothPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
}


bool PF::GmicDreamSmoothPar::import_settings( OpParBase* pin )
{
  GmicDreamSmoothPar* par = dynamic_cast<GmicDreamSmoothPar*>( pin );
  if( !par ) return false;
  preview_cache_file_name = par->get_preview_cache_file_name();
  render_cache_file_name = par->get_render_cache_file_name();

  return( PF::OpParBase::import_settings(pin) );
}


int PF::GmicDreamSmoothPar::get_padding( int level )
{
  return( padding.get() );
}


void PF::GmicDreamSmoothPar::pre_build( rendermode_t mode )
{
  if( mode==PF_RENDER_PREVIEW ) {
    if( !do_update ) return;
    
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicDreamSmoothPar::pre_build(): cache file="<<fname<<std::endl;
    preview_cache_file_name = fname;
  } else {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicDreamSmoothPar::pre_build(): cache file="<<fname<<std::endl;
    render_cache_file_name = fname;
  }

  do_update = false;
}


VipsImage* PF::GmicDreamSmoothPar::build(std::vector<VipsImage*>& in, int first, 
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) return NULL;

  std::string cache_file_name;
  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    std::cout<<"GmicDreamSmoothPar::build() setting cache file name to preview_cache_file_name ("
             <<preview_cache_file_name<<")"<<std::endl;
    cache_file_name = preview_cache_file_name;
  } else { 
    std::cout<<"GmicDreamSmoothPar::build() setting cache file name to render_cache_file_name ("
             <<render_cache_file_name<<")"<<std::endl;
    cache_file_name = render_cache_file_name;
  }
  std::cout<<"GmicDreamSmoothPar::build() render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;

  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i == raster_images.end() ) {

    std::cout<<"GmicDreamSmoothPar::build(): raster_image="<<(void*)raster_image<<std::endl;
    if( raster_image ) {
      raster_image->unref();
      std::cout<<"GmicDreamSmoothPar::build(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
      if( raster_image->get_nref() == 0 ) {
        std::map<Glib::ustring, RasterImage*>::iterator i = 
          raster_images.find( cache_file_name );
        if( i != raster_images.end() ) 
          raster_images.erase( i );
        delete raster_image;
			std::cout<<"GmicDreamSmoothPar::build(): raster_image deleted"<<std::endl;
			raster_image = 0;
      }
    }

    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    std::cout<<"GmicDreamSmoothPar: temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
    if( temp_fd < 0 ) return NULL;

    std::vector<VipsImage*> in2;
    in2.push_back( srcimg );
    convert_format->get_par()->set_image_hints( srcimg );
    convert_format->get_par()->set_format( IM_BANDFMT_FLOAT );
    VipsImage* floatimg = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
    if( !floatimg ) return NULL;

#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
    vips_image_write_to_file( floatimg, fname );
#else
    vips_image_write_to_file( floatimg, fname, NULL );
#endif
    PF_UNREF( floatimg, "GmicDreamSmoothPar::build(): after write_to_file" );

    gmic_list<float> images;
    gmic_list<char> images_names;

    std::string command = "-verbose - -input ";
    command = command + std::string(fname) + std::string(" -n 0,255 -gimp_dreamsmooth ");
    command = command + prop_interations.get_str();
    command = command + std::string(",") + prop_equalize.get_str();
    command = command + std::string(",") + prop_merging_option.get_enum_value_str();
    command = command + std::string(",") + prop_opacity.get_str();
    command = command + std::string(",") + prop_reverse.get_str();
    command = command + std::string(",") + prop_smoothness.get_str() + ",1,0 -n 0,1 -output " + cache_file_name + ",float,lzw";
    std::cout<<"dream smooth command: "<<command<<std::endl;

    gmic_instance->run( command.c_str() );
    close( temp_fd );
    unlink( fname );

    raster_image = new RasterImage( cache_file_name );
    raster_images.insert( make_pair(cache_file_name, raster_image) );
  } else {
    std::cout<<"GmicDreamSmoothPar::build(): raster_image found ("<<cache_file_name<<")"<<std::endl;
    raster_image = i->second;
    raster_image->ref();
  }
  if( !raster_image )
    return NULL;

  VipsImage* image = raster_image->get_image( level );
  
  if( !image ) return NULL;

  VipsImage* out = image;
  if( (get_format() != image->BandFmt) ) {
    std::vector<VipsImage*> in2;
    in2.push_back( image );
    convert_format2->get_par()->set_image_hints( image );
    convert_format2->get_par()->set_format( get_format() );
    out = convert_format2->get_par()->build( in2, 0, NULL, NULL, level );
    //std::cout<<"ImageReaderPar::build(): out ("<<(void*)out<<") refcount after convert_format: "<<G_OBJECT(out)->ref_count<<std::endl;
    //g_object_unref( image );
    PF_UNREF( image, "ImageReaderPar::build(): image unref after convert_format" );
  }

  if( out ) {
    //PF_REF( image, "GmicDreamSmoothPar::build()" );
    set_image_hints( out );
  }
  return out;

  /*
  if( !(gmic->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
	for( int l = 1; l <= level; l++ )
		scalefac *= 2;

  std::string command = "-gimp_dreamsmooth  ";
  command = command + prop_interations.get_str();
  command = command + std::string(",") + prop_equalize.get_str();
  command = command + std::string(",") + prop_merging_option.get_enum_value_str();
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_reverse.get_str();
  command = command + std::string(",") + prop_smoothness.get_str() + ",1," + padding.get_str();
  std::cout<<"drawm smooth command: "<<command<<std::endl;
  gpar->set_command( command.c_str() );
  gpar->set_iterations( iterations.get() );
  gpar->set_padding( get_padding( level ) );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
  */
}


PF::ProcessorBase* PF::new_gmic_dream_smooth()
{
  return( new PF::Processor<PF::GmicDreamSmoothPar,PF::GmicDreamSmoothProc>() );
}
