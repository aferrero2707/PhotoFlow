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

#ifndef FAST_DEMOSAIC_XTRANS_H
#define FAST_DEMOSAIC_XTRANS_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../base/rawmatrix.hh"


#include <cstring>

namespace PF
{

class FastDemosaicXTransPar: public OpParBase
{
  bool normalize;

public:
  FastDemosaicXTransPar();
  bool has_intensity() { return false; }
  bool has_opacity() { return false; }
  bool needs_input() { return true; }

  bool do_normalize() { return normalize; }
  void set_normalize( bool n ) { normalize = n; }

  void set_image_hints( VipsImage* img )
  {
    if( !img ) return;
    OpParBase::set_image_hints( img );
    rgb_image( get_xsize(), get_ysize() );
    set_demand_hint( VIPS_DEMAND_STYLE_FATSTRIP );
  }

  /* Function to derive the output area from the input area
   */
   virtual void transform(const VipsRect* rin, VipsRect* rout, int /*id*/)
   {
     rout->left = rin->left+5;
     rout->top = rin->top+5;
     rout->width = rin->width-10;
     rout->height = rin->height-10;
   }

   /* Function to derive the area to be read from input images,
       based on the requested output area
    */
   virtual void transform_inv(const VipsRect* rout, VipsRect* rin, int /*id*/)
   {
     rin->left = rout->left-5;
     rin->top = rout->top-5;
     rin->width = rout->width+10;
     rin->height = rout->height+10;
   }



   VipsImage* build(std::vector<VipsImage*>& in, int first,
       VipsImage* imap, VipsImage* omap,
       unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class FastDemosaicXTransProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, FastDemosaicXTransPar* par)
  {
    //fast_demosaic( in, n, in_first,
    //	     imap, omap, out, par );
  }
};


#define fcol(r,c) ((int)(rawData[r].color(c)))


template < OP_TEMPLATE_DEF_TYPE_SPEC >
class FastDemosaicXTransProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ir, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, FastDemosaicXTransPar* par)
  {
    VipsRect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;

  //#ifndef NDEBUG
    if(r->left==0)std::cout<<"fast_demosaic_xtrans(): left="<<r->left<<"  top="<<r->top<<std::endl;
  //#endif

    VipsRect r_img = {0, 0, ir[in_first]->im->Xsize, ir[in_first]->im->Ysize};

    VipsRect r_raw = {r->left - 5, r->top - 5, r->width + 10, r->height + 10};
    vips_rect_intersectrect (&r_raw, &r_img, &r_raw);

    PF::RawMatrix rawData;
    rawData.init( r_raw.width, r_raw.height, r_raw.top, r_raw.left );
    for( int y = 0; y < r_raw.height; y++ ) {
      PF::raw_pixel_t* ptr = ir ? (PF::raw_pixel_t*)VIPS_REGION_ADDR( ir[0], r_raw.left, y+r_raw.top ) : NULL;
      rawData.set_row( y+r_raw.top, ptr );
    }

    int left = r->left;
    int right = r->left + width - 1;
    int top = r->top;
    int bottom = r->top + height - 1;

    for(int row = top; row <= bottom; row++) {
      float* ptr = (float*)VIPS_REGION_ADDR( oreg, left, row );
      for(int col = left, x = 0; col <= right; col++, x+= 3) {
        float sum[3] = {0.f};

        for(int v = -1; v <= 1; v++) {
          for(int h = -1; h <= 1; h++) {
            sum[fcol(row + v, col + h)] += rawData[row + v][(col + h)];
          }
        }

        switch(fcol(row, col)) {
        case 0:
          ptr[x] = rawData[row][col];
          ptr[x+1] = sum[1] * 0.2f;
          ptr[x+2] = sum[2] * 0.33333333f;
          break;

        case 1:
          ptr[x] = sum[0] * 0.5f;
          ptr[x+1] = rawData[row][col];
          ptr[x+2] = sum[2] * 0.5f;
          break;

        case 2:
          ptr[x] = sum[0] * 0.33333333f;
          ptr[x+1] = sum[1] * 0.2f;
          ptr[x+2] = rawData[row][col];
          break;
        }
        if( par->do_normalize() ) {
          ptr[x] /= 65535.f;
          ptr[x+1] /= 65535.f;
          ptr[x+2] /= 65535.f;
        }
        if( row < 10 && col < 10 )
          std::cout<<"r="<<row<<" c="<<col<<"  raw="
          <<rawData[row][col]<<"  rgb="<<ptr[x]<<","<<ptr[x+1]<<","<<ptr[x+2]<<std::endl;
      }
    }

  }
};


ProcessorBase* new_fast_demosaic_xtrans();
}


#endif
