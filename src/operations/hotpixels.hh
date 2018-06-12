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

/*
 * The hotpixels algorithm is based on the Darktable "Hot Pixels" module
 * 
 * http://www.darktable.org/
 * 
 */

#ifndef VIPS_HOTPIXELS_H
#define VIPS_HOTPIXELS_H

#include <string>

#include "../base/processor.hh"
#include "../base/rawmatrix.hh"

#include "raw_image.hh"


namespace PF 
{

  class HotPixelsPar: public OpParBase
  {
    dcraw_data_t* image_data;

    Property<bool> hotp_enable;
    Property<float> hotp_threshold;
    Property<float> hotp_strength;
    Property<bool> hotp_permissive;
    Property<bool> hotp_markfixed;

    int pixels_fixed;

  public:
    HotPixelsPar();

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
			 creation of an intensity map is not allowed
       2. the operation can work without an input image;
			 the blending will be set in this case to "passthrough" and the image
			 data will be simply linked to the output
		*/
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return true; }

    dcraw_data_t* get_image_data() {return image_data; }

    float get_hotp_threshold() { return hotp_threshold.get(); }
    float get_hotp_strength() { return hotp_strength.get(); }
    bool get_hotp_permissive() { return hotp_permissive.get(); }
    bool get_hotp_markfixed() { return hotp_markfixed.get(); }

    int get_pixels_fixed() { return pixels_fixed; }
    void set_pixels_fixed(int p) { pixels_fixed = p; }

    /* Function to derive the output area from the input area
     */
    virtual void transform(const Rect* rin, Rect* rout, int /*id*/)
    {
      int border = 2;
      rout->left = rin->left+border;
      rout->top = rin->top+border;
      rout->width = rin->width-border*2;
      rout->height = rin->height-border*2;
    }
       
    /* Function to derive the area to be read from input images,
       based on the requested output area
    */
    virtual void transform_inv(const Rect* rout, Rect* rin, int /*id*/)
    {
      int border = 2;
      
      // Output region aligned to the Bayer pattern
      int raw_left = (rout->left/2)*2;
      int raw_top = (rout->top/2)*2;
      int raw_right = rout->left+rout->width-1;
      int raw_bottom = rout->top+rout->height-1;
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
										 VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  
  template < OP_TEMPLATE_DEF > 
  class HotPixels
  {
  public: 

    /* Detect hot sensor pixels based on the 4 surrounding sites. Pixels
     * having 3 or 4 (depending on permissive setting) surrounding pixels that
     * than value*multiplier are considered "hot", and are replaced by the maximum of
     * the neighbour pixels. The permissive variant allows for
     * correcting pairs of hot pixels in adjacent sites. Replacement using
     * the maximum produces fewer artifacts when inadvertently replacing
     * non-hot pixels. */

    void render(VipsRegion** ireg, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* oreg, OpParBase* par)
    {
      HotPixelsPar* rdpar = dynamic_cast<HotPixelsPar*>(par);
      if( !rdpar ) return;

      const float threshold = rdpar->get_hotp_threshold();
      const float multiplier = rdpar->get_hotp_strength() / 2.0f;
      Rect *r = &oreg->valid;
      Rect *ir = &ireg[in_first]->valid;
      int nbands = ireg[in_first]->im->Bands;
      const int owidth = r->width;
      const int oheight = r->height;
      const int iwidth = ir->width;
      const int iheight = ir->height;
      const int iwidthx2 = iwidth * 2;
      const bool markfixed = ( rdpar->get_hotp_markfixed() && ( rdpar->get_render_mode() == PF_RENDER_PREVIEW ) );
      const int min_neighbours = rdpar->get_hotp_permissive() ? 3 : 4;

      float *cin = NULL;
      float *cin1 = NULL;
      float *cin2 = NULL;
      float *cout = NULL;
      
      // The loop should output only a few pixels, so just copy everything first
      vips_region_copy (ireg[in_first], oreg, r, r->left, r->top);
      
      int fixed = rdpar->get_pixels_fixed();

    // Bayer sensor array
      
      PF::raw_pixel_t *p_in = NULL;
      PF::raw_pixel_t *p_in1 = NULL;
      PF::raw_pixel_t *p_in2 = NULL;
      PF::raw_pixel_t *p_out = NULL;
      
      for(int row = 0; row < oheight; row++)
      {
        p_in = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left-2, r->top + row );
        p_in1 = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left-2, r->top + row - 2);
        p_in2 = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left-2, r->top + row + 2);
        p_out = (PF::raw_pixel_t*)VIPS_REGION_ADDR( oreg, r->left, r->top + row );

        PF::RawMatrixRow in( p_in );
        PF::RawMatrixRow in1( p_in1 );
        PF::RawMatrixRow in2( p_in2 );
        PF::RawMatrixRow out( p_out );

        for(int col = 2; col < owidth - 2; col++)
        {
          float mid = in[col] * multiplier;
          if(in[col] > threshold)
          {
            int count = 0;
            float maxin = 0.0;
            float other;
    #define TESTONE(pin, OFFSET)                                                                                      \
      other = pin[col+OFFSET];                                                                                        \
      if(mid > other)                                                                                            \
      {                                                                                                          \
        count++;                                                                                                 \
        if(other > maxin) maxin = other;                                                                         \
      }
            TESTONE(in, -2);
            TESTONE(in1, 0);
            TESTONE(in, +2);
            TESTONE(in2, 0);
    #undef TESTONE
            if(count >= min_neighbours)
            {
              out[col-2] = maxin;
              fixed++;
              if(markfixed)
              {
                for(int i = -1; i >= -5 && i >= -col; i--) out[i+col-2] = in[col];
                for(int i = 1; i <= 5 && i < owidth - col; i++) out[i+col-2] = in[col];
              }
            }
          }
        }
      }

      rdpar->set_pixels_fixed(fixed);
    }
  };
  

  ProcessorBase* new_hotpixels();
}

#endif 


