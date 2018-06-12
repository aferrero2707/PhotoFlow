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

#ifndef FALSE_COLOR_CORRECTION_H
#define FALSE_COLOR_CORRECTION_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"



namespace PF
{

  class FalseColorCorrectionPar: public OpParBase
  {
  public:
    FalseColorCorrectionPar();
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
      rout->left = rin->left+2;
      rout->top = rin->top+2;
      rout->width = rin->width-4;
      rout->height = rin->height-4;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
    {
      rin->left = rout->left-2;
      rin->top = rout->top-2;
      rin->width = rout->width+4;
      rin->height = rout->height+4;
    }
      


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  


  template < OP_TEMPLATE_DEF > 
  class FalseColorCorrectionProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, FalseColorCorrectionPar* par) 
    {
      //fast_demosaic( in, n, in_first,
      //	     imap, omap, out, par );
    }
  };



	void false_color_correction(VipsRegion* ir, VipsRegion* oreg);



  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class FalseColorCorrectionProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) > 
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, FalseColorCorrectionPar* par) 
    {
			false_color_correction( in[0], out );
    }
  };


  ProcessorBase* new_false_color_correction();
}


#endif
