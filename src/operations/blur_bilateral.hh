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

#ifndef BLUR_BILATERAL_H
#define BLUR_BILATERAL_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"
extern "C" {
#include "../external/darktable/src/common/bilateral.h"
}


namespace PF 
{

#include "blur_bilateral_splat.hh"
#include "blur_bilateral_blur.hh"
#include "blur_bilateral_slice.hh"


class BlurBilateralAlgoPar: public OpParBase
{
  Property<float> sigma_s;
  Property<float> sigma_r;
  float ss, sr;

  int cur_padding;

public:
  BlurBilateralAlgoPar();

  //void set_iterations( int i ) { iterations.set( i ); }
  void set_sigma_s( float s ) { sigma_s.set( s ); }
  void set_sigma_r( float s ) { sigma_r.set( s ); }

  float get_ss() { return ss; }
  float get_sr() { return sr; }

  float get_sigma_s() { return sigma_s.get(); }
  float get_sigma_r() { return sigma_r.get(); }

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_caching() { return true; }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
  {
    float ss = sigma_s.get();
    for( int l = 1; l <= level; l++ ) {
      ss /= 2;
    }
    int iss = static_cast<int>(ss*3);
    //iss /= 16;
    //iss = (iss+1)*16;
    iss = 2;
    set_padding( iss, id );
    std::cout<<"BlurBilateralAlgoPar()::compute_padding(): sigma_s="<<sigma_s.get()
        <<"  level="<<level<<"  ss="<<ss<<"  iss="<<iss<<"  padding="<<get_padding(id)<<std::endl;
  }

  /* Function to derive the output area from the input area
   */
  virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
  {
    int pad = get_padding(0);
    rout->left = rin->left+pad;
    rout->top = rin->top+pad;
    rout->width = rin->width-pad*2;
    rout->height = rin->height-pad*2;
  }

  /* Function to derive the area to be read from input images,
     based on the requested output area
  */
  virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
  {
    int pad = get_padding(0);
    rin->left = rout->left-pad;
    rin->top = rout->top-pad;
    rin->width = rout->width+pad*2;
    rin->height = rout->height+pad*2;
  }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class BlurBilateralAlgoProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    std::cout<<"BlurBilateralAlgoProc::render() called"<<std::endl;
  }
};

template < OP_TEMPLATE_DEF_CS_SPEC >
class BlurBilateralAlgoProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_GRAYSCALE) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) return;

    BlurBilateralAlgoPar* opar = dynamic_cast<BlurBilateralAlgoPar*>(par);
    if( !opar ) return;

    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int y;
    VipsImage* srcimg = ireg[0]->im;

    if( false && r->left<10000 && r->top<10000 ) {
      std::cout<<"BlurBilateralAlgoProc::render(): ireg="<<ireg[0]->valid.width<<"x"<<ireg[0]->valid.height
          <<"+"<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<std::endl;
      std::cout<<"                                 oreg="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;
    }

    T* obuf = (T*)malloc( ireg[0]->valid.width * ireg[0]->valid.height * sizeof(T) );
    if(!obuf) return;

    T* pin = (T*)VIPS_REGION_ADDR( ireg[0], ireg[0]->valid.left, ireg[0]->valid.top );
    T* pout;
    T* p;

    int verb = 0;
    //if(ireg[0]->valid.left==0 && ireg[0]->valid.top==0) verb = 1;
    dt_bilateral_t* dt_b = dt_bilateral_init(ireg[0]->valid.width, ireg[0]->valid.height,
        opar->get_ss(), opar->get_sr(), verb);
    dt_b->buf = (float*)malloc(dt_b->size_x * dt_b->size_y * dt_b->size_z * sizeof(float));
    memset(dt_b->buf, 0, dt_b->size_x * dt_b->size_y * dt_b->size_z * sizeof(float));

    int ilskip = VIPS_REGION_LSKIP( ireg[0] ) / sizeof(T);
    int lskip = dt_b->size_x * dt_b->size_z;
    int olskip = VIPS_REGION_LSKIP( oreg ) / sizeof(T);

    dt_bilateral_splat(dt_b, pin, ilskip, lskip, verb);
    dt_bilateral_blur(dt_b);
    dt_bilateral_slice(dt_b, pin, obuf, ilskip, lskip, olskip, -1);
    dt_bilateral_free(dt_b);
    int dx = r->left - ireg[0]->valid.left;
    int dy = r->top - ireg[0]->valid.top;

    for( y = 0; y < height; y++ ) {
      p = obuf + (y+dy)*ireg[0]->valid.width + dx;
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      memcpy(pout, p, sizeof(T)*width );
    }
    free(dt_b->buf);
    free(obuf);
  }
};





class BlurBilateralPar: public OpParBase
{
  Property<float> sigma_s;
  Property<float> sigma_r;
  ProcessorBase* bsplat;
  ProcessorBase* blur;
  ProcessorBase* slice;
  ProcessorBase* balgo;

public:
  BlurBilateralPar();

  bool has_intensity() { return false; }
  bool has_opacity() { return true; }
  bool needs_caching() { return false; }

  void set_sigma_s( float s );
  void set_sigma_r( float s );
  float get_sigma_s() { return sigma_s.get(); }
  float get_sigma_r() { return sigma_r.get(); }

  void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
  {
    //g_assert(balgo->get_par() != NULL);
    //balgo->get_par()->compute_padding(full_res, id, level);
    //set_padding( balgo->get_par()->get_padding(id), id );
    float ss;
    ss = sigma_s.get();
    for( int l = 1; l <= level; l++ ) {
      ss /= 2;
    }
    ss = roundf(ss); //ss = 100;
    int iss = ss;
    if(iss < 1) iss = 1;

    set_padding( 5*iss, id );
    std::cout<<"BlurBilateralPar()::compute_padding(): padding="<<get_padding(id)<<std::endl;
  }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);

  VipsImage* build_(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class BlurBilateralProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};



template < OP_TEMPLATE_DEF_CS_SPEC >
class BlurBilateralProc< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_GRAYSCALE) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    if( ireg[0] == NULL ) return;

    BlurBilateralPar* opar = dynamic_cast<BlurBilateralPar*>(par);
    if( !opar ) return;

    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int y;
    VipsImage* srcimg = ireg[0]->im;

    if( false && r->left<10000 && r->top<10000 ) {
      std::cout<<"BlurBilateralProc::render(): ireg="<<ireg[0]->valid.width<<"x"<<ireg[0]->valid.height
          <<"+"<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<std::endl;
      std::cout<<"                                 oreg="<<r->width<<"x"<<r->height<<"+"<<r->left<<","<<r->top<<std::endl;
    }

    T* pin;
    T* pout;
    for( y = 0; y < height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      memcpy(pout, pin, sizeof(T)*width );
    }
  }
};


ProcessorBase* new_blur_bilateral();
}

#endif 


