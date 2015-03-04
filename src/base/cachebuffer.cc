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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <stdlib.h>

#include <iostream>

#include "pf_mkstemp.hh"
#include "photoflow.hh"
#include "cachebuffer.hh"
#include "exif_data.hh"


PF::CacheBuffer::CacheBuffer():
  image( NULL ), cached( NULL ), fd(-1),
  initialized( false ), completed( false ), step_x(0), step_y(0)
{
}


void PF::CacheBuffer::reset( bool reinitialize )
{
  if( cached ) 
    PF_UNREF( cached, "CacheBuffer::reset(): cached image unref" );
  cached = NULL;
  image = NULL;
  completed = false;
  step_x = step_y = 0;
  pyramid.reset();
  if( fd > 0 ) {
    close( fd );
    unlink( filename.c_str() );
    fd = -1;
    filename.clear();
  }

  if( reinitialize ) set_initialized( false );
}


void PF::CacheBuffer::step()
{
  if( completed ) return;
  if( !image ) return;
  
  bool result = false;

  VipsRect tile_area = { step_x, step_y, PF_CACHE_BUFFER_TILE_SIZE, PF_CACHE_BUFFER_TILE_SIZE };
  VipsRect image_area = { 0, 0, image->Xsize, image->Ysize };

  vips_rect_intersectrect( &image_area, &tile_area, &tile_area );
  if( tile_area.width<=0 || tile_area.height<=0 ) return;

  if( tile_area.left == 0 ) std::cout<<"CacheBuffer::step(): row="<<tile_area.top<<std::endl;

  // Update the image region corresponding to the current tile
  VipsRegion* reg = vips_region_new( image );
  if( vips_region_prepare( reg, &tile_area ) )
    return;

  // Copy the tile into the disk buffer. Create the disk buffer if not yet done.
  if( fd < 0 ) {
		char fname[500];
		sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
		//fd = mkostemp( fname, O_CREAT|O_RDWR|O_TRUNC );
		fd = pf_mkstemp( fname );
		if( fd >= 0 )
			filename = fname;
	}
  if( fd < 0 ) return;

  guchar* p;
  off_t offset = VIPS_IMAGE_SIZEOF_LINE(image)*tile_area.top+VIPS_IMAGE_SIZEOF_PEL(image)*tile_area.left;
  for( int y = 0; y < tile_area.height; y++ ) {
    lseek( fd, offset, SEEK_SET );
    p = VIPS_REGION_ADDR( reg, tile_area.left, tile_area.top+y );
    ssize_t n = ::write( fd, p, VIPS_REGION_SIZEOF_LINE(reg) );
    if( n != VIPS_REGION_SIZEOF_LINE(reg) )
      break;
    offset += VIPS_IMAGE_SIZEOF_LINE(image);
  }

  VIPS_UNREF( reg );

  // Increase the tile coordinates
  step_x += PF_CACHE_BUFFER_TILE_SIZE;
  if( step_x >= image->Xsize ) {
    step_x = 0;
    step_y += PF_CACHE_BUFFER_TILE_SIZE;
  }
  if( step_y >= image->Ysize ) {
    completed = true;
    void *profile_data;
    size_t profile_length;
    if( vips_image_get_blob( image, VIPS_META_ICC_NAME, 
                             &profile_data, &profile_length ) )
      profile_data = NULL;
  
    size_t blobsz;
    void* image_data;
    if( vips_image_get_blob( image, "raw_image_data",
                             &image_data, 
                             &blobsz ) )
      image_data = NULL;

    size_t exifsz;
    void* exif_data;
    if( vips_image_get_blob( image, PF_META_EXIF_NAME,
        &exif_data,&exifsz ) ) {
      exif_data = NULL;
    }

    int width = image->Xsize;
    int height = image->Ysize;
    int size = (width>height) ? width : height;
    int nbands = image->Bands;

    VipsImage* rawimg;

    vips_rawload( filename.c_str(), &rawimg, width, height, 
                  VIPS_IMAGE_SIZEOF_PEL( image ), NULL );
    vips_copy( rawimg, &cached, 
	       "format", image->BandFmt,
	       "bands", image->Bands,
	       "coding", image->Coding,
	       "interpretation", image->Type,
	       NULL );
    PF_UNREF( rawimg, "CacheBuffer::step() completed" );

    if( profile_data ) {
      void* profile_data2 = malloc( profile_length );
      if( profile_data2 ) {
        memcpy( profile_data2, profile_data, profile_length );
        vips_image_set_blob( cached, VIPS_META_ICC_NAME, 
                             (VipsCallbackFn) g_free, 
                             profile_data2, profile_length );
      }
    }

    if( image_data ) {
      void* image_data2 = malloc( blobsz );
      if( image_data2 ) {
        memcpy( image_data2, image_data, blobsz );
        vips_image_set_blob( cached, "raw_image_data", 
            (VipsCallbackFn) g_free,
            image_data2, blobsz );
      }
    }

    if( exif_data ) {
      void* exif_data2 = malloc( exifsz );
      if( exif_data2 ) {
        memcpy( exif_data2, exif_data, exifsz );
        vips_image_set_blob( cached, PF_META_EXIF_NAME,
            (VipsCallbackFn) PF::exif_free,
            exif_data2, exifsz );
      }
    }

    pyramid.init( cached );
    std::cout<<"CacheBuffer: caching completed"<<std::endl;
    result = true;
  }

  return;
}


void PF::CacheBuffer::write()
{
  std::cout<<"CacheBuffer::write(): complete="<<completed<<"  image="<<image<<std::endl;
  if( completed ) return;
  if( !image ) return;

  bool result = false;

  // Copy the image into the disk buffer. Create the disk buffer if not yet done.
  if( fd < 0 ) {
    char fname[500];
    sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
    //fd = mkostemp( fname, O_CREAT|O_RDWR|O_TRUNC );
    fd = pf_mkstemp( fname );
    if( fd >= 0 )
      filename = fname;
  }
  if( fd < 0 ) return;

  std::cout<<"CacheBuffer::write(): saving image data into "<<filename<<std::endl;

  int fail = vips_rawsave_fd( image, fd, NULL );
  if( fail ) {
    std::cout<<"CacheBuffer: vips_rawsave_fd() failed"<<std::endl;
    return;
  }
  close( fd );


    completed = true;
    void *profile_data;
    size_t profile_length;
    if( vips_image_get_blob( image, VIPS_META_ICC_NAME,
                             &profile_data, &profile_length ) )
      profile_data = NULL;

    size_t blobsz;
    void* image_data;
    if( vips_image_get_blob( image, "raw_image_data",
                             &image_data,
                             &blobsz ) )
      image_data = NULL;

    size_t exifsz;
    void* exif_data;
    if( vips_image_get_blob( image, PF_META_EXIF_NAME,
        &exif_data,&exifsz ) ) {
      exif_data = NULL;
    }

    int width = image->Xsize;
    int height = image->Ysize;
    int size = (width>height) ? width : height;
    int nbands = image->Bands;

    VipsImage* rawimg;

    vips_rawload( filename.c_str(), &rawimg, width, height,
                  VIPS_IMAGE_SIZEOF_PEL( image ), NULL );
    vips_copy( rawimg, &cached,
         "format", image->BandFmt,
         "bands", image->Bands,
         "coding", image->Coding,
         "interpretation", image->Type,
         NULL );
    PF_UNREF( rawimg, "CacheBuffer::step() completed" );

    if( profile_data ) {
      void* profile_data2 = malloc( profile_length );
      if( profile_data2 ) {
        memcpy( profile_data2, profile_data, profile_length );
        vips_image_set_blob( cached, VIPS_META_ICC_NAME,
                             (VipsCallbackFn) g_free,
                             profile_data2, profile_length );
      }
    }

    if( image_data ) {
      void* image_data2 = malloc( blobsz );
      if( image_data2 ) {
        memcpy( image_data2, image_data, blobsz );
        vips_image_set_blob( cached, "raw_image_data",
            (VipsCallbackFn) g_free,
            image_data2, blobsz );
      }
    }

    if( exif_data ) {
      void* exif_data2 = malloc( exifsz );
      if( exif_data2 ) {
        memcpy( exif_data2, exif_data, exifsz );
        vips_image_set_blob( cached, PF_META_EXIF_NAME,
            (VipsCallbackFn) PF::exif_free,
            exif_data2, exifsz );
      }
    }

    pyramid.init( cached );
    std::cout<<"CacheBuffer: caching completed"<<std::endl;
    result = true;

  return;
}
