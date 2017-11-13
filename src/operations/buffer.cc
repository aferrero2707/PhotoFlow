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

#include "buffer.hh"
#include "../base/processor.hh"

//#include "../vips/vips_layer.h"

VipsImage* PF::BufferPar::build(std::vector<VipsImage*>& in, int first, 
				VipsImage* imap, VipsImage* omap, 
				unsigned int& level)
{
  if( in.size() != 1 ) {
    std::cerr<<"PF::BufferPar::build(): wrong number of input images: "
	     <<in.size()<<" (should be 1)"<<std::endl;
    return NULL;
  } 

#ifndef NDEBUG    
  std::cout<<"BufferPar::build(): type="<<get_type()<<"  format="<<get_format()<<std::endl;
#endif

  g_object_ref( in[0] ); return in[0];

  set_image_hints( in[0] );
  VipsImage* out = PF::OpParBase::build( in, first, NULL, NULL, level );
  return out;
}
