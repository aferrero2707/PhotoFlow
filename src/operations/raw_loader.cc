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


#include "raw_loader.hh"


PF::RawLoaderPar::RawLoaderPar(): 
  OpParBase(), 
  file_name("file_name", this),
  raw_image(NULL),
  image(NULL),
  demo_image(NULL),
  current_format(VIPS_FORMAT_NOTSET)
{
  //convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  //blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  fast_demosaic = new_fast_demosaic();
  set_type("raw_loader" );
}


PF::RawLoaderPar::~RawLoaderPar()
{
	std::cout<<"RawLoaderPar::~RawLoaderPar(): raw_image="<<(void*)raw_image<<std::endl;
  if( raw_image ) {
    raw_image->unref();
		std::cout<<"RawLoaderPar::~RawLoaderPar(): raw_image->get_nref()="<<raw_image->get_nref()<<std::endl;
    if( raw_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RawImage*>::iterator i = 
				raw_images.find( file_name.get() );
      if( i != raw_images.end() ) 
				raw_images.erase( i );
      delete raw_image;
			std::cout<<"RawLoaderPar::~RawLoaderPar(): raw_image deleted"<<std::endl;
			raw_image = 0;
    }
  }
}


VipsImage* PF::RawLoaderPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  bool modified = false;

  if( file_name.get().empty() )
    return NULL;

  RawImage* new_raw_image = NULL;

  std::map<Glib::ustring, RawImage*>::iterator i = 
    raw_images.find( file_name.get() );
  if( i == raw_images.end() ) {
    std::cout<<"ImageReaderPar::build(): creating new RawImage for file "<<file_name.get()<<std::endl;
    new_raw_image = new RawImage( file_name.get() );
    if( new_raw_image )
      raw_images.insert( make_pair(file_name.get(), new_raw_image) );
  } else {
    new_raw_image = i->second;
    new_raw_image->ref();
  }

  if( raw_image ) std::cout<<"raw_image->get_nref(): "<<raw_image->get_nref()<<std::endl;
  if( new_raw_image ) std::cout<<"new_raw_image->get_nref(): "<<new_raw_image->get_nref()<<std::endl;

  if( raw_image ) {
    raw_image->unref();
    if( raw_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RawImage*>::iterator i =
        raw_images.find( file_name.get() );
      if( i != raw_images.end() )
        raw_images.erase( i );
      delete raw_image;
      std::cout<<"ImageReaderPar::build(): raw_image deleted"<<std::endl;
    }
  }

  raw_image = new_raw_image;

  if( !raw_image )
    return NULL;

  VipsImage* image = raw_image->get_image( level );

  if( image ) {
#ifndef NDEBUG
    std::cout<<"RawLoaderPar::build(): "<<std::endl;
    std::cout<<"image->Interpretation: "<<image->Type<<std::endl;
#endif

    //PF_REF( image, "RawLoaderPar::build()" );
    set_image_hints( image );
  }
  return image;
}


PF::ProcessorBase* PF::new_raw_loader()
{
  return new PF::Processor<PF::RawLoaderPar,PF::RawLoader>();
}
