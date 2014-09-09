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

#include <iostream>

#include "../base/format_info.hh"
#include "../base/processor.hh"

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
    Property<float> gradient_center_x;
    Property<float> gradient_center_y;

  public:
    GradientPar(): 
      OpParBase(),
      gradient_type("gradient_type",this,GRADIENT_VERTICAL,"vertical","Vertical"),
      gradient_center_x("gradient_center_x",this,0.5),
      gradient_center_y("gradient_center_y",this,0.5)
    {
      gradient_type.add_enum_value(GRADIENT_VERTICAL,"vertical","Vertical");
      gradient_type.add_enum_value(GRADIENT_HORIZONTAL,"horizontal","Horizontal");
      gradient_type.add_enum_value(GRADIENT_RADIAL,"radial","Radial");
      set_type( "gradient" );
    }

    gradient_type_t get_gradient_type() { 
      gradient_type_t result = gradient_type_t(gradient_type.get_enum_value().first);
      return( result ); 
    }

    bool needs_input() { return false; }
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
    int x, y;

    int width = oreg->im->Xsize - oreg->im->Xoffset;
    int height = oreg->im->Ysize - oreg->im->Yoffset;

    //std::cout<<"Gradient::render: height="<<height<<std::endl;
    
    switch( par->get_gradient_type() ) {
    case GRADIENT_VERTICAL: 
      {
	for( y = 0; y < r->height; y++ ) {      
	  pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	  T val = (T)((float)FormatInfo<T>::RANGE*((float)height - r->top - y)/height + FormatInfo<T>::MIN);
	  //std::cout<<"  y="<<r->top+y<<" ("<<y<<")  val="<<(int)val<<std::endl;
	  for( x = 0; x < line_size; ++x) {
	    pout[x] = val;
	  }
	}
	break;
      }
    case GRADIENT_HORIZONTAL: 
      {
	T* valvec = new T[line_size];
	T val;
	if( valvec == NULL )
	  break;
	int px, b;
	for( x = 0, px = 0; x < r->width; ++x) {
	  val = (T)((float)FormatInfo<T>::RANGE*((float)r->left + x)/width + FormatInfo<T>::MIN);
	  for( b = 0; b < bands; ++b, ++px) {
	    valvec[px] = val;
	  }
	}
	for( y = 0; y < r->height; y++ ) {      
	  pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
	  //std::cout<<"  y="<<r->top+y<<" ("<<y<<")  val="<<(int)val<<std::endl;
	  for( x = 0; x < line_size; ++x) {
	    pout[x] = valvec[x];
	  }
	}
	break;
      }
    case GRADIENT_RADIAL:
      {
	break;
      } 
    }

    //VipsRegion* ireg = ir ? ir[0] : NULL;
    //blender.blend( ireg, oreg, oreg, omap );
  };




  ProcessorBase* new_gradient();
}

#endif 


