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

#ifndef DISKBUFFER_OP_H
#define DISKBUFFER_OP_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation_ptp.hh"
#include "../base/splinecurve.hh"

namespace PF 
{

  class DiskBufferPar: public OpParBase
  {
    int descriptor;
    int width;
    int height;

  public:
    DiskBufferPar();

		void set_descriptor(int d) { descriptor = d; }
		void set_width(int w) { width = w; }
		void set_height(int h) { height = h; }

		int get_descriptor() { return descriptor; }
		int get_width() { return width; }
		int get_height() { return height; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class DiskBufferProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* out, DiskBufferPar* par);
  };


  template< OP_TEMPLATE_DEF >
  void DiskBufferProc< OP_TEMPLATE_IMP >::
  render(VipsRegion** ir, int n, int in_first,
	 VipsRegion* imap, VipsRegion* omap, 
	 VipsRegion* oreg, DiskBufferPar* par)
  {
    Rect *r = &oreg->valid;
    int width = r->width;
    int height = r->height;
    int line_size = width * oreg->im->Bands; //layer->in_all[0]->Bands; 
#ifndef NDEBUG
		std::cout<<"DiskBufferProc::render() called, height="<<height<<std::endl;
#endif
    T* pout;
    int y;

    for( y = 0; y < height; y++ ) {

      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
			memset( pout, 0, sizeof(T)*line_size );

			//if(y != height/2) continue;
			//if(y < height/2-5) continue;
			//if(y > height/2+5) continue;

			size_t skip = ((size_t)par->get_width()*(r->top + y)+r->left)*sizeof(T)*oreg->im->Bands;
			lseek( par->get_descriptor(), skip, SEEK_SET );
			ssize_t nread = read( par->get_descriptor(), pout, sizeof(T)*line_size );
#ifndef NDEBUG
			std::cout<<"DiskBufferProc::render(): fd="<<par->get_descriptor()
							 <<"  y="<<r->top + y<<"  nread="<<nread
							 <<"  sizeof(T)*line_size="<<sizeof(T)*line_size<<std::endl;
#endif
			if(nread != sizeof(T)*line_size) break;
#ifdef NDEBUG
      if( y < 20*oreg->im->Bands && r->top==0 && r->left == 0 ) {
#endif
				std::cout<<"DiskBufferProc::render()"<<std::endl;
				for( int i = 0; i < line_size; i+=1/*oreg->im->Bands*/ )
					std::cout<<(float)pout[i]<<" ";
				std::cout<<std::endl;
#ifdef NDEBUG
      }
#endif
    }
  };


  ProcessorBase* new_diskbuffer();
}

#endif 


