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

#ifndef ICC_TRANSFORM_H
#define ICC_TRANSFORM_H

#include <cstring>
#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../base/processor.hh"


namespace PF 
{

  class ICCTransformPar: public OpParBase
  {
    std::string in_profile_name;

    cmsHPROFILE in_profile, out_profile;
    cmsHTRANSFORM transform;

    cmsColorSpaceSignature input_cs_type;
    cmsColorSpaceSignature output_cs_type;

  public:

    ICCTransformPar();

    cmsHTRANSFORM get_transform() { return transform; }

    void set_out_profile( cmsHPROFILE p ) { out_profile = p; }
    void set_image_hints( VipsImage* img )
    {
      if( !img ) return;
      OpParBase::set_image_hints( img );
      //rgb_image( get_xsize(), get_ysize() );
    }

    cmsColorSpaceSignature get_input_cs_type() { return input_cs_type; }
    cmsColorSpaceSignature get_output_cs_type() { return output_cs_type; }

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
  class ICCTransform
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      T* p;
      T* pin;
      T* pout;
      int x, y;

      for( y = 0; y < height; y++ ) {
        p = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        pin = p;
        if(opar->get_transform()) 
          cmsDoTransform( opar->get_transform(), pin, pout, width );
        else 
          memcpy( pout, pin, sizeof(T)*line_size );
      }
    }
  };




  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class ICCTransform< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      float* p;
      //float* pin;
      float* pout;
      int x, y;

      float* line = NULL;
      if( opar->get_input_cs_type() == cmsSigLabData ) {
        line = new float[line_size];
      }

      for( y = 0; y < height; y++ ) {
        p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        if(opar->get_transform()) {
          if( opar->get_input_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size; x+= 3 ) {
              line[x] = (cmsFloat32Number) (p[x] * 100.0);
              line[x+1] = (cmsFloat32Number) (p[x+1]*256.0f - 128.0f);
              line[x+2] = (cmsFloat32Number) (p[x+2]*256.0f - 128.0f);
              //line[x+1] = (cmsFloat32Number) ((p[x+1] - 0.5f) * 128.f);
              //line[x+2] = (cmsFloat32Number) ((p[x+2] - 0.5f) * 128.f);
              //if( r->left==0 && r->top==0 && x==0 && y==0 ) {
              //  std::cout<<"ICCTransform::render(): line="<<line[x]<<" "<<line[x+1]<<" "<<line[x+2]<<std::endl;
              //}
            }
            cmsDoTransform( opar->get_transform(), line, pout, width );
            //if( r->left==0 && r->top==0 && y==0 ) {
            //  std::cout<<"ICCTransform::render(): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
            //}
          } else {
            cmsDoTransform( opar->get_transform(), p, pout, width );
            if( opar->get_output_cs_type() == cmsSigLabData ) {
              for( x = 0; x < line_size; x+= 3 ) {
                pout[x] = (cmsFloat32Number) (pout[x] / 100.0); 
                pout[x+1] = (cmsFloat32Number) ((pout[x+1] + 128.0f) / 256.0f);
                pout[x+2] = (cmsFloat32Number) ((pout[x+2] + 128.0f) / 256.0f);
                //pout[x+1] = (cmsFloat32Number) ((pout[x+1] / 128.f) + 0.5f);
                //pout[x+2] = (cmsFloat32Number) ((pout[x+2] / 128.f) + 0.5f);
                
                //if( r->left==0 && r->top==0 && x==0 && y==0 ) {
                //  std::cout<<"Convert2LabProc::render(): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<std::endl;
                //}
              }
            }
          }
        } else {
          memcpy( pout, p, sizeof(float)*line_size );
        }
      }

      if( opar->get_input_cs_type() == cmsSigLabData && line ) {
        delete( line );
      }
    }
  };




  template < OP_TEMPLATE_DEF_TYPE_SPEC > 
  class ICCTransform< OP_TEMPLATE_IMP_TYPE_SPEC(double) >
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      double* p;
      double* pin;
      double* pout;
      int x, y;

      double* line = NULL;
      if( opar->get_input_cs_type() == cmsSigLabData ) {
        line = new double[line_size];
      }

      for( y = 0; y < height; y++ ) {
        p = (double*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y ); 
        pout = (double*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

        if(opar->get_transform()) {
          if( opar->get_input_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size; x+= 3 ) {
              line[x] = (cmsFloat64Number) (pin[x] * 100.0); 
              line[x+1] = (cmsFloat64Number) (pin[x+1]*256.0 - 128.0);
              line[x+2] = (cmsFloat64Number) (pin[x+2]*256.0 - 128.0);
            }
            cmsDoTransform( opar->get_transform(), line, pout, width );
          } else {
            cmsDoTransform( opar->get_transform(), pin, pout, width );
          }
        } else {
          memcpy( pout, pin, sizeof(double)*line_size );
        }
      }

      if( opar->get_input_cs_type() == cmsSigLabData && line ) {
        delete( line );
      }
    }
  };




  ProcessorBase* new_icc_transform();
}

#endif 


