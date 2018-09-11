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
#include "../external/librtprocess/src/librtprocess.h"

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
  int offsx = ir->valid.left;
  int offsy = ir->valid.top;
  int rw = ir->valid.width;
  int rh = ir->valid.height;
  int w = ir->im->Xsize;
  int h = ir->im->Ysize;
  float* p = (float*)VIPS_REGION_ADDR( ir, offsx, offsy );
  int rowstride = VIPS_REGION_LSKIP(ir) / sizeof(float);

  librtprocess::array2D<float> rawData( w, h, rw, rh, p, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );

  if( false && offsx < 20 && offsy < 20 ) {
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
  librtprocess::array2D<float> red( w, h, rw, rh, pr, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  librtprocess::array2D<float> green( w, h, rw, rh, pg, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );
  librtprocess::array2D<float> blue( w, h, rw, rh, pb, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );

  if( false && offsx < 20 && offsy < 20 ) {
    std::cout<<"amaze_demosaic_PF: output image = "<<out->im<<std::endl;
    std::cout<<"amaze_demosaic_PF: output size = "<<w<<" x "<<h<<std::endl;
    std::cout<<"amaze_demosaic_PF: output region = "<<rw<<" x "<<rh<<" + "<<offsx<<" + "<<offsy<<std::endl;
    std::cout<<"amaze_demosaic_PF: rowstride = "<<rowstride<<std::endl;
  }

  int filters = par->get_image_data()->idata.filters;
  librtprocess::ColorFilterArray cfarray(filters);

  std::function<bool(double)> setProgCancel = setProgCancel_dummy;

  float initGain = 1;
  int border = 16;
  float inputScale = 1;
  float outputScale = 1;
  librtprocess::amaze_demosaic(offsx, offsy, rw, rh, rawData, red, green, blue, cfarray,
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

  delete [] pr;
  delete [] pg;
  delete [] pb;
}

