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

#ifndef CA_CORRECT_H
#define CA_CORRECT_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/operation.hh"
#include "../rt/rtengine/rawimagesource.hh"



namespace PF
{

  class CACorrectPar: public OpParBase
  {
    Property<bool> enable_ca;
    Property<bool> auto_ca;

    // Manual CA correction parameters
    Property<float> ca_red, ca_blue;

    dcraw_data_t* image_data;
  public:
    CACorrectPar();
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    dcraw_data_t* get_image_data() { return image_data; }

    void set_enable_ca(bool flag) { enable_ca.update(flag); }
    bool get_auto_ca() { return auto_ca.get(); }
    void set_auto_ca(bool flag) { auto_ca.update(flag); }
    float get_ca_red() { return ca_red.get(); }
    float get_ca_blue() { return ca_blue.get(); }

    /* Function to derive the output area from the input area
     */
    virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
    {
      rout->left = rin->left+8;
      rout->top = rin->top+8;
      rout->width = rin->width-16;
      rout->height = rin->height-16;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
    {
      int border = 8;
			// Output region aligned to the Bayer pattern
			int raw_left = (rout->left/2)*2;
			//if( raw_left < border ) raw_left = 0;

			int raw_top = (rout->top/2)*2;
      //if( raw_top < border ) raw_top = 0;

      int raw_right = rout->left+rout->width-1;
			//if( raw_right > (in->Xsize-border-1) ) raw_right = in->Xsize-1;

			int raw_bottom = rout->top+rout->height-1;
      //if( raw_bottom > (in->Ysize-border-1) ) raw_bottom = in->Ysize-1;

			int raw_width = raw_right-raw_left+1;
			int raw_height = raw_bottom-raw_top+1;

      rin->left = raw_left-border;
      rin->top = raw_top-border;
      rin->width = raw_width+border*2;
      rin->height = raw_height+border*2;

      if( (rin->width%2) ) rin->width += 1;
      if( (rin->height%2) ) rin->height += 1;
    }
      


    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, 
										 unsigned int& level);
  };

  


  template < OP_TEMPLATE_DEF > 
  class CACorrectProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, CACorrectPar* par)
    {
      //fast_demosaic( in, n, in_first,
      //	     imap, omap, out, par );
    }
  };


  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class CACorrectProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, CACorrectPar* par)
    {
			rtengine::RawImageSource rawimg;
			rawimg.set_image_data( par->get_image_data() );
			rawimg.ca_correct( in[0], out, par->get_auto_ca(), par->get_ca_red(), par->get_ca_blue() );
    }
  };


  ProcessorBase* new_ca_correct();
}


#endif
