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
  cache_files_num( 0 )
{	
  set_cache_files_num(1);
  convert_format_in = new_convert_format();
  convert_format_out = new_convert_format();
}


PF::UntiledOperationPar::~UntiledOperationPar()
{
	std::cout<<"UntiledOperationPar::~UntiledOperationPar()"<<std::endl;
  for( unsigned int i = 0; i < get_cache_files_num(); i++ )
  raster_image_detach(i);
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
  set_cache_files_num( par->get_cache_files_num() );
  preview_cache_file_names.reserve( get_cache_files_num() );
  render_cache_file_names.reserve( get_cache_files_num() );
  for( unsigned int i = 0; i < get_cache_files_num(); i++ ) {
    std::string tstr;
    tstr = par->get_preview_cache_file_name(i);
    preview_cache_file_names[i] = tstr;
    tstr = par->get_render_cache_file_name(i);
    render_cache_file_names[i] = tstr;
  }

  return( PF::OpParBase::import_settings(pin) );
}


void PF::UntiledOperationPar::pre_build( rendermode_t mode )
{
  if( true /*mode==PF_RENDER_PREVIEW*/ ) {
    if( !do_update ) return;

    preview_cache_file_names.reserve( get_cache_files_num() );
    char fname[500];
    for( unsigned int i = 0; i < get_cache_files_num(); i++ ) {
      sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
      int temp_fd = pf_mkstemp( fname, 4 );
      if( temp_fd < 0 ) return;
#ifndef NDEBUG
      std::cout<<"UntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
#endif
      preview_cache_file_names[i] = fname;
    }
  } else {
    char fname[500];
    render_cache_file_names.reserve( get_cache_files_num() );
    for( unsigned int i = 0; i < get_cache_files_num(); i++ ) {
      sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
      int temp_fd = pf_mkstemp( fname, 4 );
      if( temp_fd < 0 ) return;
#ifndef NDEBUG
      std::cout<<"UntiledOperationPar::pre_build(): cache file="<<fname<<std::endl;
#endif
      render_cache_file_names[i] = fname;
    }
  }

  do_update = false;
}


std::string PF::UntiledOperationPar::get_cache_file_name( unsigned int n )
{
  std::string cache_file_name;
  if( true /*get_render_mode() == PF_RENDER_PREVIEW*/ ) {
    cache_file_name = get_preview_cache_file_name( n );
#ifndef NDEBUG
    std::cout<<"UntiledOperationPar: setting cache file name to preview_cache_file_name ("
             <<cache_file_name<<")"<<std::endl;
#endif
  } else { 
    cache_file_name = get_render_cache_file_name( n );
#ifndef NDEBUG
    std::cout<<"UntiledOperationPar: setting cache file name to render_cache_file_name ("
             <<cache_file_name<<")"<<std::endl;
#endif
  }
#ifndef NDEBUG
  std::cout<<"UntiledOperationPar: render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;
#endif

  return cache_file_name;
}


std::string PF::UntiledOperationPar::save_image( VipsImage* image, VipsBandFormat format )
{
  char fname[500];
  sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
  int temp_fd = pf_mkstemp( fname, 4 );
#ifndef NDEBUG
  std::cout<<"UntiledOperationPar::save_image(): temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
#endif
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



void PF::UntiledOperationPar::update_raster_images()
{
  raster_image_vec.reserve( get_cache_files_num() );
  for( unsigned int id = 0; id < get_cache_files_num(); id++ ) {
    RasterImage* new_raster_image = NULL;

    std::string cache_file_name = get_cache_file_name( id );
    if( !(cache_file_name.empty()) ) {
      std::map<Glib::ustring, RasterImage*>::iterator i =
          raster_images.find( cache_file_name );
      if( i != raster_images.end() && i->second ) {
        //i->second->ref();
        new_raster_image = i->second;
      }

      if( new_raster_image != raster_image_vec[id] ) {
        raster_image_detach(id);
        if( new_raster_image )
          new_raster_image->ref();
      }
    }
    raster_image_vec[id] = new_raster_image;
  }
}


PF::RasterImage* PF::UntiledOperationPar::get_raster_image( unsigned int n)
{
  if( n < raster_image_vec.size() )
    return raster_image_vec[n];

  return NULL;

  /*
  std::string cache_file_name = get_cache_file_name();
  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i != raster_images.end() && i->second ) {
    //i->second->ref();
    return( i->second );
  }
  return NULL;
  */
}


void PF::UntiledOperationPar::raster_image_detach( unsigned int n )
{
  if( n>=raster_image_vec.size() )
    return;
  PF::RasterImage* raster_image = raster_image_vec[n];
#ifndef NDEBUG
  std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image_vec["<<n<<"]="<<(void*)raster_image<<std::endl;
#endif
  if( raster_image ) {
#ifndef NDEBUG
    std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image->get_nref()="
             <<raster_image->get_nref()<<" -> "<<raster_image->get_nref()-1<<std::endl;
#endif
    raster_image->unref();
    if( raster_image->get_nref() == 0 ) {
      std::string cache_file_name = raster_image->get_file_name();
      std::map<Glib::ustring, RasterImage*>::iterator i = 
        raster_images.find( cache_file_name );
      if( i != raster_images.end() ) 
          raster_images.erase( i );
      delete raster_image;
      unlink( cache_file_name.c_str() );
#ifndef NDEBUG
			std::cout<<"UntiledOperationPar::raster_image_unref(): raster_image deleted"<<std::endl;
#endif
    }
  }
  raster_image_vec[n] = 0;
}


void PF::UntiledOperationPar::raster_image_attach( VipsImage* in, unsigned int n )
{
  raster_image_vec.reserve( n+1 );
#ifndef NDEBUG
  std::cout<<"UntiledOperationPar::update_raster_image()"<<std::endl;
#endif
  raster_image_detach( n );
  std::string cache_file_name = get_cache_file_name( n );
  if( cache_file_name.empty() )
    return;
  PF::RasterImage* raster_image = new RasterImage( cache_file_name );
  unsigned int level = 0;
  VipsImage* image = raster_image->get_image( level );

  set_metadata( in, image );

  raster_images.insert( make_pair(cache_file_name, raster_image) );  
  raster_image_vec[n] = raster_image;
#ifndef NDEBUG
  std::cout<<"UntiledOperationPar::update_raster_image(): cache_file_name="<<cache_file_name
           <<"  raster_image="<<raster_image<<std::endl;
#endif
}



std::vector<VipsImage*> PF::UntiledOperationPar::get_output( unsigned int& level )
{
  std::vector<VipsImage*> outvec;
  for( unsigned int i = 0; i < get_cache_files_num(); i++ ) {
    PF::RasterImage* raster_image = raster_image_vec[i];
#ifndef NDEBUG
    std::cout<<"UntiledOperationPar::get_output(): raster_image="<<raster_image;
    if( raster_image) std::cout<<"  get_nref()="<<raster_image->get_nref();
    std::cout<<std::endl;
#endif
    if( !raster_image ) continue;

    //raster_image->print_icc();

    VipsImage* image = raster_image->get_image( level );

    //#ifndef NDEBUG
    std::cout<<"UntiledOperationPar::get_output(): image="<<image<<std::endl;
    //#endif
    if( !image ) continue;
    raster_image->print_icc( image );

    VipsImage* out = image;
    if( (get_format() != image->BandFmt) ) {
      std::vector<VipsImage*> in;
      in.push_back( image );
      convert_format_out->get_par()->set_image_hints( image );
      convert_format_out->get_par()->set_format( get_format() );
      out = convert_format_out->get_par()->build( in, 0, NULL, NULL, level );
      PF_UNREF( image, "UntiledOperationPar::get_output(): image unref after convert_format" );
    }

#ifndef NDEBUG
    std::cout<<"UntiledOperationPar::get_output(): out="<<out<<std::endl;
#endif
    if( out ) {
      set_image_hints( out );
      outvec.push_back(out);
    }
  }
  return outvec;
}
