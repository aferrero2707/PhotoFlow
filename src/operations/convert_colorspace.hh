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

#ifndef CONVERT_COLORSPACE_H
#define CONVERT_COLORSPACE_H

#include <cstring>
#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../base/processor.hh"


//#include "convert2lab.hh"

namespace PF 
{

class GamutWarningPar: public PF::OpParBase
{
  float delta;
  bool dest_is_matrix;
public:
  GamutWarningPar():
    delta( 0.0001 ),
    PF::OpParBase()
  {
    set_type("gamut_warning");
  }
  ~GamutWarningPar() { std::cout<<"~GamutWarningPar() called."<<std::endl; }

  void set_delta(float d) { delta = d; }
  float get_delta() { return delta; }
  void set_dest_is_matrix( bool val ) { dest_is_matrix = val; }
  bool get_dest_is_matrix() { return dest_is_matrix; }

  /*
    VipsImage* build(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level);
   */
};


template < OP_TEMPLATE_DEF >
class GamutWarningProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    if( n != 3 ) return;
    if( ireg[0] == NULL ) return;
    if( ireg[1] == NULL ) return;
    if( ireg[2] == NULL ) return;
    PF::GamutWarningPar* opar = dynamic_cast<PF::GamutWarningPar*>( par );
    if( opar == NULL ) return;

    Rect *r = &oreg->valid;
    int line_size  = r->width * 3;
    int line_size2 = r->width * oreg->im->Bands;
    int height = r->height;
    int nchan = oreg->im->Bands;

    T* p1;
    T* p2;
    T* pin;
    T* pout;
    int x, x2, y/*, pos*/;
    float diff1, diff2, diff3;
    float delta, delta_max = opar->get_delta(); //0.0001;

    for( y = 0; y < height; y++ ) {
      p1 = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      p2 = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      pin = (T*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0, x2 = 0; x < line_size; x+=3, x2+=nchan ) {
        if( opar->get_dest_is_matrix() ) {
          diff1 = (static_cast< float >(p1[x]) - static_cast< float >(p2[x]));
          diff2 = (static_cast< float >(p1[x+1]) - static_cast< float >(p2[x+1]));
          diff3 = (static_cast< float >(p1[x+2]) - static_cast< float >(p2[x+2]));
          delta = MAX3( fabs(diff1), fabs(diff2), fabs(diff3) );
          if( false && r->top==0 && r->left==0 && y<16 && x<16*3 ) {
            std::cout<<"in:  "<<p1[x]<<","<<p1[x+1]<<","<<p1[x+2]<<std::endl;
            std::cout<<"out: "<<p2[x]<<","<<p2[x+1]<<","<<p2[x+2]<<std::endl;
            std::cout<<"delta: "<<delta<<" ("<<diff1<<","<<diff2<<","<<diff3<<")  max: "<<delta_max<<std::endl;
          }
        } else {
          cmsCIELab LabIn1 = {
              (cmsFloat64Number)p1[x]*100,
              (cmsFloat64Number)p1[x+1]*255.0 - 128.0,
              (cmsFloat64Number)p1[x+2]*255.0 - 128.0
          };
          cmsCIELab LabOut1 = {
              (cmsFloat64Number)p2[x]*100,
              (cmsFloat64Number)p2[x+1]*255.0 - 128.0,
              (cmsFloat64Number)p2[x+2]*255.0 - 128.0
          };
          delta = cmsDeltaE(&LabIn1, &LabOut1);
          if( false && r->top==0 && r->left==0 && y<16 && x<16*3 ) {
            std::cout<<"in:  "<<p1[x]<<","<<p1[x+1]<<","<<p1[x+2]<<std::endl;
            std::cout<<"out: "<<p2[x]<<","<<p2[x+1]<<","<<p2[x+2]<<std::endl;
            std::cout<<"delta: "<<delta<<"  max: "<<delta_max<<std::endl;
          }
        }
        if( delta > delta_max /*diff > delta*/ ) {
          switch(nchan) {
          case 3: pout[x2] = pout[x2+1] = pout[x2+2] = PF::FormatInfo<T>::HALF; break;
          case 4: pout[x2] = pout[x2+1] = pout[x2+2] = pout[x2+3] = PF::FormatInfo<T>::HALF; break;
          default: break;
          }
        } else {
          pout[x2] = pin[x2];
          pout[x2+1] = pin[x2+1];
          pout[x2+2] = pin[x2+2];
          if(nchan==4) pout[x2+3] = pin[x2+3];
        }
      }
    }
  }
};


class ConvertColorspacePar: public OpParBase
{
  std::string in_profile_name;

  // output color profile
  PropertyBase out_profile_mode;
  PropertyBase out_profile_type;
  PropertyBase out_trc_type;
  Property<std::string> out_profile_name;
  PropertyBase intent;
  Property<bool> bpc;
  Property<float> adaptation_state;
  Property<bool> assign;

  Property<bool> clip_negative, clip_overflow;

  void* out_profile_data;
  int out_profile_data_length;

  cmsHTRANSFORM transform;
  ProcessorBase* cs_transform;
  ProcessorBase* gw_transform_in;
  ProcessorBase* gw_transform_out;
  ProcessorBase* gw;

  //ProcessorBase* convert2lab;

  bool softproof;
  bool gamut_warning;

  cmsColorSpaceSignature input_cs_type;
  cmsColorSpaceSignature output_cs_type;

public:

  ConvertColorspacePar();
  ~ConvertColorspacePar();

  cmsHTRANSFORM get_transform() { return transform; }

  int get_out_profile_mode() { return out_profile_mode.get_enum_value().first; }
  void set_out_profile_mode( profile_mode_t mode ) { out_profile_mode.set_enum_value( mode ); }
  int get_out_profile_type() { return out_profile_type.get_enum_value().first; }
  void set_out_profile_type( profile_type_t type ) { out_profile_type.set_enum_value( type ); }
  void set_out_profile_data( void* data, int length ) {
    out_profile_data = data;
    out_profile_data_length = length;
  }
  std::string get_out_profile_name() { return out_profile_name.get(); }

  int get_intent() { return intent.get_enum_value().first; }

  bool get_clip_negative() { return clip_negative.get(); }
  bool get_clip_overflow() { return clip_overflow.get(); }
  void set_clip_negative( bool flag ) { clip_negative.update(flag); }
  void set_clip_overflow( bool flag ) { clip_overflow.update(flag); }

  void set_bpc( bool flag ) { bpc.update( flag ); }

  void set_image_hints( VipsImage* img )
  {
    if( !img ) return;
    OpParBase::set_image_hints( img );
    //rgb_image( get_xsize(), get_ysize() );
  }

  cmsColorSpaceSignature get_input_cs_type() { return input_cs_type; }
  cmsColorSpaceSignature get_output_cs_type() { return output_cs_type; }

  bool softproof_enabled() { return softproof; }
  void set_softproof( bool s ) { softproof = s; }

  bool gamut_warning_enabled() { return gamut_warning; }
  void set_gamut_warning( bool s ) { gamut_warning = s; }

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

  bool import_settings( OpParBase* pin ) {
    ConvertColorspacePar* pin2 = dynamic_cast<ConvertColorspacePar*>( pin );
    if( pin2 ) {
      set_softproof( pin2->softproof_enabled() );
      set_gamut_warning( pin2->gamut_warning_enabled() );
    }
    return OpParBase::import_settings( pin );
  }



  //cmsHPROFILE create_profile_from_matrix (const double matrix[3][3], bool gamma, Glib::ustring name);

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap, unsigned int& level);
};



template < OP_TEMPLATE_DEF >
class ConvertColorspace
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ConvertColorspacePar* opar = dynamic_cast<ConvertColorspacePar*>(par);
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
class ConvertColorspace< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ConvertColorspacePar* opar = dynamic_cast<ConvertColorspacePar*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
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
        opar->get_input_cs_type() == cmsSigCmykData ) {
      line = new float[line_size_max];
    }
    if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
      std::cout<<"ConvertColorspace::render(): line="<<(void*)line<<std::endl;
    }

    for( y = 0; y < height; y++ ) {
      p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      if(opar->get_transform()) {
        if( opar->get_input_cs_type() == cmsSigLabData ) {
          for( x = 0; x < line_size_in; x+= 3 ) {
            line[x] = (cmsFloat32Number) (p[x] * 100.0);
            line[x+1] = (cmsFloat32Number) (p[x+1]*255.0 - 127.5);
            line[x+2] = (cmsFloat32Number) (p[x+2]*255.0 - 127.5);
            if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
              std::cout<<"ConvertColorspace::render(): from Lab: line="<<line[x]<<" "<<line[x+1]<<" "<<line[x+2]<<std::endl;
            }
          }
          cmsDoTransform( opar->get_transform(), line, pout, width );
          if( false && r->left==0 && r->top==0 && y==0 ) {
            std::cout<<"ConvertColorspace::render(): from Lab: pout="<<pout[0]*255<<" "<<pout[1]*255<<" "<<pout[2]*255<<std::endl;
          }
        } else if( opar->get_input_cs_type() == cmsSigCmykData ) {
          for( x = 0; x < line_size_in; x+= 4 ) {
            line[x] = (cmsFloat32Number) (p[x] * 100.0);
            line[x+1] = (cmsFloat32Number) (p[x+1] * 100.0);
            line[x+2] = (cmsFloat32Number) (p[x+2] * 100.0);
            line[x+3] = (cmsFloat32Number) (p[x+3] * 100.0);
            if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
              std::cout<<"ConvertColorspace::render(CMYK in): line="<<line[x]<<" "<<line[x+1]<<" "<<line[x+2]<<" "<<line[x+3]<<std::endl;
            }
          }
          cmsDoTransform( opar->get_transform(), line, pout, width );
          if( false && r->left==0 && r->top==0 && y==0 ) {
            std::cout<<"ConvertColorspace::render(CMYK in): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
          }
        } else {
          cmsDoTransform( opar->get_transform(), p, pout, width );
          if( opar->get_output_cs_type() == cmsSigLabData ) {
            if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
              std::cout<<"ConvertColorspace::render(Lab out): p=   "<<p[0]<<" "<<p[1]<<" "<<p[2]<<std::endl;
              std::cout<<"ConvertColorspace::render(Lab out): pout="<<pout[0]<<" "<<pout[1]<<" "<<pout[2]<<std::endl;
            }
          }
          if( opar->get_output_cs_type() == cmsSigLabData ) {
            for( x = 0; x < line_size_out; x+= 3 ) {
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ConvertColorspace::render(Lab out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<std::endl;
              }

              pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
              pout[x+1] = (cmsFloat32Number) ((pout[x+1] + 127.5) / 255.0);
              pout[x+2] = (cmsFloat32Number) ((pout[x+2] + 127.5) / 255.0);

              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ConvertColorspace::render(Lab out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<std::endl;
              }
            }
          }
          if( opar->get_output_cs_type() == cmsSigCmykData ) {
            for( x = 0; x < line_size_out; x+= 4 ) {
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ConvertColorspace::render(CMYK out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<pout[x+3]<<std::endl;
              }
              pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
              pout[x+1] = (cmsFloat32Number) (pout[x+1] / 100.0);
              pout[x+2] = (cmsFloat32Number) (pout[x+2] / 100.0);
              pout[x+3] = (cmsFloat32Number) (pout[x+3] / 100.0);
              if( false && r->left==0 && r->top==0 && x==0 && y==0 ) {
                std::cout<<"ConvertColorspace::render(CMYK out): pout="<<pout[x]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<pout[x+3]<<std::endl;
              }
            }
          }
        }
      } else {
        memcpy( pout, p, sizeof(float)*line_size_in );
      }
      if( opar->get_clip_negative() || opar->get_clip_overflow() ) {
        for( x = 0; x < line_size_out; x+= 1 ) {
          if( opar->get_clip_negative() ) pout[x] = MAX( pout[x], 0.f );
          if( opar->get_clip_overflow() ) pout[x] = MIN( pout[x], 1.f );
        }
      }
    }
    if( opar->get_input_cs_type() == cmsSigLabData && line ) {
      delete( line );
    }
  }
};




template < OP_TEMPLATE_DEF_TYPE_SPEC >
class ConvertColorspace< OP_TEMPLATE_IMP_TYPE_SPEC(double) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ConvertColorspacePar* opar = dynamic_cast<ConvertColorspacePar*>(par);
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
            line[x+1] = (cmsFloat64Number) (pin[x+1]*255.0 - 128.0);
            line[x+2] = (cmsFloat64Number) (pin[x+2]*255.0 - 128.0);
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




ProcessorBase* new_gamut_warning();

ProcessorBase* new_convert_colorspace();
}

#endif 


