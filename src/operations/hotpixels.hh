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
      int nbands = ireg[in_first]->im->Bands;
      const int width = r->width;
      const int height = r->height;
      const int widthx2 = width * 2;
      const bool markfixed = ( rdpar->get_hotp_markfixed() && ( rdpar->get_render_mode() == PF_RENDER_PREVIEW ) );
      const int min_neighbours = rdpar->get_hotp_permissive() ? 3 : 4;

      float *cin = NULL;
      float *cin1 = NULL;
      float *cin2 = NULL;
      float *cout = NULL;
      
      std::cout<<"HotPixels::render "<<std::endl;
      std::cout<<"nbands: "<<nbands<<std::endl;
      

      // The loop should output only a few pixels, so just copy everything first
//      memcpy(o, i, (size_t)roi_out->width * roi_out->height * sizeof(float));
      for(int y = 0; y < height; y++)
      {
        cin = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        cout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for(int x = 0; x < width*nbands; x++, cin++, cout++)
        {
          *cout = *cin;
        }
      }
      
      int fixed = rdpar->get_pixels_fixed();

      // TODO: do we support xtrans?
/*      if(img->filters == 9u)
      {
        fixed = process_xtrans(i, o, roi_in, width, height, img->xtrans, threshold, multiplier, markfixed,
                               min_neighbours);
        goto processed;
      }
*/
    // Bayer sensor array
      
      PF::raw_pixel_t *p_in = NULL;
      PF::raw_pixel_t *p_in1 = NULL;
      PF::raw_pixel_t *p_in2 = NULL;
      PF::raw_pixel_t *p_out = NULL;
      

//      for(int row = 2; row < roi_out->height - 2; row++)
      for(int row = 2; row < height - 2; row++)
      {
//        const float *in = (float *)i + (size_t)width * row + 2;
//        float *out = (float *)o + (size_t)width * row + 2;
        
        p_in = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + row );
        p_in1 = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + row - 2);
        p_in2 = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + row + 2);
        p_out = (PF::raw_pixel_t*)VIPS_REGION_ADDR( oreg, r->left, r->top + row );

        PF::RawMatrixRow in( p_in );
        PF::RawMatrixRow in1( p_in1 );
        PF::RawMatrixRow in2( p_in2 );
        PF::RawMatrixRow out( p_out );

//        for(int col = 2; col < width - 1; col++, in++, out++)
        for(int col = 2; col < width - 2; col++)
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
//            TESTONE(in, -2);
//            TESTONE(-widthx2);
//            TESTONE(in, +2);
//            TESTONE(+widthx2);
            TESTONE(in, -2);
            TESTONE(in1, 0);
            TESTONE(in, +2);
            TESTONE(in2, 0);
    #undef TESTONE
            if(count >= min_neighbours)
            {
              out[col] = maxin;
              fixed++;
              if(markfixed)
              {
//                for(int i = -2; i >= -10 && i >= -col; i -= 2) out[i] = in[col];
//                for(int i = 2; i <= 10 && i < width - col; i += 2) out[i] = in[col];
                for(int i = -1; i >= -5 && i >= -col; i--) out[i+col] = in[col];
                for(int i = 1; i <= 5 && i < width - col; i++) out[i+col] = in[col];
              }
            }
          }
        }
      }

      rdpar->set_pixels_fixed(fixed);
      std::cout<<"pixels fixed: "<<fixed<<std::endl;
      
    }
  };
  
/*
  template < OP_TEMPLATE_DEF > 
  class HotPixelsProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, HotPixelsPar* par)
    {
    }
  };


  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class HotPixelsProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, HotPixelsPar* par)
    {
    }
  };
*/


  ProcessorBase* new_hotpixels();
}

#endif 


