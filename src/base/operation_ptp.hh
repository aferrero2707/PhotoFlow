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

#ifndef PF_OPERATION_PTP_H
#define PF_OPERATION_PTP_H

#include <iostream>

#include "processor.hh"

namespace PF
{


  template < OP_TEMPLATE_DEF, class PEL_PROC, class OP_PAR > 
  class OperationPTP: public IntensityProc<T, has_imap>
  {
    typedef OP_PAR OpParams;
  public: 
    void render(VipsRegion** in, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* out, OpParams* par);
  };


  template< OP_TEMPLATE_DEF, class PEL_PROC, class OP_PAR >
  void OperationPTP<OP_TEMPLATE_IMP, 
		    PEL_PROC/*<T,CS,PREVIEW,OP_PAR>*/, 
		    OP_PAR>::
  render(VipsRegion** ir, int n, int in_first,
	 VipsRegion* imap, VipsRegion* omap, 
	 VipsRegion* oreg, OpParams* par)
  {
    PEL_PROC proc(par);

    BLENDER blender;
    
    float intensity = par->get_intensity();
    
    Rect *r = &oreg->valid;
    int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

    //std::cout<<"OperationPTP::render(): "<<std::endl;
    /*
	     <<"  input region:  top="<<ir[in_first]->valid.top
	     <<" left="<<ir[in_first]->valid.left
	     <<" width="<<ir[in_first]->valid.width
	     <<" height="<<ir[in_first]->valid.height<<std::endl
	     <<"  output region: top="<<oreg->valid.top
	     <<" left="<<oreg->valid.left
	     <<" width="<<oreg->valid.width
	     <<" height="<<oreg->valid.height<<std::endl;
    */
    
    const int NMAX = 100;
    T* p[NMAX+1];
    T* pout;
    T* pimap;
    //T* pomap;

    //return;
    
    if(n > NMAX) n = NMAX;
    
    //std::cout<<"sz: "<<sz<<std::endl;
    int x, y; 
    int ximap, xomap, ni;
    
    for( y = 0; y < r->height; y++ ) {
      
      for( ni = 0; ni < n; ni++) 
	p[ni] = (T*)VIPS_REGION_ADDR( ir[ni], r->left, r->top + y ); 
      //p[ni] = (T*)IM_REGION_ADDR(ir[ni],y,le);
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
      //pout = (T*)IM_REGION_ADDR(oreg,y,le);
      if(has_imap) pimap = (T*)VIPS_REGION_ADDR( imap, r->left, r->top + y );
      //if(has_omap) pomap = (T*)VIPS_REGION_ADDR( omap, r->left, r->top + y );
      blender.init_line( omap, r->left, r->top + y );
      
      //std::cout<<"  y="<<r->top+y<<" ("<<y<<")  intensity="<<intensity/*<<"  real="<<intensity_real*/<<std::endl;

      for( x=0, ximap=0, xomap=0; x < line_size; ) {

	//continue;
	float intensity_real = get_intensity( intensity, pimap, ximap );
	proc.process( p, n, in_first, sz, x, intensity_real/*get_intensity( intensity, pimap, ximap )*/, pout );
	blender.blend( p[0], pout, x, xomap );
	//for( int ni = 0; ni < n; ni++) 
	//  p[ni] += sz;
	//pout += sz;
	
	
      }
    }
  };

}

#endif
