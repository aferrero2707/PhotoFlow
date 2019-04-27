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

#include "../base/processor.hh"
#include "../base/processor_imp.hh"
#include "operations.hh"

#include "image_to_map.hh"

PF::ImageToMapPar::ImageToMapPar(): OpParBase(), profile(NULL)
{
  set_type( "image_to_map" );
}



VipsImage* PF::ImageToMapPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;
  void *profile_data;
  size_t profile_length;
  VipsImage* srcimg = in[0];
  if( !srcimg ) return NULL;
  profile = PF::get_icc_profile( srcimg );

  // No Lab conversion possible if the input image has no ICC profile
  if( !profile ) {
    std::cout<<"ImageToMapPar::build(): no profile data"<<std::endl;
    PF_REF(srcimg, "ImageToMapPar::build(): srcimg ref when no profile data");
    return srcimg;
  }

  set_image_hints( srcimg );
  out = PF::OpParBase::build( in, first, imap, omap, level );
  return out;
}



PF::ProcessorBase* PF::new_image_to_map()
{
  return new PF::Processor<ImageToMapPar, ImageToMapProc>();
}

