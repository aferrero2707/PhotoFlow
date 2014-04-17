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

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "imagepyramid.hh"

VipsImage* PF::pyramid_test_image = NULL;
GObject* PF::pyramid_test_obj = NULL;



PF::ImagePyramid::~ImagePyramid()
{
  for( unsigned int i = 0; i < levels.size(); i++ ) {
    g_object_unref( levels[i].image );
    unlink( levels[i].raw_file_name.c_str() );
  }
}


void PF::ImagePyramid::init( VipsImage* img )
{
  for( unsigned int i = 1; i < levels.size(); i++ ) {
    g_object_unref( levels[i].image );
    unlink( levels[i].raw_file_name.c_str() );
  }
  levels.clear();

  PF::PyramidLevel level;
  level.image = img;
  levels.push_back( level );

#ifndef NDEBUG
  std::cout<<"ImagePyramid::init(): full-scale image="<<img<<std::endl;
#endif
}


PF::PyramidLevel* PF::ImagePyramid::get_level( unsigned int& level )
{  
  if( levels.size() > 1 ) {
    pyramid_test_image = levels[1].image;
    pyramid_test_obj = G_OBJECT( pyramid_test_image );
  }

  if( levels.empty() ) 
    return NULL;
  
  if( level < levels.size() ) {
    // We add a reference to the returned pyramid level, since it must be kept alive until
    // the pyramid is re-built, in which case it will be unreff'd by the pyramid itself
#ifndef NDEBUG
    std::cout<<"ImagePyramid::get_level("<<level<<") cached="<<levels[level].image<<std::endl;
#endif
    g_object_ref( levels[level].image );
    return( &(levels[level]) );
  }
  
  PF::PyramidLevel newlevel;
  void *profile_data;
  size_t profile_length;
  
  VipsImage* img = levels[0].image;
  
  if( vips_image_get_blob( img, VIPS_META_ICC_NAME, 
			   &profile_data, &profile_length ) )
    profile_data = NULL;
  
  VipsImage* in = levels.back().image;
  if( !in )
    return NULL;

  int width = in->Xsize;
  int height = in->Ysize;
  int size = (width>height) ? width : height;
  int nbands = in->Bands;

  char tstr[500];
  while( size > 256 ) {
    VipsImage* out;
    if( vips_subsample( in, &out, 2, 2, NULL ) )
      return NULL;
#ifndef NDEBUG
    std::cout<<"ImagePyramid::get_level("<<level<<") subsample in="<<in<<"  out="<<out<<std::endl;
#endif
    //g_object_unref( in );
    width = out->Xsize;
    height = out->Ysize;
    size = (width>height) ? width : height;
   
    char* fname = tempnam( NULL, "pfraw" );
    if( !fname ) 
      return NULL;
    vips_rawsave( out, fname, NULL );
    //char tifname[500];
    //sprintf(tifname,"/tmp/level_%d-1.tif",(int)levels.size());
    //vips_image_write_to_file( out, tifname );
    g_object_unref( out );
    
    vips_rawload( fname, &in, width, height, VIPS_IMAGE_SIZEOF_PEL( img ), NULL );
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
    g_object_unref( in );
    if( profile_data ) {
      void* profile_data2 = malloc( profile_length );
      if( profile_data2 ) {
	memcpy( profile_data2, profile_data, profile_length );
	vips_image_set_blob( out, VIPS_META_ICC_NAME, 
			     (VipsCallbackFn) g_free, 
			     profile_data2, profile_length );
      }
    }

    newlevel.image = out;
    newlevel.raw_file_name = fname;
    levels.push_back( newlevel );

    in = out;
    
    // If the desired level is reached, we stop
    if( levels.size() == (level+1) )
      break;
  }

  // If the highest available level is smaller than the requested one, it means that 
  // the resulting image would be too small and was not computed.
  // In this case, we set the "level" variable to the smallest available one,
  // so that the information of which level is actually returned is transmitted to the caller
  if( levels.size() < (level+1) )
    level = levels.size() - 1;

  // We add a reference to the returned pyramid level, since it must be kept alive until
  // the pyramid is re-built, in which case it will be unreff'd by the pyramid itself
  g_object_ref( levels[level].image );
  return( &(levels[level]) );
}
