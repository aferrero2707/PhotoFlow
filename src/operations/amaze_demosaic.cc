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

#include "amaze_demosaic.hh"
#include "../base/processor.hh"

//#define RT_EMU 1


PF::AmazeDemosaicPar::AmazeDemosaicPar(): 
  OpParBase()
{
  set_demand_hint( VIPS_DEMAND_STYLE_SMALLTILE );
  set_type( "amaze_demosaic" );
}


VipsImage* PF::AmazeDemosaicPar::build(std::vector<VipsImage*>& in, int first, 
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  size_t blobsz;
  if( vips_image_get_blob( in[0], "raw_image_data",
         (void**)&image_data,
         &blobsz ) ) {
    std::cout<<"AmazeDemosaicPar::build(): could not extract raw_image_data."<<std::endl;
    return NULL;
  }
  if( blobsz != sizeof(dcraw_data_t) ) {
    std::cout<<"AmazeDemosaicPar::build(): wrong raw_image_data size."<<std::endl;
    return NULL;
  }

  std::cout<<"AmazeDemosaicPar::build(): filters="<<image_data->idata.filters<<std::endl;

  int border = 16;

  //VipsImage **t = (VipsImage **)
  //  vips_object_local_array( VIPS_OBJECT( in[0] ), 12 );
  VipsImage* t[12];

  //  TOP BAND
  int i0 = 0;
  VipsImage* input = in[0];
  // Extract an horizontal top band at (0,1) and with size (in[0]->Xsize,border)
  if( vips_crop(input, &t[i0], 0, 1, input->Xsize, border, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #1" );
  // Flip the band vertically
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#1) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #1" );
  // Put the vertical band above the original image
  if( vips_join(t[i0+1], in[0], &t[i0+2], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #1-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #1" );


  //  BOTTOM BAND
  i0 += 3;
  // Extract an horizontal bottom band at (0,in[0]->Ysize-border-2) and with size (in[0]->Xsize,border)
  if( vips_crop(in[0], &t[i0], 0, in[0]->Ysize-border-1, in[0]->Xsize, border, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#2) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] unref #2" );
  // Flip the band vertically
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#2) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #2" );
  // Put the vertical band below the previously joined image
  if( vips_join(t[i0-1], t[i0+1], &t[i0+2], VIPS_DIRECTION_VERTICAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#2) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #2" );


  //  LEFT BAND
  i0 += 3;
  // Extract a vertical left band at (1,0) and with size (border,t[i0-1]->Ysize)
  //PF_PRINT_REF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] refcount before crop" );
  if( vips_crop(t[i0-1], &t[i0], 1, 0, border, t[i0-1]->Ysize, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#3) failed"<<std::endl;
    return NULL;
  }
  //PF_PRINT_REF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] refcount after crop" );
  //PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #3" );
  // Flip the band horizontally
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#3) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #3" );
  // Put the vertical band left of the previously joined image
  if( vips_join(t[i0+1], t[i0-1], &t[i0+2], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#3) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #3-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #3" );


  //  RIGHT BAND
  i0 += 3;
  // Extract a vertical right band at (t[i0-1]->Xsize-2,0) and with size (border,t[i0-1]->Ysize)
  if( vips_crop(t[i0-1], &t[i0], t[i0-1]->Xsize-border-1, 0, border, t[i0-1]->Ysize, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop(#4) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #4" );
  // Flip the band horizontally
  if( vips_flip(t[i0], &t[i0+1], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_flip(#4) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0], "AmazeDemosaicPar::build(): t[i0] unref #4" );
  // Put the vertical band right of the previously joined image
  if( vips_join(t[i0-1], t[i0+1], &t[i0+2], VIPS_DIRECTION_HORIZONTAL, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_join(#4) failed"<<std::endl;
    return NULL;
  }
  PF_UNREF( t[i0-1], "AmazeDemosaicPar::build(): t[i0-1] unref #4-2" );
  PF_UNREF( t[i0+1], "AmazeDemosaicPar::build(): t[i0+1] unref #4" );

  //std::cout<<"i0+2="<<i0+2<<std::endl;

  // Copy to extended image
  VipsImage* extended;
  if( vips_copy(t[i0+2], &extended, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_copy(#1) failed"<<std::endl;
    return NULL;
  }
  //PF_UNREF( in[0], "AmazeDemosaicPar::build(): in[0] after vips_copy()" );
  PF_UNREF( t[i0+2], "AmazeDemosaicPar::build(): t[i0+2] after vips_copy()" );

  set_image_hints( extended );

  std::vector<VipsImage*> in2; in2.push_back(extended);
  VipsImage* img = OpParBase::build( in2, first, NULL, NULL, level );
  PF_UNREF( extended, "AmazeDemosaicPar::build(): extended unref" );
  if( !img ) return NULL;

  VipsImage* cropped = img;
  /**/
  int result;
  result = vips_crop(img, &cropped, border, border, in[0]->Xsize, in[0]->Ysize, NULL);
  PF_UNREF( img, "AmazeDemosaicPar::build(): img unref" )
  if( result ) {
    std::cout<<"AmazeDemosaicPar::build(): vip_crop() failed"<<std::endl;
    return NULL;
  }
  /**/

  VipsImage* out;
  int bands = 3;
  VipsCoding coding = VIPS_CODING_NONE;
  VipsInterpretation interpretation = VIPS_INTERPRETATION_RGB;
  VipsBandFormat format = VIPS_FORMAT_FLOAT;
  vips_copy( cropped, &out,
	     "format", format,
	     "bands", bands,
	     "coding", coding,
	     "interpretation", interpretation,
	     NULL );
  //sprintf(tifname,"/tmp/level_%d-2.tif",(int)levels.size());
  //vips_image_write_to_file( out, tifname );
  //g_object_unref( img );
  PF_UNREF( cropped, "AmazeDemosaicPar::build(): cropped unref" );


  int tw = 160 - 32;
  int th = tw;
  int nt = (cropped->Xsize/tw + 1) * 3;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  int threaded = 1, persistent = 0;
  VipsImage* cached;
  if( vips_tilecache(out, &cached,
      "tile_width", tw, "tile_height", th, "max_tiles", nt,
      "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
    std::cout<<"AmazeDemosaicPar::build(): vips_tilecache() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( out, "AmazeDemosaicPar::build(): out unref" );

  return cached;

  //return out;
}



PF::ProcessorBase* PF::new_amaze_demosaic()
{
  return( new PF::Processor<PF::AmazeDemosaicPar,PF::AmazeDemosaicProc>() );
}

