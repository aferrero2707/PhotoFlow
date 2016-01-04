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

#ifndef VIPS_RAW_PREPROCESSOR_H
#define VIPS_RAW_PREPROCESSOR_H

#include <string>

#include "../base/processor.hh"
#include "../base/rawmatrix.hh"

#include "raw_image.hh"


//#define RT_EMU 1


#define __CLIP(a) (a)

namespace PF 
{

  extern int raw_preproc_sample_x;
  extern int raw_preproc_sample_y;

  enum wb_mode_t {
    WB_CAMERA=0,
    WB_SPOT=1,
    WB_COLOR_SPOT=2,
    WB_DAYLIGHT,
    WB_DIRECT_SUNLIGHT,
    WB_CLOUDY,
    WB_SHADE,
    WB_INCANDESCENT,
    WB_INCANDESCENT_WARM,
    WB_TUNGSTEN,
    WB_FLUORESCENT,
    WB_FLUORESCENT_HIGH,
    WB_COOL_WHITE_FLUORESCENT,
    WB_WARM_WHITE_FLUORESCENT,
    WB_DAYLIGHT_FLUORESCENT,
    WB_NEUTRAL_FLUORESCENT,
    WB_WHITE_FLUORESCENT,
    WB_SODIUM_VAPOR_FLUORESCENT,
    WB_DAY_WHITE_FLUORESCENT,
    WB_HIGH_TEMP_MERCURY_VAPOR_FLUORESCENT,
    WB_FLASH,
    WB_FLASH_AUTO,
    WB_EVENING_SUN,
    WB_UNDERWATER,
    WB_BACK_AND_WHITE
  }; 


  enum exposure_mode_t {
    EXP_NORMAL,
    EXP_AUTO
  };


  class RawPreprocessorPar: public OpParBase
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

    Property<float> exposure;
    PropertyBase exposure_mode;
    Property<float> exposure_clip_amount;
    Property<int> black_level_correction;

    float wb_red_current, wb_green_current, wb_blue_current, exposure_current;

  public:
    RawPreprocessorPar();

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

    wb_mode_t get_wb_mode() { return (wb_mode_t)(wb_mode.get_enum_value().first); }
    float get_wb_red() { return wb_red_current; /*wb_red.get();*/ }
    float get_wb_green() { return wb_green_current; /*wb_green.get();*/ }
    float get_wb_blue() { return wb_blue_current; /*wb_blue.get();*/ }

    float get_camwb_corr_red() { return camwb_corr_red.get(); }
    float get_camwb_corr_green() { return camwb_corr_green.get(); }
    float get_camwb_corr_blue() { return camwb_corr_blue.get(); }

    void set_wb(float r, float g, float b) {
      wb_red_current = r;
      wb_green_current = g;
      wb_blue_current = b;
      std::cout<<"RawPreprocessorPar: setting WB coefficients to "<<r<<","<<g<<","<<b<<std::endl;
    }

    float get_exposure() { return exposure.get(); }

    float get_black_level_correction() { return black_level_correction.get(); }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
										 VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class RawPreprocessor
  {
  public: 
    void render_camwb(VipsRegion** ireg, int n, int in_first,
											VipsRegion* imap, VipsRegion* omap, 
											VipsRegion* oreg, RawPreprocessorPar* par)
    {
      dcraw_data_t* image_data = par->get_image_data();
      float exposure = par->get_exposure();
      Rect *r = &oreg->valid;
      int nbands = ireg[in_first]->im->Bands;
    
      int x, y;
      //float range = image_data->color.maximum - image_data->color.black;
      float range = 1;
			float mul[4] = {
	        //image_data->color.cam_mul[0]*par->get_camwb_corr_red(),
	        //image_data->color.cam_mul[1]*par->get_camwb_corr_green(),
	        //image_data->color.cam_mul[2]*par->get_camwb_corr_blue(),
	        //image_data->color.cam_mul[3]*par->get_camwb_corr_green()
	        par->get_wb_red()*par->get_camwb_corr_red(),
	        par->get_wb_green()*par->get_camwb_corr_green(),
	        par->get_wb_blue()*par->get_camwb_corr_blue(),
	        par->get_wb_green()*par->get_camwb_corr_green()
			};
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
#ifdef RT_EMU
      /* RawTherapee emulation */
      range *= max_mul;
#else      
      range *= min_mul;
      //range *= max_mul;
#endif
    
      float black[4];
      for(int i = 0; i < 4; i++) {
				mul[i] = mul[i] * exposure / range;
        black[i] = par->get_black_level_correction() * exposure / (image_data->color.maximum - image_data->color.black);
        if( nbands != 3 ) black[i] *= 65535;
        //std::cout<<"black="<<par->get_black_level_correction()<<" * 65535 * "
        //    <<exposure<<" / "<<(image_data->color.maximum - image_data->color.black)
        //    <<"="<<black[i]<<std::endl;
      }
    
      //if(r->left==0 && r->top==0) std::cout<<"RawPreprocessor::render_camwb(): nbands="<<nbands<<std::endl;
      if( nbands == 3 ) {
				float* p;
				float* pout;
				int line_sz = r->width*3;
				for( y = 0; y < r->height; y++ ) {
					p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
					pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
					for( x=0; x < line_sz; x+=3) {
						pout[x] = CLIP(p[x] * mul[0] - black[0]);
						pout[x+1] = CLIP(p[x+1] * mul[1] - black[1]);
						pout[x+2] = CLIP(p[x+2] * mul[2] - black[2]);
						//if(r->left==0 && r->top==0) std::cout<<"  p["<<x<<"]="<<p[x]<<"  pout["<<x<<"]="<<pout[x]<<std::endl;
#ifdef RT_EMU
						/* RawTherapee emulation */
						pout[x] *= 65535;
						pout[x+1] *= 65535;
						pout[x+2] *= 65535;
#endif
					}
				}
      } else {
				PF::raw_pixel_t* p;
				PF::raw_pixel_t* pout;
				for( y = 0; y < r->height; y++ ) {
					p = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
					pout = (PF::raw_pixel_t*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
					PF::RawMatrixRow rp( p );
					PF::RawMatrixRow rpout( pout );
					for( x=0; x < r->width; x++) {
					  //std::cout<<"RawPreprocessor: x="<<x<<"  r->width="<<r->width
					  //      <<"  size of pel="<<VIPS_IMAGE_SIZEOF_PEL(ireg[in_first]->im)
					  //      <<","<<VIPS_IMAGE_SIZEOF_PEL(oreg->im)<<std::endl;
						rpout.color(x) = rp.color(x);
						rpout[x] = __CLIP(rp[x] * mul[ rp.icolor(x) ] - black[ rp.icolor(x) ]);
						if(false && r->left==0 && r->top==0)
            std::cout<<"  rp.color(x)="<<rp.color(x)
                <<"  rp[x]="<<rp[x]<<"  mul[ rp.icolor(x) ]="
                <<mul[ rp.icolor(x) ]<<"  rpout[x]="<<rpout[x]<<std::endl;
#ifdef RT_EMU
						/* RawTherapee emulation */
						rpout[x] *= 65535;
#endif
					}
				}
      }
    }


    void render_spotwb(VipsRegion** ireg, int n, int in_first,
											 VipsRegion* imap, VipsRegion* omap, 
											 VipsRegion* oreg, RawPreprocessorPar* par)
    {
      dcraw_data_t* image_data = par->get_image_data();
      float exposure = par->get_exposure();
      Rect *r = &oreg->valid;
      int nbands = ireg[in_first]->im->Bands;
    
      //PF::RawPixel* p;
      //PF::RawPixel* pout;
      int x, y;
      //float range = image_data->color.maximum - image_data->color.black;
      float range = 1;
      float min_mul = par->get_wb_red();
      float max_mul = par->get_wb_red();

      if( par->get_wb_green() < min_mul )
				min_mul = par->get_wb_green();
      if( par->get_wb_green() > max_mul )
				max_mul = par->get_wb_green();

      if( par->get_wb_blue() < min_mul )
				min_mul = par->get_wb_blue();
      if( par->get_wb_blue() > max_mul )
				max_mul = par->get_wb_blue();

      //std::cout<<"WB mult: "<<par->get_wb_red()<<" "<<par->get_wb_green()<<" "<<par->get_wb_blue()<<std::endl;
      //std::cout<<"range="<<range<<"  min_mul="<<min_mul<<"  new range="<<range*min_mul<<std::endl;
#ifdef RT_EMU
      /* RawTherapee emulation */
      range *= max_mul;
#else
      range *= min_mul;
#endif

      float mul[4] = { 
				par->get_wb_red() * exposure / range, 
				par->get_wb_green() * exposure / range, 
				par->get_wb_blue() * exposure / range, 
				par->get_wb_green() * exposure / range
      };

      float black[4];
      for(int i = 0; i < 4; i++) {
        mul[i] = mul[i] * exposure / range;
        black[i] = par->get_black_level_correction() * exposure / (image_data->color.maximum - image_data->color.black);
        if( nbands != 3 ) black[i] *= 65535;
        //std::cout<<"black="<<par->get_black_level_correction()<<" * 65535 * "
        //    <<exposure<<" / "<<(image_data->color.maximum - image_data->color.black)
        //    <<"="<<black[i]<<std::endl;
      }

      //std::cout<<"render_spotwb(): region left/top="<<r->left<<","<<r->top<<"+"<<r->width<<"+"<<r->height<<std::endl;
      //std::cout<<"  raw_preproc_sample_x="<<raw_preproc_sample_x<<std::endl;
      //std::cout<<"  raw_preproc_sample_y="<<raw_preproc_sample_y<<std::endl;
    
      if( nbands == 3 ) {
				float* p;
				float* pout;
				int line_sz = r->width*3;
				for( y = 0; y < r->height; y++ ) {
					p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
					pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
					for( x=0; x < line_sz; x+=3) {
						pout[x] = CLIP(p[x] * mul[0] - black[0]);
						pout[x+1] = CLIP(p[x+1] * mul[1] - black[1]);
						pout[x+2] = CLIP(p[x+2] * mul[2] - black[2]);
#ifdef RT_EMU
						/* RawTherapee emulation */
						pout[x] *= 65535;
						pout[x+1] *= 65535;
						pout[x+2] *= 65535;
#endif
					}
				}
      } else {
				PF::raw_pixel_t* p;
				PF::raw_pixel_t* pout;
				for( y = 0; y < r->height; y++ ) {
					p = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
					pout = (PF::raw_pixel_t*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
					PF::RawMatrixRow rp( p );
					PF::RawMatrixRow rpout( pout );
					for( x=0; x < r->width; x++) {
						rpout.color(x) = rp.color(x);
						rpout[x] = __CLIP(rp[x] * mul[ rp.icolor(x) ] - black[ rp.icolor(x) ]);
            
            int dx = r->left+x-raw_preproc_sample_x;
            int dy = r->top+y-raw_preproc_sample_y;
            //if( raw_preproc_sample_x > 0 && raw_preproc_sample_y > 0 )
            //  std::cout<<"  dx="<<dx<<"  dy="<<dy<<std::endl;
						if( false && abs(dx)<2 && abs(dy)<1 ) 
              std::cout<<"  rp["<<x<<"]="<<rp[x]
                       <<"  mul["<<(int)rp.color(x)<<"]="<<mul[ rp.icolor(x) ]
                       <<"  rpout["<<x<<"]="<<rpout[x]<<std::endl;
#ifdef RT_EMU
						/* RawTherapee emulation */
						rpout[x] *= 65535;
#endif
					}
				}
      }
    }


    void render(VipsRegion** ireg, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* oreg, OpParBase* par)
    {
      RawPreprocessorPar* rdpar = dynamic_cast<RawPreprocessorPar*>(par);
      if( !rdpar ) return;
      //std::cout<<"RawPreprocessor::render(): rdpar->get_wb_mode()="<<rdpar->get_wb_mode()<<std::endl;
      switch( rdpar->get_wb_mode() ) {
      case WB_SPOT:
      case WB_COLOR_SPOT:
				render_spotwb(ireg, n, in_first, imap, omap, oreg, rdpar);
				//std::cout<<"render_spotwb() called"<<std::endl;
				break;
      default:
        render_camwb(ireg, n, in_first, imap, omap, oreg, rdpar);
        //std::cout<<"render_camwb() called"<<std::endl;
        break;
      }
    }
  };




  ProcessorBase* new_raw_preprocessor();
}

#endif 


