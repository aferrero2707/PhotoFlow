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

#include "icc_transform.hh"
#include "../base/processor_imp.hh"



PF::ICCTransformPar::ICCTransformPar():
  OpParBase(),
  out_profile( NULL ),
  intent( INTENT_RELATIVE_COLORIMETRIC ),
  bpc( true ),
  adaptation_state(-1),
  input_cs_type( cmsSigRgbData ),
  output_cs_type( cmsSigRgbData ),
  clip_negative(false),
  clip_overflow(false),
  gamut_mapping(false),
  saturation_intent(0)
{
  do_Lab = true; do_LCh = do_LSh = false;
  set_type("icc_transform" );

  set_default_name( _("ICC transform") );
}


VipsImage* PF::ICCTransformPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  if( (int)in.size() < first+1 ) {
    return NULL;
  }

  VipsImage* image = in[first];
  if( !image ) {
    return NULL;
  }

  void *data;
  size_t data_length;
  
  in_profile = PF::get_icc_profile( in[first] );

  if( !in_profile || !out_profile ) {
    PF_REF( in[first], "ICCTransformPar::build(): input image ref for missing input or output profiles" );
    std::cout<<"ICCTransformPar::build(): missing input or output profiles, no transform needed"<<std::endl;
    return in[first];
  }


  bool matching = false;
  if( in_profile && out_profile && in_profile->equals_to(out_profile) ) {
    matching = true;
  }

  if( matching ) {
    PF_REF( in[first], "ICCTransformPar::build(): input image ref for equal input and output profiles" );
    //std::cout<<"ICCTransformPar::build(): matching input and output profiles, no transform needed"<<std::endl;
    return in[first];
  }

  if( gamut_mapping )
    out_profile->init_gamut_mapping();


  //std::cout<<"ICCTransformPar::build(): image="<<in[0]<<" data="<<data<<" data_length="<<data_length<<std::endl;

  bool in_changed = false;
  if( in_profile && in_profile->get_profile() ) {
    char tstr[1024];
    cmsGetProfileInfoASCII(in_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
#ifndef NDEBUG
    std::cout<<"icc_transform: embedded profile: "<<in_profile<<std::endl;
    std::cout<<"icc_transform: embedded profile name: "<<tstr<<std::endl;
#endif
    
    if( in_profile_name != tstr ) {
      in_changed = true;
    }

    input_cs_type = cmsGetColorSpace( in_profile->get_profile() );
  }

#ifndef NDEBUG
  if( out_profile )
    std::cout<<"icc_transform: out_profile="<<out_profile<<" ("<<out_profile->get_profile()<<")"<<std::endl;
#endif

  if( in_profile && out_profile && out_profile->get_profile() ) {
    transform.init( in_profile, out_profile, in[0]->BandFmt, intent, get_bpc(), adaptation_state );
  }

  if( out_profile && out_profile->get_profile() ) {
#ifndef NDEBUG
    std::cout<<"icc_transform: output profile: "<<out_profile<<std::endl;
    char tstr[1024];
    cmsGetProfileInfoASCII(out_profile->get_profile(), cmsInfoDescription, "en", "US", tstr, 1024);
    std::cout<<"icc_transform: output profile: "<<tstr<<std::endl;
#endif
    output_cs_type = cmsGetColorSpace( out_profile->get_profile() );
    switch( output_cs_type ) {
    case cmsSigGrayData:
      grayscale_image( get_xsize(), get_ysize() );
      break;
    case cmsSigRgbData:
      rgb_image( get_xsize(), get_ysize() );
      break;
    case cmsSigLabData:
      lab_image( get_xsize(), get_ysize() );
      break;
    case cmsSigCmykData:
      cmyk_image( get_xsize(), get_ysize() );
      break;
    default:
      break;
    }
  }

  //if( in_profile )  cmsCloseProfile( in_profile );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );

  if( out && out_profile ) {
    PF::set_icc_profile( out, out_profile );
  }

  return out;
}


using namespace PF;


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



PF::ProcessorBase* PF::new_icc_transform()
{
  return new PF::Processor<PF::ICCTransformPar,ICCTransformProc>();
}
