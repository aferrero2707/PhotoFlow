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

#include "ca_correct.hh"
#include "../base/processor.hh"
#include "../external/librtprocess/src/librtprocess.h"

//#define RT_EMU 1


PF::CACorrectPar::CACorrectPar():
DemosaicBasePar(8, false),
  enable_ca( "enable_ca", this, false ),
  auto_ca( "auto_ca", this, true ),
  ca_red( "ca_red", this, 0 ),
  ca_blue( "ca_blue", this, 0 )
{
  set_type( "ca_correct" );
}


VipsImage* PF::CACorrectPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  void *data;
  size_t data_length;
  
  if( in.size()<1 || in[0]==NULL ) return NULL;
  
  if( !enable_ca.get() ) {
    PF_REF( in[0], "CACorrectPar::build(): in[0] ref when disabled" );
    return in[0];
  }

  VipsImage* img = DemosaicBasePar::build( in, first, NULL, NULL, level );
  return img;
}


static bool setProgCancel_dummy(double)
{
  return true;
}


void PF::ca_correct_PF(VipsRegion* ir, VipsRegion* out, PF::CACorrectPar* par)
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

  if( true && offsx < 20 && offsy < 20 ) {
    std::cout<<"ca_correct_PF: RAW image = "<<ir->im<<std::endl;
    std::cout<<"ca_correct_PF: RAW size = "<<w<<" x "<<h<<std::endl;
    std::cout<<"ca_correct_PF: input region = "<<rw<<" x "<<rh<<" + "<<offsx<<" + "<<offsy<<std::endl;
    std::cout<<"ca_correct_PF: rowstride = "<<rowstride<<std::endl;
  }

  offsx = out->valid.left;
  offsy = out->valid.top;
  rw = out->valid.width;
  rh = out->valid.height;
  w = out->im->Xsize;
  h = out->im->Ysize;
  p = (float*)VIPS_REGION_ADDR( out, offsx, offsy );
  rowstride = VIPS_REGION_LSKIP(out) / sizeof(float);

  librtprocess::array2D<float> rawDataOut( w, h, rw, rh, p, rowstride, offsx, offsy, ARRAY2D_BYREFERENCE );

  if( true && offsx < 20 && offsy < 20 ) {
    std::cout<<"ca_correct_PF: output image = "<<out->im<<std::endl;
    std::cout<<"ca_correct_PF: output size = "<<w<<" x "<<h<<std::endl;
    std::cout<<"ca_correct_PF: output region = "<<rw<<" x "<<rh<<" + "<<offsx<<" + "<<offsy<<std::endl;
    std::cout<<"ca_correct_PF: rowstride = "<<rowstride<<std::endl;
  }

  int filters = par->get_image_data()->idata.filters;
  librtprocess::ColorFilterArray cfarray(filters);
  CaFitParams fitParams;
  for(int i = 0; i < 2; i++) {
    for(int j = 0; j < 2; j++) {
      for(int k = 0; k < 16; k++) {
        fitParams[i][j][k] = par->get_image_data()->color.ca_fitparams[i][j][k];
      }
    }
  }

  for( int y = 0; y < rh; y++ ) {
    for( int x = 0; x < rw; x++ ) {
      rawDataOut[y+offsy][x+offsx] = rawData[y+offsy][x+offsx];
    }
  }
  //return;

  std::function<bool(double)> setProgCancel = setProgCancel_dummy;

  bool fitParamsIn = true;
  float initGain = 1;
  int border = 16;
  float inputScale = 1;
  float outputScale = 1;
  librtprocess::CA_correct(offsx, offsy, rw, rh,
      par->get_auto_ca(), par->get_ca_red(), par->get_ca_blue(), rawData, rawDataOut,
      cfarray, setProgCancel, fitParams, fitParamsIn, inputScale, outputScale );
}

