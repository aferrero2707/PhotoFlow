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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "pf_mkstemp.hh"
#include "photoflow.hh"
#include "imagepyramid.hh"
#include "exif_data.hh"
#include "operation.hh"

VipsImage* PF::pyramid_test_image = NULL;
GObject* PF::pyramid_test_obj = NULL;

static void free_mem_array (VipsObject *object, gpointer user_data)
{
  if( user_data ) free( user_data );
}



PF::ImagePyramid::~ImagePyramid()
{
  char tstr[500];
  for( unsigned int i = 1; i < levels.size(); i++ ) {
    //g_object_unref( levels[i].image );
    snprintf(tstr, 499, "PF::ImagePyramid::~ImagePyramid() levels[%d].image",i);
    PF_UNREF( levels[i].image, tstr );
    //if( i < 1 ) continue;
    if( levels[i].fd >= 0 ) 
      close( levels[i].fd );
    unlink( levels[i].raw_file_name.c_str() );
    //std::cout<<"PF::ImagePyramid::~ImagePyramid(): "<<levels[i].raw_file_name<<" removed."<<std::endl;
  }
}


void PF::ImagePyramid::init( VipsImage* img, int fd )
{
  // The input image might be the same, therefore we reference it 
  // before unreferencing the old ones
  //PF_REF( img, "ImagePyramid::init() img ref" );

  {
    size_t exifsz;
    void* exif_data;
    if( !vips_image_get_blob( img, PF_META_EXIF_NAME,
        &exif_data,&exifsz ) ) {
      //std::cout<<"ImagePyramid::init(): exif_custom_data found in img("<<img<<")"<<std::endl;
    } else {
      std::cout<<"ImagePyramid::init(): exif_custom_data not found in img("<<img<<")"<<std::endl;
    }
  }

  char tstr[500];
  for( unsigned int i = 1; i < levels.size(); i++ ) {
    //g_object_unref( levels[i].image );
    snprintf(tstr, 499, "PF::ImagePyramid::init() levels[%d].image",i);
    PF_UNREF( levels[i].image, tstr );
    if( levels[i].fd >= 0 ) 
      close( levels[i].fd );
    unlink( levels[i].raw_file_name.c_str() );
  }
  levels.clear();

  PF::PyramidLevel level;
  level.image = img;
  level.fd = fd;
  levels.push_back( level );


#ifndef NDEBUG
  std::cout<<"ImagePyramid::init(): full-scale image="<<img<<std::endl;
#endif
}


void PF::ImagePyramid::reset()
{
  if( levels.size() < 1 ) 
    return;

  PF::PyramidLevel level = levels[0];

  char tstr[500];
  for( unsigned int i = 1; i < levels.size(); i++ ) {
    //g_object_unref( levels[i].image );
    snprintf(tstr, 499, "PF::ImagePyramid::reset() levels[%d].image",i);
    PF_UNREF( levels[i].image, tstr );
    if( levels[i].fd >= 0 ) 
      close( levels[i].fd );
    unlink( levels[i].raw_file_name.c_str() );
  }
  levels.clear();

  levels.push_back( level );
}


void PF::ImagePyramid::update( const VipsRect& area )
{  
#ifndef NDEBUG
	std::cout<<"PF::ImagePyramid::update() called."<<std::endl;
#endif
  if( levels.empty() ) 
    return;

  if( levels[0].fd < 0 ) return;

  VipsImage* in = levels.back().image;
  if( !in )
    return;

  VipsRect area_in;
  area_in.left = area.left;
  area_in.top = area.top;
  area_in.width = area.width;
  area_in.height = area.height;
  VipsRect area_out;

  int nbands = in->Bands;

  int pelsz = VIPS_IMAGE_SIZEOF_PEL( in );

  for( unsigned int li = 1; li < levels.size(); li++ ) {

    if( levels[li].fd < 0 ) break;
    if( !levels[li].image ) break;

#ifndef NDEBUG
		std::cout<<"PF::ImagePyramid::update(): processing level #"<<li<<std::endl;
#endif
    int in_fd = levels[li-1].fd;
    int out_fd = levels[li].fd;

    VipsImage* in = levels[li-1].image;
    VipsImage* out = levels[li].image;

    int in_width = in->Xsize;
    int in_height = in->Ysize;
    int out_width = in_width/2;
    int out_height = in_height/2;

    area_out.left = area_in.left/2;
    area_out.top = area_in.top/2;
    area_out.width = area_in.width/2 + 1;
    area_out.height = area_in.height/2 + 1;

    int out_right = area_out.left + area_out.width - 1;
    if( out_right >= out_width ) {
      area_out.width -= (out_right-out_width-1);
      out_right = area_out.left + area_out.width - 1;
    }

    int out_bottom = area_out.top + area_out.height - 1;
    if( out_bottom >= out_height ) {
      area_out.height -= (out_bottom-out_height-1);
      out_bottom = area_out.top + area_out.height - 1;
    }

#ifndef NDEBUG
    std::cout<<"PF::ImagePyramid::update():"<<std::endl
	     <<"  level="<<li<<std::endl
	     <<"  in="<<in<<"  out="<<out<<std::endl
	     <<"  area in: "<<area_in.width<<"x"<<area_in.height<<"+"<<area_in.left<<"+"<<area_in.top<<std::endl
	     <<"  area out: "<<area_out.width<<"x"<<area_out.height<<"+"<<area_out.left<<"+"<<area_out.top<<std::endl
	     <<"  out right="<<out_right<<"  out bottom="<<out_bottom<<std::endl;
#endif
    area_in.left = area_out.left*2;
    area_in.top = area_out.top*2;
    area_in.width = area_out.width*2;
    area_in.height = area_out.height*2;

    int x, x2, y, in_linesz = area_in.width*pelsz, out_linesz = area_out.width*pelsz;

    unsigned char* buf_in = new unsigned char[in_linesz];
    if( !buf_in ) break;
    unsigned char* buf_out = new unsigned char[out_linesz];
    if( !buf_out ) { delete( buf_in ); break; }

    for( y = area_out.top; y <= out_bottom; y++ ) {

      off_t in_offset = (off_t(in_width)*y*2+area_in.left)*pelsz;
      lseek( in_fd, in_offset, SEEK_SET );

      read( in_fd, buf_in, in_linesz );

      for( x = 0, x2 = 0; x < out_linesz; x += pelsz, x2 += pelsz*2 ) {
	memcpy( &(buf_out[x]), &(buf_in[x2]), pelsz );
      }
      
      off_t out_offset = (off_t(out_width)*y+area_out.left)*pelsz;
      lseek( out_fd, out_offset, SEEK_SET );
      
      write( out_fd, buf_out, out_linesz );
    }

    area_in.left = area_out.left;
    area_in.top = area_out.top;
    area_in.width = area_out.width;
    area_in.height = area_out.height;

    delete( buf_in );
    delete( buf_out );
  }  
}


PF::PyramidLevel* PF::ImagePyramid::get_level( unsigned int& level )
{  
  char tstr[500];
/*
  if( levels.size() > 1 ) {
    pyramid_test_image = levels[1].image;
    pyramid_test_obj = G_OBJECT( pyramid_test_image );
  }
*/
  if( levels.empty() ) 
    return NULL;
  
  VipsImage* img = levels[0].image;

  /*
  size_t exifsz;
  void* exif_data;
  if( !vips_image_get_blob( img, PF_META_EXIF_NAME,
      &exif_data,&exifsz ) ) {
    //std::cout<<"ImagePyramid::get_level(): exif_custom_data found in img("<<img<<")"<<std::endl;
  } else {
    std::cout<<"ImagePyramid::get_level(): exif_custom_data not found in img("<<img<<")"<<std::endl;
    exif_data = NULL;
  }
  */

  if( level < levels.size() ) {
    // We add a reference to the returned pyramid level, since it must be kept alive until
    // the pyramid is re-built, in which case it will be unreff'd by the pyramid itself
#ifndef NDEBUG
    std::cout<<"ImagePyramid::get_level("<<level<<") cached="<<levels[level].image<<std::endl;
#endif
    if( levels[level].raw_file_name.empty() )
      snprintf(tstr,499,"ImagePyramid::get_level(): levels[%d].image ref",(int)level);
    else
      snprintf(tstr,499,"ImagePyramid::get_level(): levels[%d].image ref (%s)",
               (int)level,levels[level].raw_file_name.c_str());
    PF_REF( levels[level].image, tstr );
    return( &(levels[level]) );
  }
  
  PF::PyramidLevel newlevel;

  /*
  void *profile_data;
  size_t profile_length;
  if( vips_image_get_blob( img, VIPS_META_ICC_NAME, 
			   &profile_data, &profile_length ) )
    profile_data = NULL;
  
  size_t blobsz;
  void* image_data;
  if( vips_image_get_blob( img, "raw_image_data",
			   &image_data, 
			   &blobsz ) )
    image_data = NULL;
*/

  Glib::Timer timer;
  timer.start();

  VipsImage* in = levels.back().image;
  if( !in )
    return NULL;
  VipsImage* fullres = in;

  int width = in->Xsize;
  int height = in->Ysize;
  int size = (width>height) ? width : height;
  int nbands = in->Bands;
  double scale = 1;

//  std::cout<<"ImagePyramid::get_level("<<level<<") scale="<<scale
//      <<"  image size: "<<in->Xsize<<" x "<<in->Ysize<<std::endl;
  while( size > 256 ) {
    VipsImage* out;
    /*
    VipsImage* blurred;
    if( vips_gaussblur( in, &blurred, 0.7, NULL ) )
      return NULL;
    if( vips_subsample( blurred, &out, 2, 2, NULL ) )
      return NULL;
    PF_UNREF( blurred, "ImagePyramid::get_level(): blurred unref" );
    */
    scale /= 2;
    if( vips_resize( in/*fullres*/, &out, 0.5/*scale*/, NULL) )
    //if( vips_subsample( in, &out, 2, 2, NULL ) )
      return NULL;
    //PF_UNREF( in, "ImagePyramid::get_level(): in unref" );
#ifndef NDEBUG
    std::cout<<"ImagePyramid::get_level("<<level<<") subsample in="<<in<<"  out="<<out<<std::endl;
    std::cout<<"ImagePyramid::get_level("<<level<<") scale="<<scale
        <<"  image size: "<<out->Xsize<<" x "<<out->Ysize<<std::endl;
#endif
    //g_object_unref( in );
    width = out->Xsize;
    height = out->Ysize;
    size = (width>height) ? width : height;

    size_t imgsz = VIPS_IMAGE_SIZEOF_PEL(out)*width*height;
    size_t imgmax = 500*1024*1024;
    bool memory_storage = (imgsz < imgmax) ? true : false;
    int fd = -1;
    char fname[500]; fname[0] = '\0';

    std::cout<<"ImagePyramid: level #"<<levels.size()<<" size: "<<imgsz/1024/1024<<"MB (max="<<imgmax/1024/1024<<"MB)"<<std::endl;
    if( memory_storage ) {
      size_t array_sz;
      std::cout<<"ImagePyramid: saving cache buffer..."<<std::endl;
      void* mem_array = vips_image_write_to_memory( out, &array_sz );
      std::cout<<"ImagePyramid: cache buffer saved."<<std::endl;
      snprintf(tstr,499,"PF::ImagePyramid::get_level(%d) level #%d (after write_to_memory)",level, (int)levels.size());
      PF_UNREF( out, tstr );
      in = vips_image_new_from_memory( mem_array, array_sz, width, height, img->Bands, img->BandFmt );
      std::cout<<"ImagePyramid: cache buffer loaded."<<std::endl;
      g_signal_connect( in, "postclose", G_CALLBACK(free_mem_array), mem_array );
      std::cout<<"ImagePyramid: postclose callback attached to cache buffer."<<std::endl;
    } else {
      sprintf( fname,"%spfraw-XXXXXX", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
      fd = pf_mkstemp( fname );
      if( fd < 0 )
        return NULL;
      std::cout<<"ImagePyramid: cache file: "<<fname<<std::endl;

      std::cout<<"ImagePyramid: saving cache file..."<<std::endl;
      vips_rawsave_fd( out, fd, NULL );
      std::cout<<"ImagePyramid: cache file saved."<<std::endl;
      //char tifname[500];
      //sprintf(tifname,"/tmp/level_%d-1.tif",(int)levels.size());
      //vips_image_write_to_file( out, tifname );
      //g_object_unref( out );
      snprintf(tstr,499,"PF::ImagePyramid::get_level(%d) level #%d (after rawsave)",level, (int)levels.size());
      PF_UNREF( out, tstr );

      vips_rawload( fname, &in, width, height, VIPS_IMAGE_SIZEOF_PEL( img ), NULL );
      std::cout<<"ImagePyramid: cache file loaded."<<std::endl;
      //unlink( fname );
    }

    vips_copy( in, &out,
        "format", img->BandFmt,
	       "bands", img->Bands,
	       "coding", img->Coding,
	       "interpretation", img->Type,
	       NULL );
#ifndef NDEBUG
    std::cout<<"ImagePyramid::get_level("<<level<<") raw load in="<<in<<"  out="<<out<<std::endl;
#endif
    //sprintf(tifname,"/tmp/level_%d-2.tif",(int)levels.size());
    //vips_image_write_to_file( out, tifname );
    //g_object_unref( in );
    snprintf(tstr,499,"PF::ImagePyramid::get_level(%d) level #%d (after vips_copy)",level, (int)levels.size());
    PF_UNREF( in, tstr );

    PF::vips_copy_metadata( img, out );

    /*
    if( profile_data ) {
      void* profile_data2 = malloc( profile_length );
      if( profile_data2 ) {
        memcpy( profile_data2, profile_data, profile_length );
        vips_image_set_blob( out, VIPS_META_ICC_NAME,
            (VipsCallbackFn) g_free,
            profile_data2, profile_length );
      }
    }

    if( image_data ) {
      void* image_data2 = malloc( blobsz );
      if( image_data2 ) {
        memcpy( image_data2, image_data, blobsz );
        vips_image_set_blob( out, "raw_image_data",
            (VipsCallbackFn) g_free,
            image_data2, blobsz );
      }
    }

    if( exif_data ) {
      void* exif_data2 = malloc( exifsz );
      if( exif_data2 ) {
        memcpy( exif_data2, exif_data, exifsz );
        vips_image_set_blob( out, PF_META_EXIF_NAME,
            (VipsCallbackFn) PF::exif_free,
            exif_data2, exifsz );
      }
    }
    */

    newlevel.image = out;
    newlevel.fd = fd;
    newlevel.raw_file_name = fname;
    levels.push_back( newlevel );

    in = out;

    // If the desired level is reached, we stop
    if( levels.size() == (level+1) )
      break;
  }

  timer.stop();
  std::cout<<"ImagePyramid::get_level(): processing of level #"<<level<<" took "<<timer.elapsed()<<" s"<<std::endl;

  // If the highest available level is smaller than the requested one, it means that
  // the resulting image would be too small and was not computed.
  // In this case, we set the "level" variable to the smallest available one,
  // so that the information of which level is actually returned is transmitted to the caller
  if( levels.size() < (level+1) )
    level = levels.size() - 1;

  // We add a reference to the returned pyramid level, since it must be kept alive until
  // the pyramid is re-built, in which case it will be unreff'd by the pyramid itself
  if( levels[level].raw_file_name.empty() )
    snprintf(tstr,499,"ImagePyramid::get_level(): levels[%d].image ref",(int)level);
  else
    snprintf(tstr,499,"ImagePyramid::get_level(): levels[%d].image ref (%s)",
             (int)level,levels[level].raw_file_name.c_str());
  PF_REF( levels[level].image, tstr );
  return( &(levels[level]) );
}
