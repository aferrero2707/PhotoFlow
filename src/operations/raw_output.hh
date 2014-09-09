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

#ifndef RAW_OUTPUT_H
#define RAW_OUTPUT_H

#include <string>

#include <glibmm.h>

#include <libraw/libraw.h>

#include "../base/processor.hh"

//#include "../rt/iccmatrices.hh"
#include "../dt/common/srgb_tone_curve_values.h"

#include "raw_image.hh"

namespace PF 
{

  enum input_profile_mode_t {
    IN_PROF_NONE,
    IN_PROF_MATRIX,
    IN_PROF_ICC
  }; 


  enum output_profile_mode_t {
    OUT_PROF_NONE,
    OUT_PROF_sRGB,
    OUT_PROF_ADOBE,
    OUT_PROF_PROPHOTO,
    OUT_PROF_LAB,
    OUT_PROF_CUSTOM
  }; 


  enum input_gamma_mode_t {
    IN_GAMMA_NONE,
    IN_GAMMA_sRGB,
    IN_GAMMA_CUSTOM
  }; 


  static
  cmsToneCurve* Build_sRGBGamma(cmsContext ContextID)
  {
    cmsFloat64Number Parameters[5];
    
    Parameters[0] = 2.4;
    Parameters[1] = 1. / 1.055;
    Parameters[2] = 0.055 / 1.055;
    Parameters[3] = 1. / 12.92;
    Parameters[4] = 0.04045;
    
    return cmsBuildParametricToneCurve(ContextID, 4, Parameters);
  }

  class RawOutputPar: public OpParBase
  {
    dcraw_data_t* image_data;

    // Conversion matrix from camera colorspace to xyz
    // Initialized as xyz_sRGB * rgb_cam
    double xyz_cam[3][3];

    PropertyBase profile_mode;
    input_profile_mode_t current_profile_mode;

    cmsToneCurve* srgb_curve;
    cmsToneCurve* gamma_curve;

    // Camera input color profile
    // Either from xyz_cam or from icc 
    Property<std::string> cam_profile_name;
    std::string current_cam_profile_name;
    cmsHPROFILE cam_profile;

    // Input gamma 
    PropertyBase gamma_mode;
    Property<float> gamma_lin;
    Property<float> gamma_exp;

    // output color profile
    PropertyBase out_profile_mode;
    output_profile_mode_t current_out_profile_mode;
    Property<std::string> out_profile_name;
    std::string current_out_profile_name;
    cmsHPROFILE out_profile;

    cmsHTRANSFORM transform;

  public:

    RawOutputPar();

    input_profile_mode_t get_camera_profile_mode() { return current_profile_mode; }
    input_gamma_mode_t get_gamma_mode() { return (input_gamma_mode_t)gamma_mode.get_enum_value().first; }
    cmsToneCurve* get_gamma_curve() { return gamma_curve; }
    cmsToneCurve* get_srgb_curve() { return srgb_curve; }
    cmsHTRANSFORM get_transform() { return transform; }

    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      rgb_image( get_xsize(), get_ysize() );
    }

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

    //cmsHPROFILE create_profile_from_matrix (const double matrix[3][3], bool gamma, Glib::ustring name);

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class RawOutput
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      RawOutputPar* opar = dynamic_cast<RawOutputPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      cmsToneCurve* srgb_curve = opar->get_srgb_curve();
      cmsToneCurve* gamma_curve = opar->get_gamma_curve();

      if( false && r->top==0 && r->left==0 ) {
        std::cout<<"RawOutput::render(): ireg[in_first]->im->Bands="<<ireg[in_first]->im->Bands
                 <<"  oreg->im->Bands="<<oreg->im->Bands<<std::endl;
        std::cout<<"RawOutput::render(): ireg[in_first]->im->BandFmt="<<ireg[in_first]->im->BandFmt
                 <<"  oreg->im->BandFmt="<<oreg->im->BandFmt<<std::endl;
      }
    
      T* p;
      T* pin;
      T* pout;
      int x, y;

      T* line = new T[line_size];

      //std::cout<<"opar->get_transform(): "<<opar->get_transform()<<std::endl;
    
      /*
      float max[3] = {0,0,0};
      for( y = 0; y < height; y++ ) {
        p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        for( x = 0; x < width; x+=3 ) {
          if(p[x] > max[0]) max[0] = p[x];
          if(p[x+1] > max[1]) max[1] = p[x+1];
          if(p[x+2] > max[2]) max[2] = p[x+2];
        }
      }
      std::cout<<"("<<r->left<<","<<r->top<<"): max = "<<max[0]<<"  "<<max[1]<<"  "<<max[2]<<std::endl;
      */

      for( y = 0; y < height; y++ ) {
        p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        if( opar->get_camera_profile_mode() == IN_PROF_ICC ) {

          pin = p;
          if( opar->get_gamma_mode() == IN_GAMMA_sRGB ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( srgb_curve, p[x] );
            }
            pin = line;
          } else if( opar->get_gamma_mode() == IN_GAMMA_CUSTOM ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( gamma_curve, p[x] );
            }
            pin = line;
          }
          if(opar->get_transform()) 
            cmsDoTransform( opar->get_transform(), pin, pout, width );
          else 
            memcpy( pout, pin, sizeof(T)*line_size );

        } else if( opar->get_camera_profile_mode() == IN_PROF_MATRIX ) {
          if(opar->get_transform()) 
            cmsDoTransform( opar->get_transform(), p, pout, width );
          else 
            memcpy( pout, p, sizeof(T)*line_size );
        } else {

          pin = p;
          if( opar->get_gamma_mode() == IN_GAMMA_sRGB ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( srgb_curve, p[x] );
            }
            pin = line;
          } else if( opar->get_gamma_mode() == IN_GAMMA_CUSTOM ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( gamma_curve, p[x] );
            }
            pin = line;
          }
          memcpy( pout, pin, sizeof(T)*line_size );

        }
      }
      delete line;
    }
  };




  ProcessorBase* new_raw_output();
}

#endif 


