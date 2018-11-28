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

#ifndef PF_GUIDED_FILTER_H
#define PF_GUIDED_FILTER_H

#include "../base/operation.hh"

#include "padded_op.hh"

namespace PF 
{

class GuidedFilterPar: public PaddedOpPar
{
  Property<float> radius;
  float radius_real;
  Property<float> threshold;

  ICCProfile* icc_data;
public:
  GuidedFilterPar();

  ICCProfile* get_icc_data() { return icc_data; }

  bool has_intensity() { return false; }
  bool needs_caching();

  float get_radius() { return radius_real; }
  float get_threshold() { return threshold.get(); }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class GuidedFilterProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
  }
};


void guidedFilter(const PixelMatrix<float> &guide, const PixelMatrix<float> &src,
    PixelMatrix<float> &dst, int r, float epsilon, int subsampling);


template < OP_TEMPLATE_DEF_TYPE_SPEC >
class GuidedFilterProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    //std::cout<<"GuidedFilterProc::render: this="<<this<<"  in_first="<<in_first<<std::endl;

    if( n != 1 ) return;
    if( ireg[0] == NULL ) return;

    GuidedFilterPar* opar = dynamic_cast<GuidedFilterPar*>(par);
    if( !opar ) return;

    int es = VIPS_IMAGE_SIZEOF_ELEMENT( ireg[0]->im );
    int ips = VIPS_IMAGE_SIZEOF_PEL( ireg[0]->im );
    const int ops = VIPS_IMAGE_SIZEOF_PEL( oreg->im );

    //const int order = 1; // 0,1,2
    const float radius = opar->get_radius();
    const float thresfold = opar->get_threshold();

    int offsx = ireg[0]->valid.left;
    int offsy = ireg[0]->valid.top;
    int rw = ireg[0]->valid.width;
    int rh = ireg[0]->valid.height;
    float* p = (float*)VIPS_REGION_ADDR( ireg[0], offsx, offsy );
    int rowstride = VIPS_REGION_LSKIP(ireg[0]) / sizeof(float);
    offsx = 0;
    offsy = 0;
    PixelMatrix<float> rgbin(p, rw, rh, rowstride, offsy, offsx);
    PixelMatrix<float> rin(rw, rh, offsy, offsx);
    PixelMatrix<float> gin(rw, rh, offsy, offsx);
    PixelMatrix<float> bin(rw, rh, offsy, offsx);
    PixelMatrix<float> rguide(rw, rh, offsy, offsx);
    PixelMatrix<float> gguide(rw, rh, offsy, offsx);
    PixelMatrix<float> bguide(rw, rh, offsy, offsx);
    PixelMatrix<float> rout(rw, rh, offsy, offsx);
    PixelMatrix<float> gout(rw, rh, offsy, offsx);
    PixelMatrix<float> bout(rw, rh, offsy, offsx);

    ICCProfile* profile = opar->get_icc_data();

    //std::cout<<"GuidedFilterProc::render: splitting RGB channels"<<std::endl;

    int x, y, z;

    for(y = 0; y < rh; y++) {
      float* row = rgbin[y];
      float* rr = rin[y];
      float* gr = gin[y];
      float* br = bin[y];
      float* rrg = rguide[y];
      float* grg = gguide[y];
      float* brg = bguide[y];
      for( x = 0; x < rw; x++ ) {
        //std::cout<<"  y="<<y<<"  x="<<x<<"  row="<<row<<"  rr="<<rr<<"  gr="<<gr<<"  br="<<br<<std::endl;
        //if(x==0 && y==0) std::cout<<"  row="<<row[0]<<" "<<row[1]<<" "<<row[2]<<std::endl;
        *rr = row[0];
        *gr = row[1];
        *br = row[2];
        if(true && profile && profile->is_linear()) {
          *rrg = powf( *rr, 1./2.4 );
          *grg = powf( *gr, 1./2.4 );
          *brg = powf( *br, 1./2.4 );
        } else {
          *rrg = *rr;
          *grg = *gr;
          *brg = *br;
        }
        row += 3;
        rr += 1;
        gr += 1;
        br += 1;
        rrg += 1;
        grg += 1;
        brg += 1;
      }
    }

    int r = opar->get_radius();
    float epsilon = opar->get_threshold();
    int subsampling = 1;

    //std::cout<<"GuidedFilterProc::render: processing RGB channels"<<std::endl;

    guidedFilter(rguide, rin, rout, r, epsilon, subsampling);
    guidedFilter(gguide, gin, gout, r, epsilon, subsampling);
    guidedFilter(bguide, bin, bout, r, epsilon, subsampling);

    //return;

    int dx = oreg->valid.left - ireg[0]->valid.left;
    int dy = oreg->valid.top - ireg[0]->valid.top;
    offsx = oreg->valid.left;
    offsy = oreg->valid.top;
    rw = oreg->valid.width;
    rh = oreg->valid.height;
    p = (float*)VIPS_REGION_ADDR( oreg, offsx, offsy );
    rowstride = VIPS_REGION_LSKIP(oreg) / sizeof(float);
    PF::PixelMatrix<float> rgbout(p, rw, rh, rowstride, 0, 0);

    for(y = 0; y < rh; y++) {
      float* irow = rgbin[y+dy];
      float* rr = rout[y+dy];
      float* gr = gout[y+dy];
      float* br = bout[y+dy];
      float* row = rgbout[y];
      //float* rr = rin[y+dy];
      //float* gr = gin[y+dy];
      //float* br = bin[y+dy];
      rr += dx; gr += dx; br += dx; irow += dx*3;
      for( x = 0; x < rw; x++ ) {
        row[0] =
            *rr;
        row[1] =
            *gr;
        row[2] =
            *br;
        //row[0] = irow[0];
        //row[1] = irow[1];
        //row[2] = irow[2];
        row += 3;
        irow += 3;
        rr += 1;
        gr += 1;
        br += 1;
      }
    }
  }
};


ProcessorBase* new_guided_filter();

}

#endif 


