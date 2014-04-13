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

#ifndef CONVERT_2_LAB_H
#define CONVERT_2_LAB_H

#include <iostream>

#include <lcms2.h>

#include "../base/format_info.hh"
#include "../base/operation.hh"

namespace PF 
{

  class Convert2LabPar: public OpParBase
  {
    cmsHPROFILE profile_in;
    cmsHPROFILE profile_out;
    cmsHTRANSFORM transform;

  public:
    Convert2LabPar();
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    cmsHTRANSFORM get_transform() { return transform; }

    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      lab_image( get_xsize(), get_ysize() );
    }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class Convert2LabProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* out, Convert2LabPar* par);
  };


  template< OP_TEMPLATE_DEF >
  void Convert2LabProc< OP_TEMPLATE_IMP >::
  render(VipsRegion** ir, int n, int in_first,
	 VipsRegion* imap, VipsRegion* omap, 
	 VipsRegion* oreg, Convert2LabPar* par)
  {
    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    //int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 
    cmsHTRANSFORM transform = par->get_transform();


    T* p;    
    T* pout;
    int y;

    for( y = 0; y < height; y++ ) {
      
      p = ir ? (T*)VIPS_REGION_ADDR( ir[0], r->left, r->top + y ) : NULL; 
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
      cmsDoTransform( transform, p, pout, width );
#ifndef NDEBUG
      if( y == 0 && r->top==0 && r->left == 0 ) {
	std::cout<<"Convert2LabProc::render()"<<std::endl;
	for( int i = 0; i < 12; i++ )
	  std::cout<<(int)p[i]<<" ";
	std::cout<<std::endl;
	for( int i = 0; i < 12; i++ )
	  std::cout<<(int)pout[i]<<" ";
	std::cout<<std::endl;
      }
#endif
    }
  };


  ProcessorBase* new_convert2lab();
}

#endif 


