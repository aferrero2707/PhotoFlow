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
#include "raster_image.hh"



PF::RasterImage::RasterImage( const std::string f ):
	nref(1), file_name( f ),
  image( NULL )
{
  if( file_name.empty() ) return;
  // Create VipsImage from given file
#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
  image = vips_image_new_from_file( file_name.c_str() );
#else
  image = vips_image_new_from_file( file_name.c_str(), NULL );
#endif
  if( !image ) {
    std::cout<<"Failed to load "<<file_name<<std::endl;
    return;
  }
  
  pyramid.init( image );
}


PF::RasterImage::~RasterImage()
{
  if( image ) PF_UNREF( image, "RasterImage::~RasterImage() image" );
	std::cout<<"RasterImage::~RasterImage() called."<<std::endl;
	if( !(cache_file_name.empty()) )
		unlink( cache_file_name.c_str() );
}


std::map<Glib::ustring, PF::RasterImage*> PF::raster_images;


