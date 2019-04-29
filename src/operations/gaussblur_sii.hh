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

#ifndef GAUSS_BLUR_SII_H
#define GAUSS_BLUR_SII_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"

#include "gaussian_conv_sii.hh"


namespace PF 
{

  class GaussBlurSiiPar: public OpParBase
  {
    Property<float> radius;

		sii_coeffs coeffs;
  public:
    GaussBlurSiiPar();

    void set_radius( float r ) { radius.set( r ); sii_precomp( &coeffs, radius.get(), 3 ); }

		sii_coeffs& get_coeffs() { return coeffs; }

		int get_padding() { return coeffs.radii[0] + 1; }

    /* Function to derive the output area from the input area
     */
    virtual void transform(const VipsRect* rin, VipsRect* rout, int /*id*/)
    {
			int pad = get_padding();
      rout->left = rin->left+pad;
      rout->top = rin->top+pad;
      rout->width = rin->width-pad*2;
      rout->height = rin->height-pad*2;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const VipsRect* rout, VipsRect* rin, int /*id*/)
    {
			int pad = get_padding();
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
  class GaussBlurSiiProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
			
    }
  };


  template < OP_TEMPLATE_DEF_PREVIEW_SPEC > 
  class GaussBlurSiiProc<OP_TEMPLATE_IMP_PREVIEW_SPEC(true)>
  {
		void sii_gaussian_conv_h(sii_coeffs& c, T *dest, double *buffer, const T *src, 
														 long start, long N, long width, int NCH);
		void sii_gaussian_conv_v(sii_coeffs& c, T *dest, double *buffer, const T *src, 
														 long start, long N, long heigth, long src_stride, long dest_stride, int NCH);

  public: 
    void render(VipsRegion** ireg, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* oreg, OpParBase* par)
    {
			GaussBlurSiiPar* gpar = dynamic_cast<GaussBlurSiiPar*>(par);
			if( !gpar ) return;
			VipsRect *ir = &(ireg[0]->valid);
			VipsRect *r = &oreg->valid;
			//int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
			//int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      //const int NCH = PF::ColorspaceInfo<CS>::NCH;
      const int NCH = oreg->im->Bands;
			const int out_stride = VIPS_REGION_LSKIP( oreg )/sizeof(T)/oreg->im->Bands;

			T* p;
			T* pout;

			// Temporary buffer to hold the row blur step
			long buf_width = r->width;
			long buf_width2 = buf_width*NCH;
			long buf_height = ir->height;
			//long buf_stride = buf_width;
			T* row_buf = (T*)malloc(sizeof(T)*NCH*buf_width*buf_height);
			T* row_buf_y = row_buf;
			T* row_buf_x = row_buf;

			long tmp_buf_size = sii_buffer_size( gpar->get_coeffs(), 
																					 ( (r->width>ir->height) ? r->width : ir->height ) );
			double* tmp_buf = (double*)malloc(sizeof(double)*NCH*tmp_buf_size);

			//std::cout<<"tmp_buf_size: "<<NCH*tmp_buf_size<<std::endl;

			int x, y;
			//int ch, dx=CHMAX-CHMIN+1, CHMAXplus1=CHMAX+1;
			//int ximap, ni;
			int pad = gpar->get_coeffs().radii[0] + 1; 
    
			for( y = 0; y < ir->height; y++ ) {
      
				p = (T*)VIPS_REGION_ADDR( ireg[0], r->left, ir->top + y ); 
				pout = (T*)VIPS_REGION_ADDR( oreg, r->left, ir->top + y ); 
				row_buf_y = &(row_buf[buf_width2*y]);

				sii_gaussian_conv_h(gpar->get_coeffs(), row_buf_y, tmp_buf, p, r->left, r->width, oreg->im->Xsize, NCH);
				//sii_gaussian_conv_h(gpar->get_coeffs(), pout, tmp_buf, p, r->left, r->width, oreg->im->Xsize);
				//row_buf_y += NCH*buf_width;
			}			
    
			int voffs = r->top - ir->top;
			for( x = 0; x < r->width; x++ ) {
      
				row_buf_x = &(row_buf[buf_width2*voffs+NCH*x]);
				//p = (T*)VIPS_REGION_ADDR( ireg[0], r->left+x, r->top ); 
				pout = (T*)VIPS_REGION_ADDR( oreg, r->left+x, r->top ); 

				sii_gaussian_conv_v(gpar->get_coeffs(), pout, tmp_buf, row_buf_x, r->top, r->height, oreg->im->Ysize, buf_width, out_stride, NCH );
			}	

			free( row_buf );
			free( tmp_buf );

			//std::cout<<"gaussblur_sii finished"<<std::endl;
			//usleep(1000000);
    }
  };


#include "gaussian_conv_sii_imp.hh"


  ProcessorBase* new_gaussblur_sii();
}

#endif 


