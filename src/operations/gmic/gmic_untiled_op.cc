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
  do_update( true ),
  raster_image( NULL )
{	
  convert_format_in = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  convert_format_out = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
}


PF::GmicUntiledOperationPar::~GmicUntiledOperationPar()
{
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


bool PF::GmicUntiledOperationPar::run_gmic( std::vector< std::pair<VipsImage*,bool> >& in )
{
  std::string cache_file_name;
  if( get_render_mode() == PF_RENDER_PREVIEW ) {
    std::cout<<"GmicUntiledOperationPar::build() setting cache file name to preview_cache_file_name ("
             <<preview_cache_file_name<<")"<<std::endl;
    cache_file_name = preview_cache_file_name;
  } else { 
    std::cout<<"GmicUntiledOperationPar::build() setting cache file name to render_cache_file_name ("
             <<render_cache_file_name<<")"<<std::endl;
    cache_file_name = render_cache_file_name;
  }
  std::cout<<"GmicUntiledOperationPar::build() render_mode="<<get_render_mode()<<"  cache_file_name="<<cache_file_name<<std::endl;

  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( cache_file_name );
  if( i == raster_images.end() ) {

    std::cout<<"GmicUntiledOperationPar::build(): raster_image="<<(void*)raster_image<<std::endl;
    if( raster_image ) {
      raster_image->unref();
      std::cout<<"GmicUntiledOperationPar::build(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
      if( raster_image->get_nref() == 0 ) {
        std::map<Glib::ustring, RasterImage*>::iterator i = 
          raster_images.find( cache_file_name );
        if( i != raster_images.end() ) 
          raster_images.erase( i );
        delete raster_image;
			std::cout<<"GmicUntiledOperationPar::build(): raster_image deleted"<<std::endl;
			raster_image = 0;
      }
    }

    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    int temp_fd = pf_mkstemp( fname, 4 );
    std::cout<<"GmicUntiledOperationPar: temp file: "<<fname<<"  fd="<<temp_fd<<std::endl;
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
    PF_UNREF( floatimg, "GmicUntiledOperationPar::build(): after write_to_file" );

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
    std::cout<<"GmicUntiledOperationPar::build(): raster_image found ("<<cache_file_name<<")"<<std::endl;
    raster_image = i->second;
    raster_image->ref();
  }
}
