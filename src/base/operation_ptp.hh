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
#include "layer.hh"

namespace PF
{

#define PIXELPROC_TEMPLATE_DEF                  \
  typename T, colorspace_t CS,                  \
    int CHMIN, int CHMAX,                       \
    bool PREVIEW, typename OP_PAR

#define PIXELPROC_TEMPLATE_IMP                  \
  T, CS, CHMIN, CHMAX, PREVIEW, OP_PAR



  template < OP_TEMPLATE_DEF, class OP_PAR, template < PIXELPROC_TEMPLATE_DEF > class PEL_PROC > 
  class OperationPTP: public IntensityProc<T, has_imap>
  {
    typedef OP_PAR OpParams;
  public: 
    void render(VipsRegion** in, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* out, OpParams* par);
  };


  template< OP_TEMPLATE_DEF, class OP_PAR, template < PIXELPROC_TEMPLATE_DEF > class PEL_PROC >
  void OperationPTP< OP_TEMPLATE_IMP, OP_PAR, PEL_PROC >::
  render(VipsRegion** ir, int n, int in_first,
         VipsRegion* imap, VipsRegion* omap, 
         VipsRegion* oreg, OP_PAR* par)
  {
    PEL_PROC<PIXELPROC_TEMPLATE_IMP> proc(par);

    float intensity = par->get_intensity();
    //float opacity = par->get_opacity();
    
    //BLENDER blender( par->get_blend_mode(), opacity );
    
    Rect *r = &oreg->valid;
    int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

    //#ifndef NDEBUG
#ifdef _NDEBUG
    std::cout<<"OperationPTP::render(): "<<std::endl
             <<"  name: "<<par->get_config_ui()->get_layer()->get_name()<<std::endl
             <<"  input region:  top="<<ir[in_first]->valid.top
             <<" left="<<ir[in_first]->valid.left
             <<" width="<<ir[in_first]->valid.width
             <<" height="<<ir[in_first]->valid.height<<std::endl
             <<"  output region: top="<<oreg->valid.top
             <<" left="<<oreg->valid.left
             <<" width="<<oreg->valid.width
             <<" height="<<oreg->valid.height<<std::endl;
#endif    
    
    const int NMAX = 100;
    T* p[NMAX+1];
    T* pout;
    T* pimap;
    //T* pomap;

    //return;
    
    if(n > NMAX) n = NMAX;
    
    //std::cout<<"sz: "<<sz<<std::endl;
    int x, y, ch, dx=CHMAX-CHMIN+1, CHMAXplus1=CHMAX+1;
    int ximap, ni;
    
    for( y = 0; y < r->height; y++ ) {
      
      for( ni = 0; ni < n; ni++) 
        p[ni] = (T*)VIPS_REGION_ADDR( ir[ni], r->left, r->top + y ); 
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
      if(has_imap) pimap = (T*)VIPS_REGION_ADDR( imap, r->left, r->top + y );
      
      //std::cout<<"  y="<<r->top+y<<" ("<<y<<")  intensity="<<intensity/*<<"  real="<<intensity_real*/<<std::endl;

      for( x=0, ximap=0; x < line_size; ) {

        //continue;
        float intensity_real = this->get_intensity( intensity, pimap, ximap );
        for( ch=0; ch<CHMIN; ch++, x++ ) pout[x] = p[0][x];
        proc.process( p, n, in_first, sz, x, intensity_real/*get_intensity( intensity, pimap, ximap )*/, pout );
        x += dx;
        for( ch=CHMAXplus1; ch<PF::ColorspaceInfo<CS>::NCH; ch++, x++ ) pout[x] = p[0][x];
        //x += sz;
        //for( int ni = 0; ni < n; ni++) 
        //  p[ni] += sz;
        //pout += sz;
	
	
      }
    }

    //VipsRegion* ireg = ir ? ir[0] : NULL;
    //blender.blend( ireg, oreg, oreg, omap );
  };

}

#endif
