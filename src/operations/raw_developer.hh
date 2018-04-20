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

#ifndef PF_RAW_DEVELOPER_H
#define PF_RAW_DEVELOPER_H

#include <string>

//#include <libraw/libraw.h>

#include "../base/processor.hh"
#include "../base/imagepyramid.hh"
#include "../base/rawmatrix.hh"

#include "raw_image.hh"
#include "raw_preprocessor.hh"

namespace PF 
{

  enum demo_method_t {
    PF_DEMO_FAST,
    PF_DEMO_AMAZE,
    PF_DEMO_RCD,
    PF_DEMO_LMMSE,
    PF_DEMO_IGV,
    PF_DEMO_NONE
  };

  enum TCA_correction_mode_t
  {
    PF_TCA_CORR_PROFILED_AUTO,
    PF_TCA_CORR_PROFILED,
    PF_TCA_CORR_AUTO,
    PF_TCA_CORR_MANUAL
  };

  class RawDeveloperPar: public OpParBase
  {
    VipsBandFormat output_format;
    dcraw_data_t* image_data;
    PF::ProcessorBase* amaze_demosaic;
    PF::ProcessorBase* lmmse_demosaic;
    PF::ProcessorBase* igv_demosaic;
    PF::ProcessorBase* rcd_demosaic;
    PF::ProcessorBase* no_demosaic;
    PF::ProcessorBase* xtrans_demosaic;
    PF::ProcessorBase* fast_demosaic;
    PF::ProcessorBase* fast_demosaic_xtrans;
    PF::ProcessorBase* raw_preprocessor;
    PF::ProcessorBase* ca_correct;
    PF::ProcessorBase* lensfun;
    PF::ProcessorBase* raw_output;
    PF::ProcessorBase* convert_format;
    PF::ProcessorBase* fcs[4];
    PF::ProcessorBase* hotpixels;

    Property<std::string> lf_prop_camera_maker;
    Property<std::string> lf_prop_camera_model;
    Property<std::string> lf_prop_lens;

    Property<bool> enable_distortion, enable_tca, enable_vignetting, enable_all, auto_crop;
    // False color suppression steps
    PropertyBase tca_method;
    // False color suppression steps
    PropertyBase demo_method;
    // False color suppression steps
    Property<int> fcs_steps;
    // Highlights reconstruction mode
    PropertyBase hlreco_mode;

    bool caching_enabled;

  public:
    RawDeveloperPar();

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
    bool needs_caching() { return caching_enabled; }

    void set_caching( bool flag ) { caching_enabled = flag; }

    dcraw_data_t* get_image_data() {return image_data; }
    RawPreprocessorPar* get_rawpreprocessor_par() {
      return dynamic_cast<PF::RawPreprocessorPar*>( raw_preprocessor->get_par() );
    }

    wb_mode_t get_wb_mode();
    void get_wb(float* mul);
    void set_wb(float r, float g, float b);
    void add_wb_area(std::vector<int>& area);
    std::vector< std::vector<int> >& get_wb_areas();

    int get_hotp_fixed();

    int get_tca_method() { return tca_method.get_enum_value().first; }
    bool get_tca_enabled() { return enable_tca.get(); }
    bool get_all_enabled() { return enable_all.get(); }

    std::string get_lf_maker();
    std::string get_lf_model();
    std::string get_lf_lens();


    VipsImage* build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap, unsigned int& level);
  };



  template < OP_TEMPLATE_DEF >
  class RawDeveloper
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
        VipsRegion* imap, VipsRegion* omap,
        VipsRegion* oreg, OpParBase* par)
    {
      /*
      RawDeveloperPar* rdpar = dynamic_cast<RawDeveloperPar*>(par);
      if( !rdpar ) return;
      dcraw_data_t* image_data = rdpar->get_image_data();
      Rect *r = &oreg->valid;
      //int sz = oreg->im->Bands;//IM_REGION_N_ELEMENTS( oreg );
      //int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

      PF::raw_pixel_t* p;
      PF::raw_pixel_t* pout;
      int x, y;
      float range = image_data->color.maximum - image_data->color.black;
      float min_mul = image_data->color.cam_mul[0];
      float max_mul = image_data->color.cam_mul[0];
      for(int i = 1; i < 4; i++) {
	if( image_data->color.cam_mul[i] < min_mul )
	  min_mul = image_data->color.cam_mul[i];
	if( image_data->color.cam_mul[i] > max_mul )
	  max_mul = image_data->color.cam_mul[i];
      }
#ifndef NDEBUG
      if( r->top==0 && r->left==0 ) {
	std::cout<<"RawDeveloper::render(): input format="<<ireg[in_first]->im->BandFmt
		 <<"  output format="<<oreg->im->BandFmt
		 <<std::endl;
      }
#endif
      std::cout<<"range="<<range<<"  min_mul="<<min_mul<<"  new range="<<range*min_mul<<std::endl;
      // RawTherapee emulation
      //range *= max_mul;

      range *= min_mul;

      for( y = 0; y < r->height; y++ ) {
	p = (PF::raw_pixel_t*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
	pout = (PF::raw_pixel_t*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
		PF::RawMatrixRow rp( p );
		PF::RawMatrixRow rpout( pout );
	for( x=0; x < r->width; x++) {
	  //std::cout<<"x: "<<x<<"  y: "<<y<<"  color: "<<(int)p[x].color<<std::endl;
	  rpout.color(x) = rp.color(x);
	  rpout[x] = rp[x] * image_data->color.cam_mul[ rp.color(x) ];
	  rpout[x] /= range;
       */
      /* RawTherapee emulation
	  pout[x].data *= 65535;
       */
      /*
	  std::cout<<"data: input="<<p[x].data<<"  output="<<pout[x].data
		   <<"  cam_mul="<<image_data->color.cam_mul[ p[x].color ]
		   <<"  range="<<range<<std::endl;
       */
      /*
		}
	}
       */
    }
  };




  ProcessorBase* new_raw_developer();
}

#endif 


