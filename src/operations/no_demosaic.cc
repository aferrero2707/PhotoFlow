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

#include <stdlib.h>

#include "no_demosaic.hh"
#include "../base/processor.hh"

//#define RT_EMU 1


PF::NoDemosaicPar::NoDemosaicPar():
  OpParBase()
{
  set_demand_hint( VIPS_DEMAND_STYLE_SMALLTILE );
  set_type( "no_demosaic" );
}


VipsImage* PF::NoDemosaicPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  size_t blobsz;
  if( PF_VIPS_IMAGE_GET_BLOB( in[0], "raw_image_data", &image_data, &blobsz ) ) {
    std::cout<<"NoDemosaicPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"NoDemosaicPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }

  VipsImage* input = in[0];

  std::vector<VipsImage*> in2; in2.push_back(input);
  VipsImage* img = OpParBase::build( in2, first, NULL, NULL, level );
  if( !img ) return NULL;

  VipsImage* out;
  int bands = 3;
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_RGB;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  vips_copy( img, &out,
	     "format", format,
	     "bands", bands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  PF_UNREF( img, "NoDemosaicPar::build(): img unref" );

  return out;
}

