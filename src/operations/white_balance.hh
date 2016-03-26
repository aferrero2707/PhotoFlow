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

#ifndef PF_WHITE_BALANCE_H
#define PF_WHITE_BALANCE_H

#include <string>

#include "../base/processor.hh"
#include "../base/rawmatrix.hh"

#include "raw_image.hh"


//#define RT_EMU 1


//#define __CLIP(a)  CLIP(a)
#define __CLIP(a) (a)

namespace PF 
{

  extern int wb_sample_x;
  extern int wb_sample_y;

  class WhiteBalancePar: public OpParBase
  {
    dcraw_data_t* image_data;

    PropertyBase wb_mode;

    Property<float> wb_red;
    Property<float> wb_green;
    Property<float> wb_blue;

    Property<float> camwb_corr_red;
    Property<float> camwb_corr_green;
    Property<float> camwb_corr_blue;

    Property<float> wb_target_L;
    Property<float> wb_target_a;
    Property<float> wb_target_b;

    Property<float> saturation_level_correction;
    Property<float> black_level_correction;

    float wb_red_current, wb_green_current, wb_blue_current;

  public:
    WhiteBalancePar();

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
			 creation of an intensity map is not allowed
       2. the operation can work without an input image;
			 the blending will be set in this case to "passthrough" and the image
			 data will be simply linked to the output
		*/
    bool has_intensity() { return false; }
    bool has_opacity() { return true; }
    bool needs_input() { return true; }

    dcraw_data_t* get_image_data() {return image_data; }

    wb_mode_t get_wb_mode() { return (wb_mode_t)(wb_mode.get_enum_value().first); }
    float get_wb_red() { return wb_red_current; /*wb_red.get();*/ }
    float get_wb_green() { return wb_green_current; /*wb_green.get();*/ }
    float get_wb_blue() { return wb_blue_current; /*wb_blue.get();*/ }

    float get_camwb_corr_red() { return camwb_corr_red.get(); }
    float get_camwb_corr_green() { return camwb_corr_green.get(); }
    float get_camwb_corr_blue() { return camwb_corr_blue.get(); }

    void get_wb(float* mul) {
      mul[0] = get_wb_red();
      mul[1] = get_wb_green();
      mul[2] = get_wb_blue();
    }
    void set_wb(float r, float g, float b) {
      wb_red_current = r;
      wb_green_current = g;
      wb_blue_current = b;
      std::cout<<"WhiteBalancePar: setting WB coefficients to "<<r<<","<<g<<","<<b<<std::endl;
    }

    float get_saturation_level_correction() { return saturation_level_correction.get(); }
    float get_black_level_correction() { return black_level_correction.get(); }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class WhiteBalance
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* oreg, OpParBase* par)
    {
      WhiteBalancePar* rdpar = dynamic_cast<WhiteBalancePar*>(par);
      if( !rdpar ) return;

      float mul[4] = {1,1,1,1};

      //std::cout<<"WhiteBalance::render(): rdpar->get_wb_mode()="<<rdpar->get_wb_mode()<<std::endl;
      //switch( rdpar->get_wb_mode() ) {
      //case WB_SPOT:
      //case WB_COLOR_SPOT:
				//render_spotwb(ireg, n, in_first, imap, omap, oreg, rdpar);
				//std::cout<<"render_spotwb() called"<<std::endl;
        mul[0] = rdpar->get_wb_red();
        mul[1] = rdpar->get_wb_green();
        mul[2] = rdpar->get_wb_blue();
        mul[3] = rdpar->get_wb_green();
				//break;
      //default:
        //render_camwb(ireg, n, in_first, imap, omap, oreg, rdpar);
        //std::cout<<"render_camwb() called"<<std::endl;
        //mul[0] = rdpar->get_wb_red()*rdpar->get_camwb_corr_red();
        //mul[1] = rdpar->get_wb_green()*rdpar->get_camwb_corr_green();
        //mul[2] = rdpar->get_wb_blue()*rdpar->get_camwb_corr_blue();
        //mul[3] = rdpar->get_wb_green()*rdpar->get_camwb_corr_green();
        //break;
      //}

      dcraw_data_t* image_data = rdpar->get_image_data();
      Rect *r = &oreg->valid;
      int nbands = ireg[in_first]->im->Bands;

      int x, y;
      //float range = image_data->color.maximum - image_data->color.black;
      float range = 1;
      float min_mul = mul[0];
      float max_mul = mul[0];
      /*
      for(int i = 0; i < 4; i++) {
        std::cout<<"cam_mu["<<i<<"]="<<image_data->color.cam_mul[i]<<"  ";
      }
      std::cout<<std::endl;
      */
      for(int i = 1; i < 4; i++) {
        if( mul[i] < min_mul )
          min_mul = mul[i];
        if( mul[i] > max_mul )
          max_mul = mul[i];
      }
      //std::cout<<"range="<<range<<"  min_mul="<<min_mul<<"  new range="<<range*min_mul<<std::endl;
      range *= min_mul;

      //for(int i = 0; i < 4; i++) {
      //  mul[i] = mul[i] / range;
      //}

      float* p;
      float* pout;
      int line_sz = r->width*3;
      for( y = 0; y < r->height; y++ ) {
        p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
        for( x=0; x < line_sz; x+=3) {
          //pout[x] = __CLIP( (p[x]-black[0]) * sat_corr * mul[0] - black[0]);
          //pout[x+1] = __CLIP(p[x+1] * sat_corr * mul[1] - black[1]);
          //pout[x+2] = __CLIP(p[x+2] * sat_corr * mul[2] - black[2]);
          pout[x] = __CLIP( p[x] * mul[0] );
          pout[x+1] = __CLIP( p[x+1] * mul[1] );
          pout[x+2] = __CLIP( p[x+2] * mul[2] );
          if(false && r->left==0 && r->top==0) std::cout<<"  p["<<x<<"]="<<p[x]<<"  pout["<<x<<"]="<<pout[x]<<std::endl;
        }
      }
    }
  };




  ProcessorBase* new_white_balance();
}

#endif 


