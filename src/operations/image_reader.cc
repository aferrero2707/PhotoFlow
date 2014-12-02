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

#include "image_reader.hh"
//#include "../vips/vips_layer.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

int
vips_cast( VipsImage *in, VipsImage **out, VipsBandFormat format, ... );

#ifdef __cplusplus
}
#endif /*__cplusplus*/


PF::ImageReaderPar::~ImageReaderPar()
{
	std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image="<<(void*)raster_image<<std::endl;
  if( raster_image ) {
    raster_image->unref();
		std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
    if( raster_image->get_nref() == 0 ) {
      std::map<Glib::ustring, RasterImage*>::iterator i = 
				raster_images.find( raster_image->get_file_name() );
      if( i != raster_images.end() ) 
				raster_images.erase( i );
      delete raster_image;
			std::cout<<"ImageReaderPar::~ImageReaderPar(): raster_image deleted"<<std::endl;
			raster_image = 0;
    }
  }
}


VipsImage* PF::ImageReaderPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  bool modified = false;

  if( file_name.get().empty() )
    return NULL;

  
  std::map<Glib::ustring, RasterImage*>::iterator i = 
    raster_images.find( file_name.get() );
  
  if( i == raster_images.end() ) {

    std::cout<<"ImageReaderPar::build(): raster_image="<<(void*)raster_image<<std::endl;
    if( raster_image ) {
      raster_image->unref();
      std::cout<<"ImageReaderPar::build(): raster_image->get_nref()="<<raster_image->get_nref()<<std::endl;
      if( raster_image->get_nref() == 0 ) {
        std::map<Glib::ustring, RasterImage*>::iterator i = 
          raster_images.find( file_name.get() );
        if( i != raster_images.end() ) 
          raster_images.erase( i );
        delete raster_image;
			std::cout<<"ImageReaderPar::build(): raster_image deleted"<<std::endl;
			raster_image = 0;
      }
    }

    raster_image = new RasterImage( file_name.get() );
    raster_images.insert( make_pair(file_name.get(), raster_image) );
  } else {
    std::cout<<"ImageReaderPar::build(): raster_image found ("<<file_name.get()<<")"<<std::endl;
    raster_image = i->second;
    raster_image->ref();
  }
  if( !raster_image )
    return NULL;
  
  VipsImage* image = raster_image->get_image( level );
  
  if( !image ) return NULL;

#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): "<<std::endl
	   <<"input images:"<<std::endl;
  for(int i = 0; i < in.size(); i++) {
    std::cout<<"  "<<(void*)in[i]<<std::endl;
  }
  std::cout<<"image->Interpretation: "<<image->Type<<std::endl;
  std::cout<<"imap: "<<(void*)imap<<std::endl<<"omap: "<<(void*)omap<<std::endl;
#endif



  void *data;
  size_t data_length;
  if( !vips_image_get_blob( image, VIPS_META_ICC_NAME, 
			    &data, &data_length ) ) {
#ifndef NDEBUG
    cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
    if( profile_in ) {  
      char tstr[1024];
      cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
      std::cout<<"ImageReader: Embedded profile found: "<<tstr<<std::endl;
      cmsCloseProfile( profile_in );
    }
#endif
  }


#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): get_format()="<<get_format()<<"  image->BandFmt="<<image->BandFmt<<std::endl;
#endif
  VipsImage* out = image;
  std::vector<VipsImage*> in2;
  in2.push_back( image );
  convert_format->get_par()->set_image_hints( image );
  convert_format->get_par()->set_format( get_format() );
  out = convert_format->get_par()->build( in2, 0, NULL, NULL, level );

  if( !out ) return NULL;
  PF_UNREF( image, "ImageReaderPar::build(): image unref after convert_format" );

  set_image_hints( out );
#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): image refcount ("<<(void*)image<<") = "<<G_OBJECT(image)->ref_count<<std::endl;
  std::cout<<"                         out refcount ("<<(void*)out<<") = "<<G_OBJECT(out)->ref_count<<std::endl;
#endif

  return out;
}


PF::ProcessorBase* PF::new_image_reader()
{
  return new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
}
