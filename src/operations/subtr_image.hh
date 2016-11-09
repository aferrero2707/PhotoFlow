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

/* The Wavelet Decompose algorithm is based on the GIMP's Wavelet Decompose plugin, by Marco Rossini 
 * 
 * http://registry.gimp.org/node/11742
 * 
 * */


#ifndef PF_SUBTRIMG_H
#define PF_SUBTRIMG_H

#include <string.h>

namespace PF 
{


class SubtrImgPar: public OpParBase
{
  Property<float> blendFactor;
  
  ProcessorBase* subtrimg_algo;

public:
  SubtrImgPar();

  bool has_intensity() { return false; }
//  bool needs_caching() { return true; }

  float get_blendFactor() { return (float)blendFactor.get(); }
  float set_blendFactor(float a) { blendFactor.set(a); }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};

class SubtrImgAlgoPar: public OpParBase
{
  float blendFactor;

public:
  SubtrImgAlgoPar(): OpParBase()
  {
      blendFactor = .5f;
  }
  
  float get_blendFactor() { return (float)blendFactor; }
  void set_blendFactor(float a) { blendFactor=a; }


};



template < OP_TEMPLATE_DEF >
class SubtrImgProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    std::cout<<"SubtrImgAlgoProc::render() 1"<<std::endl;
  }
};

template < OP_TEMPLATE_DEF >
class SubtrImgAlgoProc
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, OpParBase* par)
  {
    std::cout<<"SubtrImgAlgoProc::render() 2"<<std::endl;
  }
};



template < OP_TEMPLATE_DEF_TYPE_SPEC >
class SubtrImgAlgoProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
    
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
//    std::cout<<"SubtrImgAlgoProc::render()"<<std::endl;
    
    if( n < 2 ) {
      std::cout<<"SubtrImgAlgoProc::render n < 2"<<std::endl;
      return;
    }

    if( ireg[0] == NULL ) {
      std::cout<<"SubtrImgAlgoProc::render ireg[0] == NULL"<<std::endl;
      return;
    }

    if( ireg[1] == NULL ) {
      std::cout<<"SubtrImgAlgoProc::render ireg[1] == NULL"<<std::endl;
      return;
    }

    SubtrImgAlgoPar* opar = dynamic_cast<SubtrImgAlgoPar*>(par);
    if( !opar ) {
      std::cout<<"SubtrImgAlgoProc::render opar == NULL"<<std::endl;
      return;
    }

    Rect *r = &oreg->valid;
    Rect *ir0 = &ireg[0]->valid;
    Rect *ir1 = &ireg[1]->valid;
    
//    const int width = r->width;
//    const int height = r->height;
    const int width = std::min(r->width, std::min(ir0->width, ir1->width));
    const int height = std::min(r->height, std::min(ir0->height, ir1->height));
    const int ch = oreg->im->Bands;
    
    const int line_size = width * ch;
    const float blendfactor = opar->get_blendFactor();
    
//    std::cout<<"SubtrImgAlgoProc::render ch: "<<ch<<std::endl;
//    std::cout<<"SubtrImgAlgoProc::render blendfactor: "<<blendfactor<<std::endl;
//    std::cout<<"SubtrImgAlgoProc::render r->width: "<<r->width<<", r->height: "<<r->height<<", r->top: "<<r->top<<", r->left: "<<r->left<<std::endl;
//    std::cout<<"SubtrImgAlgoProc::render ir0->width: "<<ir0->width<<", ir0->height: "<<ir0->height<<", ir0->top: "<<ir0->top<<", ir0->left: "<<ir0->left<<std::endl;
//    std::cout<<"SubtrImgAlgoProc::render ir1->width: "<<ir1->width<<", ir1->height: "<<ir1->height<<", ir1->top: "<<ir1->top<<", ir1->left: "<<ir1->left<<std::endl;
    
    for( int y = 0; y < height; y++ ) {
      float *pin0 = (float*)VIPS_REGION_ADDR( ireg[0], ir0->left, ir0->top + y );
      float *pin1 = (float*)VIPS_REGION_ADDR( ireg[1], ir1->left, ir1->top + y );
      float *pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for (int x = 0; x < line_size; x++) {
        pout[x] = (pin0[x] - pin1[x]) + blendfactor;
      }
    }
        
  }
  

};


ProcessorBase* new_subtrimg();

}

#endif 

