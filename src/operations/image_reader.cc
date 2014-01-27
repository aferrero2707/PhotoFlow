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

void PF::ImageReaderPar::build(std::vector<VipsImage*>& in, int first, 
			       VipsImage* imap, VipsImage* omap)
{
  //VipsImage* outnew;

  // Create VipsImage from given file; unref previously opened image if any
  //if( image ) g_object_unref( image );
  image = vips_image_new_from_file( file_name.c_str() );
  //g_object_ref( image );
  

  std::cout<<"ImageReaderPar::build(): "<<std::endl
	   <<"input images:"<<std::endl;
  for(int i = 0; i < in.size(); i++) {
    std::cout<<"  "<<(void*)in[i]<<std::endl;
  }
  std::cout<<"imap: "<<(void*)imap<<std::endl<<"omap: "<<(void*)omap<<std::endl;
  std::cout<<"out: "<<(void*)image<<std::endl<<std::endl;

  set_image( image );
  return;

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
}
