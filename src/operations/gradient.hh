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
#include "../base/operation_ptp.hh"

namespace PF 
{

  class GradientPar: public OpParBase
  {
  public:
    GradientPar(): OpParBase() {}
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
    //BLENDER blender;
    
    double intensity = par->get_intensity();
    
    Rect *r = &oreg->valid;
    int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 


      /*
    std::cout<<"OperationPTP::render(): "<<std::endl
	     <<"  input region:  top="<<ir[in_first]->valid.top
	     <<" left="<<ir[in_first]->valid.left
	     <<" width="<<ir[in_first]->valid.width
	     <<" height="<<ir[in_first]->valid.height<<std::endl
	     <<"  output region: top="<<oreg->valid.top
	     <<" left="<<oreg->valid.left
	     <<" width="<<oreg->valid.width
	     <<" height="<<oreg->valid.height<<std::endl;
      */

    T* p;    
    T* pout;
    T* pimap;
    T* pomap;

    int height = oreg->im->Ysize - oreg->im->Yoffset;

    //std::cout<<"Gradient::render: height="<<height<<std::endl;
    
    for( int y = 0; y < r->height; y++ ) {
      
      p = ir ? (T*)VIPS_REGION_ADDR( ir[0], r->left, r->top + y ) : NULL; 
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
      //if(has_imap) pimap = (T*)IM_REGION_ADDR(imap,y,le);
      //if(has_omap) pomap = (T*)IM_REGION_ADDR(omap,y,le);

      T val = (T)((float)FormatInfo<T>::RANGE*((float)height - r->top - y)/height + FormatInfo<T>::MIN);

      //std::cout<<"  y="<<r->top+y<<" ("<<y<<")  val="<<(int)val<<std::endl;
      
      for( int x = 0; x < line_size; ++x) {
	pout[x] = val;
      }
    }
  };


}

#endif 


