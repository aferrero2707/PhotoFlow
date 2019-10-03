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
#include "../base/roi.hh"
#include "../external/librtprocess/src/include/librtprocess.h"

//#define RT_EMU 1


PF::AmazeDemosaicPar::AmazeDemosaicPar(): 
DemosaicBasePar(16)
{
  set_type( "amaze_demosaic" );
}


static bool setProgCancel_dummy(double)
{
  return true;
}


void PF::amaze_demosaic_PF(VipsRegion* ir, VipsRegion* out, PF::AmazeDemosaicPar* par)
{
  //std::cout<<"amaze_demosaic_PF() called"<<std::endl;

  //rtengine::RawImageSource rawimg; rawimg.set_image_data( par->get_image_data() ); rawimg.amaze_demosaic( ir, out ); return;

  int offsx = ir->valid.left;
  int offsy = ir->valid.top;
  int rw = ir->valid.width;
  int rh = ir->valid.height;
  int w = ir->im->Xsize;
  int h = ir->im->Ysize;
  float* p = (float*)VIPS_REGION_ADDR( ir, offsx, offsy );
  int rowstride = VIPS_REGION_LSKIP(ir) / sizeof(float);

  /*// Copy the raw pixel data from the input region
  float* rawbuf = new float[rw*rh];
  for( int y = 0; y < ir->valid.height; y++ ) {
    float* ptr = (float*)VIPS_REGION_ADDR( out, out->valid.left, y+out->valid.top );
    for( int x = 0, xx = 0; x < ir->valid.width; x++, xx+=2 ) {

    }
  }*/

  //librtprocess::array2D<float> rawData( w, h, rw, rh, p, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  PF::PixelBuffer rawData(p, rw, rh, rowstride, offsy, offsx);
  PF::rp_roi_rect_t roi_rect = { ir->valid.left, ir->valid.top, ir->valid.width, ir->valid.height };
  PF::rp_roi_rect_t roi_rect_in = { 0, 0, ir->valid.width, ir->valid.height };
  PF::rp_roi_t* roi = PF::rp_roi_new_from_data( &roi_rect, &roi_rect_in, 2, rowstride, 1, p );
  if( false && offsx < 20 && offsy < 20 ) {
    std::cout<<"top= "<<ir->valid.top<<std::endl;
    std::cout<<"left="<<ir->valid.left<<std::endl;
    std::cout<<"roi->data[0]["<<ir->valid.top<<"]["<<ir->valid.left<<"="<<roi->data[0][ir->valid.top][ir->valid.left]<<std::endl;
    std::cout<<"p[0]="<<p[0]<<std::endl;
    std::cout<<"p[1]="<<p[1]<<std::endl;
    std::cout<<"p[2]="<<p[2]<<std::endl;
    std::cout<<"p[3]="<<p[3]<<std::endl;

    std::cout<<"amaze_demosaic_PF: RAW image = "<<ir->im<<std::endl;
    std::cout<<"amaze_demosaic_PF: RAW size = "<<w<<" x "<<h<<std::endl;
    std::cout<<"amaze_demosaic_PF: input region = "<<rw<<" x "<<rh<<" + "<<offsx<<" + "<<offsy<<std::endl;
    std::cout<<"amaze_demosaic_PF: rowstride = "<<rowstride<<std::endl;
  }

  offsx = out->valid.left;
  offsy = out->valid.top;
  rw = out->valid.width;
  rh = out->valid.height;
  w = out->im->Xsize;
  h = out->im->Ysize;
  rowstride = rw;

  float* pr = new float[rw*rh];
  float* pg = new float[rw*rh];
  float* pb = new float[rw*rh];
  //librtprocess::array2D<float> red( w, h, rw, rh, pr, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  //librtprocess::array2D<float> green( w, h, rw, rh, pg, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  //librtprocess::array2D<float> blue( w, h, rw, rh, pb, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  PF::PixelBuffer red(pr, rw, rh, rowstride, offsy, offsx);
  PF::PixelBuffer green(pg, rw, rh, rowstride, offsy, offsx);
  PF::PixelBuffer blue(pb, rw, rh, rowstride, offsy, offsx);

  if( false && offsx < 20 && offsy < 20 ) {
    std::cout<<"amaze_demosaic_PF: output image = "<<out->im<<std::endl;
    std::cout<<"amaze_demosaic_PF: output size = "<<w<<" x "<<h<<std::endl;
    std::cout<<"amaze_demosaic_PF: output region = "<<rw<<" x "<<rh<<" + "<<offsx<<" + "<<offsy<<std::endl;
    std::cout<<"amaze_demosaic_PF: rowstride = "<<rowstride<<std::endl;
  }

  int filters = par->get_image_data()->idata.filters;
  unsigned cfarray[2][2];
  PF::init_CFA(filters, cfarray);

  std::function<bool(double)> setProgCancel = setProgCancel_dummy;

  float initGain = 1;
  int border = 16;
  float inputScale = 1;
  float outputScale = 1;
  amaze_demosaic(w, h, offsx, offsy, rw, rh,
      roi->data[0], red.get_rows(), green.get_rows(), blue.get_rows(), cfarray,
      setProgCancel, initGain, border, inputScale, outputScale);

  // Copy back the pixel data into the output region
  int x, xx, y;
  for( y = 0; y < out->valid.height; y++ ) {
    float* ptr = (float*)VIPS_REGION_ADDR( out, out->valid.left, y+out->valid.top );
    for( x = 0, xx = 0; x < out->valid.width; x++, xx+=3 ) {
      ptr[xx]   = red[y+out->valid.top][x+out->valid.left];
      ptr[xx+1] = green[y+out->valid.top][x+out->valid.left];
      ptr[xx+2] = blue[y+out->valid.top][x+out->valid.left];
    }
  }

  PF::rp_roi_free(roi);

  delete [] pr;
  delete [] pg;
  delete [] pb;
}

