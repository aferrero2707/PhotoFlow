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
  image(NULL),
  demo_image(NULL),
  current_format(VIPS_FORMAT_NOTSET)
{
  set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
  //convert_format = new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>();
  //blender = new PF::Processor<PF::BlenderPar,PF::BlenderProc>();
  fast_demosaic = new_fast_demosaic();
  set_type("raw_loader" );
}


PF::RawLoaderPar::~RawLoaderPar()
{
  if( raw_image ) {
    raw_image->unref();
    if( raw_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RawImage*>::iterator i = 
	raw_images.find( file_name.get() );
      if( i != raw_images.end() ) 
	raw_images.erase( i );
      delete raw_image;
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

  std::map<Glib::ustring, RawImage*>::iterator i = 
    raw_images.find( file_name.get() );
  if( i == raw_images.end() ) {
    raw_image = new RawImage( file_name.get() );
    raw_images.insert( make_pair(file_name.get(), raw_image) );
  } else {
    raw_image = i->second;
    raw_image->ref();
  }
  if( !raw_image )
    return NULL;

  VipsImage* image = raw_image->get_image( level );
  
#ifndef NDEBUG
  std::cout<<"RawLoaderPar::build(): "<<std::endl;
  std::cout<<"image->Interpretation: "<<image->Type<<std::endl;
#endif


  if( image ) {
    g_object_ref( image );
    set_image_hints( image );
  }
  return image;
}


PF::ProcessorBase* PF::new_raw_loader()
{
  return new PF::Processor<PF::RawLoaderPar,PF::RawLoader>();
}
