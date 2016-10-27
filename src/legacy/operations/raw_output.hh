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

#ifndef RAW_OUTPUT_V1_H
#define RAW_OUTPUT_V1_H

#include <string>

#include <glibmm.h>

//#include <libraw/libraw.h>

#include "../../base/processor.hh"

//#include "../rt/iccmatrices.hh"
#include "../../dt/common/srgb_tone_curve_values.h"

#include "../../operations/raw_image.hh"

//#define CLIPRAW(a) ((a)>0.0?((a)<1.0?(a):1.0):0.0)
#define CLIPRAW(a) (a)

//#define CLIPOUT(a) ((a)>0.0?((a)<1.0?(a):1.0):0.0)
#define CLIPOUT(a) (a)

#define PF_CLIP(a,MAX)  ((a)<(MAX)?(a):(MAX))
#define PF_CLAMP(a,MIN,MAX)  ((a)>(MIN)?((a)<(MAX)?(a):(MAX)):(MIN))

namespace PF 
{

enum exposure_mode_t {
  EXP_NORMAL,
  EXP_AUTO
};


enum hlreco_mode_t {
  HLRECO_NONE,
  HLRECO_CLIP,
  HLRECO_BLEND
};


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

  class RawOutputV1Par: public OpParBase
  {
    dcraw_data_t* image_data;

    // Conversion matrix from camera colorspace to xyz
    // Initialized as xyz_sRGB * rgb_cam
    double xyz_cam[3][3];

    Property<float> exposure;
    PropertyBase exposure_mode;
    Property<float> exposure_clip_amount;
    PropertyBase hlreco_mode;

    float wb_red_current, wb_green_current, wb_blue_current, exposure_current;

    PropertyBase profile_mode;
    input_profile_mode_t current_profile_mode;

    cmsToneCurve* srgb_curve;
    cmsToneCurve* gamma_curve;

    // Camera input color profile
    // Either from xyz_cam or from icc 
    Property<std::string> cam_profile_name;
    std::string current_cam_profile_name;
    //cmsHPROFILE cam_profile;
    PF::ICCProfile* cam_profile;

    // Input gamma 
    PropertyBase gamma_mode;
    Property<float> gamma_lin;
    Property<float> gamma_exp;

    // output color profile
    PropertyBase out_profile_mode;
    //output_profile_mode_t current_out_profile_mode;
    Property<std::string> out_profile_name;
    std::string current_out_profile_name;
    //cmsHPROFILE out_profile;
    PF::ICCProfile* out_profile;

    cmsHTRANSFORM transform;

  public:

    RawOutputV1Par();

    float get_wb_red() { return wb_red_current; /*wb_red.get();*/ }
    float get_wb_green() { return wb_green_current; /*wb_green.get();*/ }
    float get_wb_blue() { return wb_blue_current; /*wb_blue.get();*/ }

    void set_wb(float r, float g, float b) {
      wb_red_current = r;
      wb_green_current = g;
      wb_blue_current = b;
      std::cout<<"RawPreprocessorPar: setting WB coefficients to "<<r<<","<<g<<","<<b<<std::endl;
    }

    float get_exposure() { return exposure.get(); }

    hlreco_mode_t get_hlreco_mode() { return (hlreco_mode_t)hlreco_mode.get_enum_value().first; }

    input_profile_mode_t get_camera_profile_mode() { return (input_profile_mode_t)profile_mode.get_enum_value().first; }
    input_gamma_mode_t get_gamma_mode() { return (input_gamma_mode_t)gamma_mode.get_enum_value().first; }
    cmsToneCurve* get_gamma_curve() { return gamma_curve; }
    cmsToneCurve* get_srgb_curve() { return srgb_curve; }
    cmsHTRANSFORM get_transform() { return transform; }

    void set_image_hints( VipsImage* img );

    int get_out_profile_mode() { return out_profile_mode.get_enum_value().first; }

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
  class RawOutputV1
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                    VipsRegion* imap, VipsRegion* omap,
                    VipsRegion* oreg, OpParBase* par)
    {
    }
  };

  template < OP_TEMPLATE_DEF_TYPE_SPEC >
  class RawOutputV1< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
  {

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // derived from Dcraw "blend_highlights()"
    //  very effective to reduce (or remove) the magenta, but with levels of grey !
    void HLRecovery_blend(float* in, int row, int width, float maxval, float* hlmax)
    {
        const int ColorCount = 3;

        // Transform matrixes rgb>lab and back
        static const float trans[2][ColorCount][ColorCount] = {
            { { 1, 1, 1 }, { 1.7320508, -1.7320508, 0 }, { -1, -1, 2 } },
            { { 1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 } }
        };
        static const float itrans[2][ColorCount][ColorCount] = {
            { { 1, 0.8660254, -0.5 }, { 1, -0.8660254, -0.5 }, { 1, 0, 1 } },
            { { 1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 } }
        };

    #define FOREACHCOLOR for (int c=0; c < ColorCount; c++)

        //std::cout<<"width="<<width<<"  hlmax="<<hlmax[0]<<","<<hlmax[1]<<","<<hlmax[2]<<std::endl;

        float minpt = min(hlmax[0], hlmax[1], hlmax[2]); //min of the raw clip points
        //float maxpt=max(hlmax[0],hlmax[1],hlmax[2]);//max of the raw clip points
        //float medpt=hlmax[0]+hlmax[1]+hlmax[2]-minpt-maxpt;//median of the raw clip points
        float maxave = (hlmax[0] + hlmax[1] + hlmax[2]) / 3; //ave of the raw clip points
        //some thresholds:
        const float clipthresh = 0.95;
        const float fixthresh = 0.5;
        const float satthresh = 0.5;

        int line_size = width * ColorCount;

        float clip[3];
        FOREACHCOLOR clip[c] = min(maxave, hlmax[c]);

        // Determine the maximum level (clip) of all channels
        const float clippt = clipthresh * maxval;
        const float fixpt = fixthresh * minpt;
        const float desatpt = satthresh * maxave + (1 - satthresh) * maxval;


        //for (int col = 0; col < width; col++) {
        for (int col = 0; col < line_size; col+=ColorCount) {
            float rgb[ColorCount], cam[2][ColorCount], lab[2][ColorCount], sum[2], chratio, lratio = 0;
            float L, C, H, Lfrac;

            //std::cout<<"in["<<row<<"]["<<col/ColorCount<<"]="<<in[col+0]*65535.f<<","<<in[col+1]*65535.f<<","<<in[col+2]*65535.f<<std::endl;
            //std::cout<<"clippt="<<clippt*65535.f<<std::endl;
            // Copy input pixel to rgb so it's easier to access in loops
            rgb[0] = in[col];
            rgb[1] = in[col+1];
            rgb[2] = in[col+2];

            // If no channel is clipped, do nothing on pixel
            int c;

            for (c = 0; c < ColorCount; c++) {
                if (rgb[c] > clippt) {
                    break;
                }
            }

            if (c == ColorCount) {
                continue;
            }

            //std::cout<<"found clipped pixel:  minpt="<<minpt<<"  maxave="<<maxave<<"  clipt="<<clippt<<std::endl;
            //std::cout<<"hlmax="<<hlmax[0]<<","<<hlmax[1]<<","<<hlmax[2]<<std::endl;
            //std::cout<<"rgb="<<rgb[0]<<","<<rgb[1]<<","<<rgb[2]<<std::endl;

            // Initialize cam with raw input [0] and potentially clipped input [1]
            FOREACHCOLOR {
                lratio += min(rgb[c], clip[c]);
                cam[0][c] = rgb[c];
                cam[1][c] = min(cam[0][c], maxval);
            }

            // Calculate the lightness correction ratio (chratio)
            for (int i = 0; i < 2; i++) {
                FOREACHCOLOR {
                    lab[i][c] = 0;

                    for (int j = 0; j < ColorCount; j++)
                    {
                        lab[i][c] += trans[ColorCount - 3][c][j] * cam[i][j];
                    }
                }

                sum[i] = 0;

                for (int c = 1; c < ColorCount; c++) {
                    sum[i] += SQR(lab[i][c]);
                }
            }

            chratio = (sqrt(sum[1] / sum[0]));

            // Apply ratio to lightness in LCH space
            for (int c = 1; c < ColorCount; c++) {
                lab[0][c] *= chratio;
            }

            // Transform back from LCH to RGB
            FOREACHCOLOR {
                cam[0][c] = 0;

                for (int j = 0; j < ColorCount; j++)
                {
                    cam[0][c] += itrans[ColorCount - 3][c][j] * lab[0][j];
                }
            }
            FOREACHCOLOR rgb[c] = cam[0][c] / ColorCount;

            // Copy converted pixel back
            if (in[col] > fixpt) {
                float rfrac = SQR((min(clip[0], in[col]) - fixpt) / (clip[0] - fixpt));
                in[col] = min(maxave, rfrac * rgb[0] + (1 - rfrac) * in[col]);
            }

            if (in[col+1] > fixpt) {
                float gfrac = SQR((min(clip[1], in[col+1]) - fixpt) / (clip[1] - fixpt));
                in[col+1] = min(maxave, gfrac * rgb[1] + (1 - gfrac) * in[col+1]);
            }

            if (in[col+2] > fixpt) {
                float bfrac = SQR((min(clip[2], in[col+2]) - fixpt) / (clip[2] - fixpt));
                in[col+2] = min(maxave, bfrac * rgb[2] + (1 - bfrac) * in[col+2]);
            }

            lratio /= (in[col] + in[col+1] + in[col+2]);
            L = (in[col] + in[col+1] + in[col+2]) / 3;
            C = lratio * 1.732050808 * (in[col] - in[col+1]);
            H = lratio * (2 * in[col+2] - in[col] - in[col+1]);
            in[col] = L - H / 6.0 + C / 3.464101615;
            in[col+1] = L - H / 6.0 - C / 3.464101615;
            in[col+2] = L + H / 3.0;

            if ((L = (in[col] + in[col+1] + in[col+2]) / 3) > desatpt) {
                Lfrac = max(0.0f, (maxave - L) / (maxave - desatpt));
                C = Lfrac * 1.732050808 * (in[col] - in[col+1]);
                H = Lfrac * (2 * in[col+2] - in[col] - in[col+1]);
                in[col] = L - H / 6.0 + C / 3.464101615;
                in[col+1] = L - H / 6.0 - C / 3.464101615;
                in[col+2] = L + H / 3.0;
            }
            //std::cout<<"out="<<in[col+0]*65535.f<<","<<in[col+1]*65535.f<<","<<in[col+2]*65535.f<<std::endl<<std::endl;
        }
    }


  public: 
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, OpParBase* par)
    {
      RawOutputV1Par* opar = dynamic_cast<RawOutputV1Par*>(par);
      if( !opar ) return;
      float exposure = opar->get_exposure();
      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      int width = r->width;
      int height = r->height;

      cmsToneCurve* srgb_curve = opar->get_srgb_curve();
      cmsToneCurve* gamma_curve = opar->get_gamma_curve();

      if( false && r->top==0 && r->left==0 ) {
        std::cout<<"RawOutputV1::render(): ireg[in_first]->im->Bands="<<ireg[in_first]->im->Bands
                 <<"  oreg->im->Bands="<<oreg->im->Bands<<std::endl;
        std::cout<<"RawOutputV1::render(): ireg[in_first]->im->BandFmt="<<ireg[in_first]->im->BandFmt
                 <<"  oreg->im->BandFmt="<<oreg->im->BandFmt<<std::endl;
      }
    
      float mul[3] = {
        opar->get_wb_red(),
        opar->get_wb_green(),
        opar->get_wb_blue()
      };
      float sat[3], satcorr[3];
      float min_mul = mul[0];
      float max_mul = mul[0];
      for( int i = 1; i < 3; i++ ) {
        if( mul[i] < min_mul ) min_mul = mul[i];
        if( mul[i] > max_mul ) max_mul = mul[i];
      }
      for( int i = 0; i < 3; i++ ) {
        mul[i] /= max_mul;
        sat[i] = mul[i];
      }
      float sat_min = min_mul/max_mul;
      float mul_corr = max_mul/min_mul;
      //exposure = exposure * mul_corr;
      for( int i = 0; i < 3; i++ ) {
        satcorr[i] = sat[i]*mul_corr;
      }

      float* p;
      float* pin;
      float* pout;
      int x, y;

      float* line = new float[line_size];
      float* line2 = new float[line_size];

      //std::cout<<"opar->get_transform(): "<<opar->get_transform()<<std::endl;
    
      /*
      float max[3] = {0,0,0};
      for( y = 0; y < height; y++ ) {
        p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        for( x = 0; x < width; x+=3 ) {
          if(p[x] > max[0]) max[0] = p[x];
          if(p[x+1] > max[1]) max[1] = p[x+1];
          if(p[x+2] > max[2]) max[2] = p[x+2];
        }
      }
      std::cout<<"("<<r->left<<","<<r->top<<"): max = "<<max[0]<<"  "<<max[1]<<"  "<<max[2]<<std::endl;
      */

      for( y = 0; y < height; y++ ) {
        p = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
        pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        if( false && r->top==0 && r->left==0 ) {
          std::cout<<"RawOutputV1::render(): camera_profile_mode="<<opar->get_camera_profile_mode()<<std::endl;
        }

        switch( opar->get_hlreco_mode() ) {
        case HLRECO_BLEND: {
          for( x = 0; x < line_size; x+=3 ) {
            line[x] = p[x]*mul_corr;
            line[x+1] = p[x+1]*mul_corr;
            line[x+2] = p[x+2]*mul_corr;
          }
          //HLRecovery_blend( line, width, 1.0f, sat );
          HLRecovery_blend( line, y, width, 1.f, satcorr );
          for( x = 0; x < line_size; x+=3 ) {
            //line[x] = PF_CLIP( line[x], 1.f )*exposure;
            //line[x+1] = PF_CLIP( line[x+1], 1.f )*exposure;
            //line[x+2] = PF_CLIP( line[x+2], 1.f )*exposure;
            line[x] = line[x]*exposure;
            line[x+1] = line[x+1]*exposure;
            line[x+2] = line[x+2]*exposure;
            //std::cout<<"line["<<x<<"]="<<line[x]<<","<<line[x+1]<<","<<line[x+2]<<std::endl;
          }
          break;
        }
        case HLRECO_CLIP: {
          for( x = 0; x < line_size; x+=3 ) {
            line[x] = PF_CLIP( p[x], sat_min )*mul_corr*exposure;
            line[x+1] = PF_CLIP( p[x+1], sat_min )*mul_corr*exposure;
            line[x+2] = PF_CLIP( p[x+2], sat_min )*mul_corr*exposure;
          }
          break;
        }
        default: {
          for( x = 0; x < line_size; x+=3 ) {
            line[x] = p[x]*mul_corr*exposure;
            line[x+1] = p[x+1]*mul_corr*exposure;
            line[x+2] = p[x+2]*mul_corr*exposure;
          }
        }
        }


        if( opar->get_camera_profile_mode() == IN_PROF_ICC ) {

          if( opar->get_gamma_mode() == IN_GAMMA_sRGB ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( srgb_curve, line[x] );
            }
          } else if( opar->get_gamma_mode() == IN_GAMMA_CUSTOM ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( gamma_curve, line[x] );
            }
          }

          if(opar->get_transform()) {
            for( int xi = 0; xi < line_size; xi++ ) {
              line2[xi] = CLIPRAW(line[xi]);
            }
            cmsDoTransform( opar->get_transform(), line2, pout, width );
            if(false && r->left<20 && r->top<20 && y==0) {
              std::cout<<"in: "<<line[0]<<","<<line[1]<<","<<line[2]<<"   ";
              std::cout<<"out: "<<pout[0]<<","<<pout[1]<<","<<pout[2]<<"   "<<std::endl;
            }

            if( opar->get_out_profile_mode() == OUT_PROF_LAB ) {
              for( x = 0; x < line_size; x+= 3 ) {
                pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
                pout[x+1] = (cmsFloat32Number) ((pout[x+1] + 128.0) / 256.0);
                pout[x+2] = (cmsFloat32Number) ((pout[x+2] + 128.0) / 256.0);
              }
            }
          } else {
            memcpy( pout, line, sizeof(float)*line_size );
            for( int xi = 0; xi < line_size; xi++ ) {
              pout[xi] = CLIPRAW(pout[xi]*exposure);
            }
          }

        } else if( opar->get_camera_profile_mode() == IN_PROF_MATRIX ) {
          if(opar->get_transform()) {
            for( int xi = 0; xi < line_size; xi++ ) {
              line2[xi] = CLIPRAW(line[xi]);
            }
            cmsDoTransform( opar->get_transform(), line2, pout, width );
            //std::cout<<"cmsDoTransform(): in="<<line2[0]<<","<<line2[1]<<","<<line2[2]<<" -> out="
            //    <<pout[0]<<","<<pout[1]<<","<<pout[2]<<std::endl;

            if( opar->get_out_profile_mode() == OUT_PROF_LAB ) {
              for( x = 0; x < line_size; x+= 3 ) {
                pout[x] = (cmsFloat32Number) (pout[x] / 100.0);
                pout[x+1] = (cmsFloat32Number) ((pout[x+1] + 128.0) / 256.0);
                pout[x+2] = (cmsFloat32Number) ((pout[x+2] + 128.0) / 256.0);
              }
            }
          } else {
            memcpy( pout, line, sizeof(float)*line_size );
          }
        } else {

          if( false && r->top==0 && r->left==0 ) {
            std::cout<<"RawOutputV1::render(): gamma_mode="<<opar->get_gamma_mode()<<std::endl;
          }
          if( opar->get_gamma_mode() == IN_GAMMA_sRGB ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( srgb_curve, line[x] );
            }
          } else if( opar->get_gamma_mode() == IN_GAMMA_CUSTOM ) {
            for( x = 0; x < line_size; x++ ) {
              line[x] = cmsEvalToneCurveFloat( gamma_curve, line[x] );
            }
          }
          memcpy( pout, line, sizeof(float)*line_size );
        }
        for( int xi = 0; xi < line_size; xi++ ) {
          //if(pout[xi] > 1 || pout[xi] < 0)
          //  std::cout<<"RGB_out["<<xi%3<<"]="<<pout[xi]<<std::endl;
          pout[xi] = CLIPOUT(pout[xi]);
        }
        for( int xi = 0; xi < line_size; xi+=3 ) {
          if(true && r->left<20 && r->top<20 && y<4 && xi<12) {
            std::cout<<"("<<y<<","<<xi/3<<")  "<<pout[xi]<<","<<pout[xi+1]<<","<<pout[xi+2]<<"   "<<std::endl;
          }
        }
      }
      delete line; delete line2;
    }
  };




  ProcessorBase* new_raw_output_v1();
}

#endif 


