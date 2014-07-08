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


VipsImage* PF::ImageReaderPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  bool modified = false;

  if( file_name.get().empty() )
    return NULL;

  if( file_name.get() != current_file ) {
    /*
    char* fullpath = realpath( file_name.get().c_str(), NULL );
    if( fullpath ) {
      file_name.set( fullpath );
      free( fullpath );
    }
    */
    // Create VipsImage from given file
#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
    image = vips_image_new_from_file( file_name.get().c_str() );
#else
    image = vips_image_new_from_file( file_name.get().c_str(), NULL );
#endif
    if( !image ) return NULL;
#ifndef NDEBUG
    std::string msg = std::string("ImageReaderPar::build(): image refcount after new_from_file");
    PF_PRINT_REF( image, msg );
#endif
    modified = true;
    //} else {
    //g_object_ref( image );
    //std::string msg = std::string("ImageReaderPar::build(): image refcount after ref");
    //PF_PRINT_REF( image, msg );
  }
  
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


  // If the requested format has changed the pyramid has to be re-built
  if( current_format != get_format() )
    modified = true;

#ifndef NDEBUG
  std::cout<<"ImageReaderPar::build(): get_format()="<<get_format()<<"  image->BandFmt="<<image->BandFmt<<std::endl;
#endif
  VipsImage* out = image;
  if( modified && (get_format() != image->BandFmt) ) {
    std::vector<VipsImage*> in2;
    in2.push_back( image );
    convert_format->get_par()->set_image_hints( image );
    convert_format->get_par()->set_format( get_format() );
    out = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
    std::cout<<"ImageReaderPar::build(): out ("<<(void*)out<<") refcount after convert_format: "<<G_OBJECT(out)->ref_count<<std::endl;
    //g_object_unref( image );
    PF_UNREF( image, "ImageReaderPar::build(): image unref after convert_format" );
  }

  current_file = file_name.get();
  current_format = get_format();

  //return out;

  // The pyramid is re-built if the input file or the format have changed
  if( modified ) {
    pyramid.init( out );
    PF_UNREF( out, "ImageReaderPar::build(): out unref after pyramid.init()" );
    std::cout<<"ImageReaderPar::build(): out ("<<(void*)out<<") refcount after pyramid.init(): "<<G_OBJECT(out)->ref_count<<std::endl;
  }

  PF::PyramidLevel* plevel = pyramid.get_level( level );
  if( plevel ) {
    set_image_hints( plevel->image );
#ifndef NDEBUG
    std::cout<<"ImageReaderPar::build(): image refcount ("<<(void*)image<<") = "<<G_OBJECT(image)->ref_count<<std::endl;
    std::cout<<"                         out refcount ("<<(void*)out<<") = "<<G_OBJECT(out)->ref_count<<std::endl;
    std::cout<<"                         plevel->image refcount ("<<(void*)plevel->image<<") = "<<G_OBJECT(plevel->image)->ref_count<<std::endl;
#endif
    return plevel->image;
  }

  return NULL;

  /*
  // Prepare the blending step between the new image (in in2[1]) and the underlying image
  // if existing (in in2[0]).
  // The blending code will simply force the mode to "passthrough" and copy in2[1] to outnew
  // if in2[0] is NULL
  std::vector<VipsImage*> in2;
  if( !in.empty() ) in2.push_back( in[0] );
  else in2.push_back( NULL );
  in2.push_back( image );

  blender->get_par()->build( in2, 0, imap, omap);

  set_image( blender->get_par()->get_image() );
  */
}


PF::ProcessorBase* PF::new_image_reader()
{
  return new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
}
