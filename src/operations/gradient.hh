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

#ifndef VIPS_GRADIENT_H
#define VIPS_GRADIENT_H

#include <math.h>

#include <iostream>

#include "../base/format_info.hh"
#include "../base/processor.hh"
#include "../base/splinecurve.hh"

#include "curves.hh"

namespace PF 
{

enum gradient_type_t {
  GRADIENT_VERTICAL,
  GRADIENT_HORIZONTAL,
  GRADIENT_RADIAL
};

class GradientPar: public OpParBase
{
  PropertyBase gradient_type;
  Property<bool> invert;
  Property<float> gradient_center_x;
  Property<float> gradient_center_y;

  Property<SplineCurve> grey_curve;
  Property<SplineCurve> RGB_curve;
  Property<SplineCurve> R_curve;
  Property<SplineCurve> G_curve;
  Property<SplineCurve> B_curve;
  Property<SplineCurve> L_curve;
  Property<SplineCurve> a_curve;
  Property<SplineCurve> b_curve;
  Property<SplineCurve> C_curve;
  Property<SplineCurve> M_curve;
  Property<SplineCurve> Y_curve;
  Property<SplineCurve> K_curve;
  PropertyBase RGB_active_curve;
  PropertyBase Lab_active_curve;
  PropertyBase CMYK_active_curve;

  Property<SplineCurve> hmod, vmod;

  ProcessorBase* curve;

public:
  float* modvec;

  GradientPar();

  gradient_type_t get_gradient_type() {
    gradient_type_t result = gradient_type_t(gradient_type.get_enum_value().first);
    return( result );
  }
  bool get_invert() { return invert.get(); }
  float get_gradient_center_x() { return gradient_center_x.get(); }
  float get_gradient_center_y() { return gradient_center_y.get(); }

  SplineCurve& get_hmod() { return hmod.get(); }
  SplineCurve& get_vmod() { return vmod.get(); }

  bool needs_input() { return false; }
  bool has_intensity() { return false; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class Gradient: public IntensityProc<T, has_imap>
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, GradientPar* par);
};


template< OP_TEMPLATE_DEF >
void Gradient< OP_TEMPLATE_IMP >::
render(VipsRegion** ir, int n, int in_first,
    VipsRegion* imap, VipsRegion* omap,
    VipsRegion* oreg, GradientPar* par)
    {
  //BLENDER blender( par->get_blend_mode(), par->get_opacity() );

  Rect *r = &oreg->valid;
  int bands = oreg->im->Bands;
  int line_size = r->width * bands; //layer->in_all[0]->Bands;

  T* pout;
  int x, y, c, pos;

  int width = oreg->im->Xsize;// - oreg->im->Xoffset;
  int height = oreg->im->Ysize;// - oreg->im->Yoffset;

  //std::cout<<"Gradient::render: height="<<height<<"  offset="<<oreg->im->Yoffset<<std::endl;

  switch( par->get_gradient_type() ) {
  case GRADIENT_VERTICAL:
  {
    for( y = 0; y < r->height; y++ ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      float fval = static_cast<float>(height - r->top - y)/height;
      float fval2;
      //std::cout<<"y="<<y+r->top<<" ("<<y<<")"<<std::endl;
      for( x = 0, pos = 0; x < r->width; x++, pos+=bands) {
        fval2 = MIN( MAX(fval+par->modvec[r->left+x]-0.5f,0), 1 );
        if( par->get_invert() == true ) fval2 = 1.0f - fval2;
        T val = static_cast<T>( fval2*FormatInfo<T>::RANGE + FormatInfo<T>::MIN );
        for( c = 0; c < bands; c++ )
          pout[pos+c] = val;
      }
    }
    break;
  }
  case GRADIENT_HORIZONTAL:
  {
    float* valvec = new float[line_size];
    T val;
    if( valvec == NULL )
      break;
    int px, b;
    float fval, fval2;
    if( par->get_invert() == true ) {
      for( x = 0, px = 0; x < r->width; ++x) {
        fval = ((float)width - r->left - x - 1)/width;
        valvec[x] = fval;
        //val = (T)((float)FormatInfo<T>::RANGE*((float)width - r->left - x - 1)/width + FormatInfo<T>::MIN);
        //for( b = 0; b < bands; ++b, ++px) {
        //valvec[px] = val;
        //valvec[px] = fval;
        //}
      }
    } else {
      for( x = 0, px = 0; x < r->width; ++x) {
        fval = ((float)r->left + x)/width;
        valvec[x] = fval;
        //val = (T)((float)FormatInfo<T>::RANGE*((float)r->left + x)/width + FormatInfo<T>::MIN);
        //for( b = 0; b < bands; ++b, ++px) {
        //valvec[px] = val;
        //}
      }
    }
    for( y = 0; y < r->height; y++ ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      for( x = 0, pos = 0; x < r->width; ++x, pos+=bands ) {
        fval = MIN( MAX(valvec[x]+par->modvec[r->top+y]-0.5f,0), 1 );
        val = (T)(fval*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
        if(false && y==0) std::cout<<"  x="<<r->left+x<<" ("<<x<<")  fval="<<fval<<"  val="<<val<<std::endl;
        for( c = 0; c < bands; c++ )
          pout[pos+c] = val;
      }
    }

    delete[] valvec;
    break;
  }
  case GRADIENT_RADIAL:
  {
    int px, b;
    int width2 = width/2;
    int height2 = height/2;
    int center_x = (int)( par->get_gradient_center_x()*width );
    int center_y = (int)( par->get_gradient_center_y()*height );
    int dx1 = center_x;
    int dy1 = center_y;
    int dx2 = width - center_x;
    int dy2 = height - center_y;
    float diag1 = sqrtf( dx1*dx1 + dy1*dy1 );
    float diag2 = sqrtf( dx1*dx1 + dy2*dy2 );
    float diag3 = sqrtf( dx2*dx2 + dy1*dy1 );
    float diag4 = sqrtf( dx2*dx2 + dy2*dy2 );
    float diag = diag1;
    if( diag2 > diag ) diag = diag2;
    if( diag3 > diag ) diag = diag3;
    if( diag4 > diag ) diag = diag4;
    //float diag = sqrtf( width2*width2 + height2*height2);
    T val;

    for( y = 0; y < r->height; y++ ) {
      int dy = r->top + y - center_y;
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
      for( x = 0, px = 0; x < r->width; ++x) {
        int dx = r->left + x - center_x;
        float R = sqrtf( dx*dx + dy*dy );
        if( par->get_invert() == true ) {
          val = (T)((float)FormatInfo<T>::RANGE*(1.0f-R/diag) + FormatInfo<T>::MIN);
        } else {
          val = (T)((float)FormatInfo<T>::RANGE*(R/diag) + FormatInfo<T>::MIN);
        }
        for( b = 0; b < bands; ++b, ++px) {
          pout[px] = val;
        }
      }
    }
    break;
  }
  }

  //VipsRegion* ireg = ir ? ir[0] : NULL;
  //blender.blend( ireg, oreg, oreg, omap );
    };




ProcessorBase* new_gradient();
}

#endif 


