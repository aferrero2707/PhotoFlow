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
#include "gmic_untiled_op.hh"



PF::GmicUntiledOperationPar::GmicUntiledOperationPar(): 
  OpParBase(),
  gmic_instance( NULL ),
  do_update( true ),
  raster_image( NULL )
{	
  convert_format_in = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  convert_format_out = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
}


PF::GmicUntiledOperationPar::~GmicUntiledOperationPar()
{
	std::cout<<"GmicUntiledOperationPar::~GmicUntiledOperationPar(): raster_image="<<(void*)raster_image<<std::endl;
  raster_image_detach();
  /*
	std::cout<<"GmicUntiledOperationPar::~GmicUntiledOperationPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"GmicUntiledOperationPar::~GmicUntiledOperationPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"GmicUntiledOperationPar::~GmicUntiledOperationPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
  */
}


gmic* PF::GmicUntiledOperationPar::new_gmic()
{
  //if( custom_gmic_commands ) delete [] custom_gmic_commands;
  if( gmic_instance ) delete gmic_instance;

  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
  snprintf( fname, 499, "%s\\gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
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
  return gmic_instance;
}


bool PF::GmicUntiledOperationPar::import_settings( OpParBase* pin )
{
  GmicUntiledOperationPar* par = dynamic_cast<GmicUntiledOperationPar*>( pin );
  if( !par ) return false;
  preview_cache_file_name = par->get_preview_cache_file_name();
  render_cache_file_name = par->get_render_cache_file_name();

  return( PF::OpParBase::import_settings(pin) );
}


void PF::GmicUntiledOperationPar::pre_build( rendermode_t mode )
{
  if( mode==PF_RENDER_PREVIEW ) {
    if( !do_update ) return;
    
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicUntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
    preview_cache_file_name = fname;
  } else {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"GmicUntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
    render_cache_file_name = fname;
  }

  do_update = false;
}


std::string PF::GmicUntiledOperationPar::get_cache_file_name()
{
  std::string cache_file_name;
  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    std::cout<<"GmicUntiledOperationPar: setting cache file name to preview_cache_file_name ("
             <<preview_cache_file_name<<")"<<std::endl;
    cache_file_name = preview_cache_file_name;
  } else { 
    std::cout<<"GmicUntiledOperationPar: setting cache file name to render_cache_file_name ("
             <<render_cache_file_name<<")"<<std::endl;
    cache_file_name = render_cache_file_name;
  }
  std::cout<<"GmicUntiledOperationPar: render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;

  return cache_file_name;
}


std::string PF::GmicUntiledOperationPar::save_image( VipsImage* image, VipsBandFmt format )
{
  char fname[500];
  sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int temp_fd = pf_mkstemp( fname, 4 );
  std::cout<<"GmicExtractForegroundPar: temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
  if( temp_fd < 0 ) return NULL;

  unsigned int level = 0;
  std::vector<VipsImage*> in;
  in.push_back( image );
  convert_format_in->get_par()->set_image_hints( image );
  convert_format_in->get_par()->set_format( format );
  VipsImage* out = convert_format_in->get_par()->build( in, 0, NULL, NULL, level );
  if( !out ) {
    close( temp_fd );
    unlink( fname );
    return( std::string("") );
  }

#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
  vips_image_write_to_file( out, fname );
#else
  vips_image_write_to_file( out, fname, NULL );
#endif
  PF_UNREF( out, "GmicUntiledOperationPar::save_image(): after write_to_file" );
  close( temp_fd );
  return( std::string(fname) );
}



void PF::GmicUntiledOperationPar::update_raster_image()
{
  RasterImage* new_raster_image = NULL;
  
  std::string cache_file_name = get_cache_file_name();
  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i != raster_images.end() && i->second ) {
    //i->second->ref();
    new_raster_image = i->second;
  }

  if( new_raster_image != raster_image ) {
    raster_image_detach();
    if( new_raster_image ) 
      new_raster_image->ref();
  }
  raster_image = new_raster_image;
}


PF::RasterImage* PF::GmicUntiledOperationPar::get_raster_image()
{
  return raster_image;
  
  std::string cache_file_name = get_cache_file_name();
  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i != raster_images.end() && i->second ) {
    //i->second->ref();
    return( i->second );
  }
  return NULL;
}


void PF::GmicUntiledOperationPar::raster_image_detach()
{
  std::cout<<"GmicUntiledOperationPar::raster_image_unref(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    std::cout<<"GmicUntiledOperationPar::raster_image_unref(): raster_image->get_nref()="
             <<raster_image->get_nref()<<" -> "<<raster_image->get_nref()-1<<std::endl;
    raster_image->unref();
    if( raster_image->get_nref() == 0 ) {
      std::string cache_file_name = raster_image->get_file_name();
      std::map<Glib::ustring, RasterImage*>::iterator i = 
        raster_images.find( cache_file_name );
      if( i != raster_images.end() ) 
          raster_images.erase( i );
      delete raster_image;
      unlink( cache_file_name.c_str() );
			std::cout<<"GmicUntiledOperationPar::raster_image_unref(): raster_image deleted"<<std::endl;
    }
  }
  raster_image = 0;
}


void PF::GmicUntiledOperationPar::raster_image_attach()
{
  std::cout<<"GmicUntiledOperationPar::update_raster_image()"<<std::endl;
  raster_image_detach();
  std::string cache_file_name = get_cache_file_name();
  raster_image = new RasterImage( cache_file_name );
  raster_images.insert( make_pair(cache_file_name, raster_image) );  
  std::cout<<"GmicUntiledOperationPar::update_raster_image(): cache_file_name="<<cache_file_name
           <<"  raster_image="<<raster_image<<std::endl;
}



bool PF::GmicUntiledOperationPar::run_gmic( std::string command )
{
  if( command.empty() ) 
    return false;
  std::cout<<"g'mic command: "<<command<<std::endl;
  if( !new_gmic() ) 
    return false;
  gmic_instance->run( command.c_str() );
  
  if( gmic_instance ) {
    delete gmic_instance;
    gmic_instance = NULL;
  }
  
  raster_image_attach();
  
  return true;
}


VipsImage* PF::GmicUntiledOperationPar::get_output( unsigned int& level )
{
  std::cout<<"GmicUntiledOperationPar::get_output(): raster_image="<<raster_image;
  if( raster_image) std::cout<<"  get_nref()="<<raster_image->get_nref();
  std::cout<<std::endl;
  if( !raster_image ) return NULL;

  VipsImage* image = raster_image->get_image( level );
  
  std::cout<<"GmicUntiledOperationPar::get_output(): image="<<image<<std::endl;
  if( !image ) return NULL;

  VipsImage* out = image;
  if( (get_format() != image->BandFmt) ) {
    std::vector<VipsImage*> in;
    in.push_back( image );
    convert_format_out->get_par()->set_image_hints( image );
    convert_format_out->get_par()->set_format( get_format() );
    out = convert_format_out->get_par()->build( in, 0, NULL, NULL, level );
    PF_UNREF( image, "GmicUntiledOperationPar::get_output(): image unref after convert_format" );
  }

  std::cout<<"GmicUntiledOperationPar::get_output(): out="<<out<<std::endl;
  if( out ) {
    set_image_hints( out );
  }
  return out;  
}
