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

#include "../base/new_operation.hh"
#include "padded_op.hh"


PF::PaddedOpPar::PaddedOpPar(): OpParBase()
{
}




VipsImage* PF::PaddedOpPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  VipsImage* srcimg = in[0];

  std::vector<VipsImage*> in2;

  int padding = get_padding(0);

  //std::cout<<"padding: "<<padding<<std::endl;
  //std::cout<<"srcimg->Xsize: "<<srcimg->Xsize<<std::endl;

  // Extend the image by two pixels to account for the pixel averaging window
  // of the impulse noise reduction algorithm
  VipsImage* extended;
  VipsExtend extend = VIPS_EXTEND_COPY;
  if( vips_embed(srcimg, &extended, padding, padding,
      srcimg->Xsize+padding*2, srcimg->Ysize+padding*2,
      "extend", extend, NULL) ) {
    std::cout<<"PaddedOpPar::build(): vips_embed() failed."<<std::endl;
    PF_REF( in[0], "PaddedOpPar::build(): vips_embed() failed." );
    return NULL;
  }
  //std::cout<<"extended->Xsize: "<<extended->Xsize<<std::endl;

  set_image_hints( extended );
  set_format( get_format() );
  in2.clear();
  in2.push_back( extended );
  VipsImage* padded = PF::OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( extended, "PaddedOpPar::build(): extended unref after op" );

//  std::cout<<"defr->Xsize: "<<defr->Xsize<<std::endl;

  // Final cropping to remove the padding pixels
  VipsImage* cropped;
  if( vips_crop(padded, &cropped, padding, padding,
      srcimg->Xsize, srcimg->Ysize, NULL) ) {
    std::cout<<"PaddedOpPar::build(): vips_crop() failed."<<std::endl;
    PF_UNREF( padded, "PaddedOpPar::build(): padded unref" );
    PF_REF( in[0], "PaddedOpPar::build(): vips_crop() failed" );
    return in[0];
  }
  PF_UNREF( padded, "PaddedOpPar::build(): padded unref" );

  //std::cout<<"cropped->Xsize: "<<cropped->Xsize<<std::endl;

  VipsImage* out = cropped;
//  std::cout<<"out->Xsize: "<<out->Xsize<<std::endl;

  set_image_hints( out );

  return out;
}
