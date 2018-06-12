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

#ifndef IGV_DEMOSAIC_H
#define IGV_DEMOSAIC_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../rt/rtengine/rawimagesource.hh"



namespace PF
{

  class IgvDemosaicPar: public OpParBase
  {
  public:
    IgvDemosaicPar();
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      rgb_image( get_xsize(), get_ysize() );
    }

    /* Function to derive the output area from the input area
     */
    virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
    {
      rout->left = rin->left+16;
      rout->top = rin->top+16;
      rout->width = rin->width-32;
      rout->height = rin->height-32;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
    {
			// Output region aligned to the Bayer pattern
			int raw_left = (rout->left/2)*2;
			int raw_top = (rout->top/2)*2;
			int raw_right = rout->left+rout->width-1;
			int raw_bottom = rout->top+rout->height-1;
			int raw_width = raw_right-raw_left+1;
			int raw_height = raw_bottom-raw_top+1;

      rin->left = raw_left-16;
      rin->top = raw_top-16;
      rin->width = raw_width+32;
      rin->height = raw_height+32;
    }
      


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  


  template < OP_TEMPLATE_DEF > 
  class IgvDemosaicProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, IgvDemosaicPar* par) 
    {
      //fast_demosaic( in, n, in_first,
      //	     imap, omap, out, par );
    }
  };


  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class IgvDemosaicProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) > 
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, IgvDemosaicPar* par) 
    {
			rtengine::RawImageSource rawimg;
			rawimg.igv_demosaic( in[0], out );
    }
  };


  ProcessorBase* new_igv_demosaic();
}


#endif
