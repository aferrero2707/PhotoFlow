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

#include "untiled_op.hh"



PF::UntiledOperationPar::UntiledOperationPar():
  OpParBase(),
  do_update( true ),
  raster_image( NULL )
{	
  convert_format_in = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  convert_format_out = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
}


PF::UntiledOperationPar::~UntiledOperationPar()
{
	std::cout<<"UntiledOperationPar::~UntiledOperationPar(): raster_image="<<(void*)raster_image<<std::endl;
  raster_image_detach();
  /*
	std::cout<<"UntiledOperationPar::~UntiledOperationPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"UntiledOperationPar::~UntiledOperationPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"UntiledOperationPar::~UntiledOperationPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
  */
}


bool PF::UntiledOperationPar::import_settings( OpParBase* pin )
{
  UntiledOperationPar* par = dynamic_cast<UntiledOperationPar*>( pin );
  if( !par ) return false;
  preview_cache_file_name = par->get_preview_cache_file_name();
  render_cache_file_name = par->get_render_cache_file_name();

  return( PF::OpParBase::import_settings(pin) );
}


void PF::UntiledOperationPar::pre_build( rendermode_t mode )
{
  if( mode==PF_RENDER_PREVIEW ) {
    if( !do_update ) return;
    
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"UntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
    preview_cache_file_name = fname;
  } else {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    if( temp_fd < 0 ) return;
    std::cout<<"UntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
    render_cache_file_name = fname;
  }

  do_update = false;
}


std::string PF::UntiledOperationPar::get_cache_file_name()
{
  std::string cache_file_name;
  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    std::cout<<"UntiledOperationPar: setting cache file name to preview_cache_file_name ("
             <<preview_cache_file_name<<")"<<std::endl;
    cache_file_name = preview_cache_file_name;
  } else { 
    std::cout<<"UntiledOperationPar: setting cache file name to render_cache_file_name ("
             <<render_cache_file_name<<")"<<std::endl;
    cache_file_name = render_cache_file_name;
  }
  std::cout<<"UntiledOperationPar: render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;

  return cache_file_name;
}


std::string PF::UntiledOperationPar::save_image( VipsImage* image, VipsBandFmt format )
{
  char fname[500];
  sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int temp_fd = pf_mkstemp( fname, 4 );
  std::cout<<"UntiledOperationPar::save_image(): temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
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
  PF_UNREF( out, "UntiledOperationPar::save_image(): after write_to_file" );
  close( temp_fd );
  return( std::string(fname) );
}



void PF::UntiledOperationPar::update_raster_image()
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


PF::RasterImage* PF::UntiledOperationPar::get_raster_image()
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


void PF::UntiledOperationPar::raster_image_detach()
{
  std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image->get_nref()="
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
			std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image deleted"<<std::endl;
    }
  }
  raster_image = 0;
}


void PF::UntiledOperationPar::raster_image_attach()
{
  std::cout<<"UntiledOperationPar::update_raster_image()"<<std::endl;
  raster_image_detach();
  std::string cache_file_name = get_cache_file_name();
  raster_image = new RasterImage( cache_file_name );
  raster_images.insert( make_pair(cache_file_name, raster_image) );  
  std::cout<<"UntiledOperationPar::update_raster_image(): cache_file_name="<<cache_file_name
           <<"  raster_image="<<raster_image<<std::endl;
}



VipsImage* PF::UntiledOperationPar::get_output( unsigned int& level )
{
  std::cout<<"UntiledOperationPar::get_output(): raster_image="<<raster_image;
  if( raster_image) std::cout<<"  get_nref()="<<raster_image->get_nref();
  std::cout<<std::endl;
  if( !raster_image ) return NULL;

  VipsImage* image = raster_image->get_image( level );
  
  std::cout<<"UntiledOperationPar::get_output(): image="<<image<<std::endl;
  if( !image ) return NULL;

  VipsImage* out = image;
  if( (get_format() != image->BandFmt) ) {
    std::vector<VipsImage*> in;
    in.push_back( image );
    convert_format_out->get_par()->set_image_hints( image );
    convert_format_out->get_par()->set_format( get_format() );
    out = convert_format_out->get_par()->build( in, 0, NULL, NULL, level );
    PF_UNREF( image, "UntiledOperationPar::get_output(): image unref after convert_format" );
  }

  std::cout<<"UntiledOperationPar::get_output(): out="<<out<<std::endl;
  if( out ) {
    set_image_hints( out );
  }
  return out;  
}
