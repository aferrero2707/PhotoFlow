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

    ICCProfile* in_profile;
    ICCProfile* out_profile;
    cmsUInt32Number intent;
    bool bpc;
    float adaptation_state;
    PF::ICCTransform transform;
    bool do_LCh, do_LSh, do_Lab;

    cmsColorSpaceSignature input_cs_type;
    cmsColorSpaceSignature output_cs_type;

    bool clip_negative, clip_overflow, gamut_mapping;
    float saturation_intent;

  public:

    ICCTransformPar();

    cmsUInt32Number get_intent() { return intent; }
    void set_intent( cmsUInt32Number i ) { intent = i; }

    void set_Lab_format() { do_Lab = true; do_LCh = do_LSh = false; }
    void set_LCh_format() { do_LCh = true; do_Lab = do_LSh = false; }
    void set_LSh_format() { do_LSh = true; do_Lab = do_LCh = false; }

    bool get_Lab_format() { return do_Lab; }
    bool get_LCh_format() { return do_LCh; }
    bool get_LSh_format() { return do_LSh; }

    bool get_bpc() { return bpc; }
    void set_bpc( bool flag ) { bpc = flag; }
    void set_adaptation_state( float s ) { adaptation_state = s; }

    PF::ICCTransform& get_transform() { return transform; }

    ICCProfile* get_in_profile() { return in_profile; }
    void set_out_profile( ICCProfile* p ) { out_profile = p; }
    ICCProfile* get_out_profile() { return out_profile; }

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

    bool get_clip_negative() { return clip_negative; }
    bool get_clip_overflow() { return clip_overflow; }
    void set_clip_negative( bool flag ) { clip_negative = flag; }
    void set_clip_overflow( bool flag ) { clip_overflow = flag; }

    bool get_gamut_mapping() { return gamut_mapping; }
    void set_gamut_mapping( bool flag ) { gamut_mapping = flag; }
    float get_saturation_intent() { return saturation_intent; }
    void set_saturation_intent( float s ) { saturation_intent = s; }

    //cmsHPROFILE create_profile_from_matrix (const double matrix[3][3], bool gamma, Glib::ustring name);

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
                     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };



  template < OP_TEMPLATE_DEF >
  class ICCTransformProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
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
        if(opar->get_transform().valid())
          //cmsDoTransform( opar->get_transform(), pin, pout, width );
          opar->get_transform().apply(pin,pout,width);
        else
          memcpy( pout, pin, sizeof(T)*line_size );
      }
    }
  };




  template < OP_TEMPLATE_DEF_TYPE_SPEC >
  class ICCTransformProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
      int line_size_in = ireg[in_first]->valid.width * ireg[in_first]->im->Bands; //layer->in_all[0]->Bands;
      int line_size_out = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
      int line_size_max = (line_size_in > line_size_out) ? line_size_in : line_size_out;
      int width = r->width;
      int height = r->height;

      float* p;
      //float* pin;
      float* pout;
      int x, y;

      float* line = NULL;
      if( opar->get_input_cs_type() == cmsSigLabData ||
          opar->get_input_cs_type() == cmsSigCmykData ||
          opar->get_gamut_mapping() ) {
        line = new float[line_size_max];
      }

      for( y = 0; y < height; y++ ) {
        //std::cout<<"icc_transform: ti="<<ti<<" y="<<y<<"  corner="<<r->left<<","<<r->top<<std::endl;
        p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        if(opar->get_transform().valid()) {
          if( opar->get_input_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size_in; x+= 3 ) {
              line[x] = (cmsFloat32Number) (p[x] * 100.0);
              line[x+1] = (cmsFloat32Number) (p[x+1]*256.0f - 128.0f);
              line[x+2] = (cmsFloat32Number) (p[x+2]*256.0f - 128.0f);
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ICCTransform::render(): line="<<line[x]<<" "<<line[x+1]<<" "<<line[x+2]<<std::endl;
              }
            }
            //cmsDoTransform( opar->get_transform(), line, pout, width );
            opar->get_transform().apply(line,pout,width);
            if( false && r->left==0 && r->top==0 && y==0 ) {
              std::cout<<"ICCTransform::render(Lab): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
            }
          } else if( opar->get_input_cs_type() == cmsSigCmykData ) {
            for( x = 0; x < line_size_in; x+= 4 ) {
              line[x] = (cmsFloat32Number) (p[x] * 100.0);
              line[x+1] = (cmsFloat32Number) (p[x+1] * 100.0);
              line[x+2] = (cmsFloat32Number) (p[x+2] * 100.0);
              line[x+3] = (cmsFloat32Number) (p[x+3] * 100.0);
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ICCTransform::render(CMYK in): line="<<line[x]<<" "<<line[x+1]<<" "<<line[x+2]<<" "<<line[x+3]<<std::endl;
              }
            }
            //cmsDoTransform( opar->get_transform(), line, pout, width );
            opar->get_transform().apply(line,pout,width);
            if( false && r->left==0 && r->top==0 && y==0 ) {
              std::cout<<"ICCTransform::render(CMYK in): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
            }
          } else {
            if( opar->get_in_profile() && opar->get_in_profile()->is_rgb() &&
                opar->get_out_profile() && opar->get_out_profile()->is_rgb() &&
                opar->get_gamut_mapping() ) {
              ICCProfile* inprof = opar->get_in_profile();
              ICCProfile* outprof = opar->get_out_profile();
              float** gbound = outprof->get_gamut_boundary();
              float* gLid = outprof->get_gamut_Lid_Cmax();
              for( x = 0; x < line_size_out; x+= 3 ) {
                line[x] = p[x]; line[x+1] = p[x+1]; line[x+2] = p[x+2];
                inprof->gamut_mapping(line[x], line[x+1], line[x+2], gbound, gLid, opar->get_saturation_intent());
              }
              opar->get_transform().apply(line,pout,width);
            } else {
              //cmsDoTransform( opar->get_transform(), p, pout, width );
              opar->get_transform().apply(p,pout,width);
            }
            if( false && r->left==0 && r->top==0 && y==0 ) {
              std::cout<<"ICCTransform::render(): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
            }
          }
          if( opar->get_output_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size_out; x+= 3 ) {

              if( opar->get_LCh_format() || opar->get_LSh_format() ) {
                PF::Lab2LCH( &(pout[x]), &(pout[x]), 1 );
                if( opar->get_LSh_format() ) {
                  float den = std::sqrt(pout[x]*pout[x] + pout[x+1]*pout[x+1]);
                  if( den > 1.0e-10 ) pout[x+1] /= den;
                  else pout[x+1] = 0;
                } else {
                  pout[x+1] /= 256.0f;
                }
                pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
                //std::cout<<"H: "<<pout[x+2]<<std::endl;
                pout[x+2] /= (M_PI*2);
              } else {
                pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
                pout[x+1] = (cmsFloat32Number) ((pout[x+1] + 128.0f) / 256.0f);
                pout[x+2] = (cmsFloat32Number) ((pout[x+2] + 128.0f) / 256.0f);
              }

              //if( r->left==0 && r->top==0 && x==0 && y==0 ) {
              //  std::cout<<"Convert2LabProc::render(): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<std::endl;
              //}
            }
          } else if( opar->get_output_cs_type() == cmsSigCmykData ) {
            for( x = 0; x < line_size_out; x+= 4 ) {
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ICCTransform::render(CMYK out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<pout[x+3]<<std::endl;
              }
              pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
              pout[x+1] = (cmsFloat32Number) (pout[x+1] / 100.0);
              pout[x+2] = (cmsFloat32Number) (pout[x+2] / 100.0);
              pout[x+3] = (cmsFloat32Number) (pout[x+3] / 100.0);
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ICCTransform::render(CMYK out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<pout[x+3]<<std::endl;
              }
            }
          }
        } else {
          memcpy( pout, p, sizeof(float)*line_size_in );
        }
        //std::cout<<"opar->get_out_profile(): "<<opar->get_out_profile()
        //    <<"  opar->get_out_profile()->is_rgb(): "<<opar->get_out_profile()->is_rgb()
        //    <<"  opar->get_gamut_mapping(): "<<opar->get_gamut_mapping()<<std::endl;
        if( opar->get_clip_negative() || opar->get_clip_overflow() ) {
          for( x = 0; x < line_size_out; x+= 1 ) {
            if( opar->get_clip_negative() ) pout[x] = MAX( pout[x], 0.f );
            if( opar->get_clip_overflow() ) pout[x] = MIN( pout[x], 1.f );
          }
        }
      }

      if( line ) {
        delete( line );
      }
    }
  };



  /*
  template < OP_TEMPLATE_DEF_TYPE_SPEC >
  class ICCTransformProc< OP_TEMPLATE_IMP_TYPE_SPEC(double) >
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, OpParBase* par)
    {
      ICCTransformPar* opar = dynamic_cast<ICCTransformPar*>(par);
      if( !opar ) return;
      VipsRect *r = &oreg->valid;
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

        if(opar->get_transform().valid()) {
          if( opar->get_input_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size; x+= 3 ) {
              line[x] = (cmsFloat64Number) (pin[x] * 100.0);
              line[x+1] = (cmsFloat64Number) (pin[x+1]*256.0 - 128.0);
              line[x+2] = (cmsFloat64Number) (pin[x+2]*256.0 - 128.0);
            }
            //cmsDoTransform( opar->get_transform(), line, pout, width );
            opar->get_transform().apply(line,pout,width);
          } else {
            //cmsDoTransform( opar->get_transform(), pin, pout, width );
            opar->get_transform().apply(pin,pout,width);
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
  */



  ProcessorBase* new_icc_transform();
}

#endif 


