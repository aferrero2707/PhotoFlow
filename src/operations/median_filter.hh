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

#ifndef PF_MEDIAN_FILTER_H
#define PF_MEDIAN_FILTER_H

#include "../base/operation.hh"

#include "padded_op.hh"
#include "CTMF/ctmf.h"

namespace PF 
{

class MedianFilterPar: public PaddedOpPar
{
  Property<float> radius;
  float radius_real;
  Property<float> threshold;
  Property<bool> fast_approx;

  ICCProfile* icc_data;
  bool convert_to_perceptual;
public:
  MedianFilterPar();

  ICCProfile* get_icc_data() { return icc_data; }

  bool has_intensity() { return false; }
  bool needs_caching();
  void set_convert_to_perceptual(bool val) { convert_to_perceptual = val; }
  bool get_convert_to_perceptual() { return convert_to_perceptual; }

  void set_radius(float r) { radius.set(r); }
  float get_radius() { return radius_real; }
  void set_threshold(float t) { threshold.set(t); }
  float get_threshold() { return threshold.get(); }
  void set_fast_approx(bool f) { fast_approx.set(f); }
  bool get_fast_approx() { return fast_approx.get(); }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class MedianFilterProc
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
class MedianFilterProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
  ICCProfile* profile;
  MedianFilterPar* opar;

public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    //std::cout<<"MedianFilterProc::render: this="<<this<<"  in_first="<<in_first<<std::endl;

    if( n != 1 ) return;
    if( ireg[0] == NULL ) return;

    opar = dynamic_cast<MedianFilterPar*>(par);
    if( !opar ) return;

    int es = VIPS_IMAGE_SIZEOF_ELEMENT( ireg[0]->im );
    int ips = VIPS_IMAGE_SIZEOF_PEL( ireg[0]->im );
    const int ops = VIPS_IMAGE_SIZEOF_PEL( oreg->im );

    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    //const int order = 1; // 0,1,2
    //const float radius = opar->get_radius();
    //const float thresfold = opar->get_threshold();

    float epsilon = opar->get_threshold();
    int subsampling = 1;

    profile = opar->get_icc_data();
    if( !profile ) return;

    int offsx = ireg[0]->valid.left;
    int offsy = ireg[0]->valid.top;
    int dx = r->left - offsx;
    int dy = r->top - offsy;
    int rw = ireg[0]->valid.width;
    int rh = ireg[0]->valid.height;
    int iline_size = rw * ireg[0]->im->Bands;
    //float* p = (float*)VIPS_REGION_ADDR( ireg[0], offsx, offsy );
    //int rowstride = VIPS_REGION_LSKIP(ireg[0]) / sizeof(float);
    //offsx = 0;
    //offsy = 0;

    float* pin2;
    float* pout;

    int x, y, ri, ci, radius = opar->get_radius();
    int boxsz = radius*2 + 1;
    int rowstride = rw*ireg[0]->im->Bands;

    if( opar->get_fast_approx() ) {
    unsigned char* buf_in = new unsigned char[rh*rowstride];
    float* buf_out = new float[rh*rowstride];
    unsigned long memsize = 1024*1024;
    for( y = 0; y < rh; y++ ) {
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top + y );
      for( x = 0; x < iline_size; x+=1 ) {
        float val = pin2[x] * 255;
        buf_in[y*rowstride+x] = static_cast<unsigned char>( (val<0 ? 0 : val) > 255 ? 255 : val );
      }
    }

    //std::cout<<"width="<<r->width<<"  height="<<height<<"  rw="<<rw<<"  rh="<<rh<<"  radius="<<radius
    //    <<" dx="<<dx<<"  dy="<<dy<<std::endl;

    ctmf( buf_in, buf_out, rw, rh, rowstride, rowstride, radius, ireg[0]->im->Bands,
        opar->get_threshold()*255, memsize );

    for( y = 0; y < height; y++ ) {
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      for( x = 0; x < line_size; x+=1 ) {
        //std::cout<<"y="<<y<<"  x="<<x<<"  dy="<<dy<<"  dx="<<dx
        //    <<"  (y+dx)*rowstride+dx+x="<<(y+dx)*rowstride+dx+x
        //    <<"   rh*rowstride="<<rh<<"*"<<rowstride<<"="<<rh*rowstride<<std::endl;
        pout[x] = static_cast<float>(buf_out[(y+dy)*rowstride+dx+x]) / 255.0f;
        //std::cout<<"y="<<y<<"  x="<<x
        //    <<"  buf_out[(y+dy)*rowstride+dx+x]="<<buf_out[(y+dy)*rowstride+dx+x]
        //                                                   <<"  pout[x]="<<pout[x]<<std::endl;
      }
    }
    //std::cout<<std::endl;
    return;
    }

    float** pin = new float*[boxsz];

    //std::cout<<"offsx="<<offsx<<"  offsy="<<offsy<<"  rw="<<rw<<"  rh="<<rh<<"  radius="<<radius<<"  boxsz="<<boxsz<<std::endl;
    //std::cout<<"r->left="<<r->left<<"  r->top="<<r->top<<"  r->width="<<r->width<<"  r->height="<<r->height<<std::endl;

    std::vector<float> buf( boxsz*boxsz );

    for( y = 0; y < height; y++ ) {
      for( ri = 0; ri < boxsz; ri++ ) {
        //pin[ri] = (float*)VIPS_REGION_ADDR( ireg[0], r->left - radius, r->top + y - radius + ri );
        pin[ri] = (float*)VIPS_REGION_ADDR( ireg[0], offsx, offsy + y + ri );
      }
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      if( ireg[0]->im->Bands == 1 ) {
        for( x = 0; x < line_size; x+=1 ) {

          float* bufp = buf.data();
          int npx = 0;
          for( ri = 0; ri < boxsz; ri++ ) {
            float* prin = pin[ri];
            for( ci = 0; ci < boxsz; ci++, prin += 1 ) {
              if( fabs(prin[0]-pin2[x]) > opar->get_threshold() ) continue;
              *bufp = prin[0];
              //std::cout<<"buf["<<npx<<"]="<<*bufp<<std::endl;
              npx +=1; bufp+=1;
            }
          }
          //std::cout<<"y="<<y<<"  x="<<x<<"  pin2[x]="<<pin2[x]<<std::endl;
          //std::cout<<"npx(1)="<<npx<<"  boxsz*boxsz="<<boxsz*boxsz<<std::endl;
          if( npx > 0 && (npx%2) == 0 ) npx -= 1;
          //std::cout<<"npx(2)="<<npx<<std::endl;

          float median;
          if( npx > 2 ) {
          //buf.resize(npx);

          std::vector<float>::iterator first = buf.begin();
          std::vector<float>::iterator last = first + npx; //buf.end();
          std::vector<float>::iterator middle = first + (last - first) / 2;
          std::nth_element(first, middle, last); // can specify comparator as optional 4th arg
          median = *middle;
          //float L = pin2[x];
          //if( fabs(L-median) > opar->get_threshold() ) median = L;

          } else {
            median = pin2[x];
          }
          //std::cout<<"y="<<y<<"  x="<<x<<"  median="<<median<<std::endl;

          pout[x] = median;

          for( ri = 0; ri < boxsz; ri++ ) {
            pin[ri] += 1;
          }
        }
      } else if( ireg[0]->im->Bands == 3 ) {
        for( x = 0; x < line_size; x+=3 ) {

          float* bufp = buf.data();
          for( ri = 0; ri < boxsz; ri++ ) {
            float* prin = pin[ri];
            for( ci = 0; ci < boxsz; ci++, prin += 3, bufp+=1 ) {
              *bufp = profile->get_lightness(prin[0],prin[1],prin[2]);
            }
          }

          std::vector<float>::iterator first = buf.begin();
          std::vector<float>::iterator last = buf.end();
          std::vector<float>::iterator middle = first + (last - first) / 2;
          std::nth_element(first, middle, last); // can specify comparator as optional 4th arg
          float median = *middle;
          float L = profile->get_lightness(pin2[x],pin2[x+1],pin2[x+2]);
          if( fabs(L-median) > opar->get_threshold() ) median = L;

          pout[x] = pout[x+1] = pout[x+2] = median;

          for( ri = 0; ri < boxsz; ri++ ) {
            pin[ri] += 3;
          }
        }
      }
    }
    delete[] pin;
  }
};


ProcessorBase* new_median_filter();

}

#endif 


