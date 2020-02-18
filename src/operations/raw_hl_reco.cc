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

#include "../base/processor_imp.hh"

#include "raw_hl_reco.hh"

using namespace PF;


PF::RawHLRecoPar::RawHLRecoPar(): OpParBase()
{
  set_type("raw_hl_reco" );
}


VipsImage* PF::RawHLRecoPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (int)in.size() < first+1 )
    return NULL;

  VipsImage* image = in[first];
  if( !image )
  set_image_hints( image );

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  return out;
}





template < OP_TEMPLATE_DEF >
class RawHLRecoProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
                  VipsRegion* imap, VipsRegion* omap,
                  VipsRegion* oreg, OpParBase* par)
  {
  }
};

template < OP_TEMPLATE_DEF_TYPE_SPEC >
class RawHLRecoProc< OP_TEMPLATE_IMP_TYPE_SPEC(float) >
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
    PF::RawHLRecoPar* opar = dynamic_cast<PF::RawHLRecoPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;

    if( false && r->top==0 && r->left==0 ) {
      std::cout<<"RawHLReco::render(): ireg[in_first]->im->Bands="<<ireg[in_first]->im->Bands
               <<"  oreg->im->Bands="<<oreg->im->Bands<<std::endl;
      std::cout<<"RawHLReco::render(): ireg[in_first]->im->BandFmt="<<ireg[in_first]->im->BandFmt
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
    float sat_min = 1;
    float mul_corr = 1;
    if( opar->get_hlreco_mode() != HLRECO_CLIP ) {
      sat_min = min_mul/max_mul;
      mul_corr = max_mul/min_mul;
    }
    for( int i = 0; i < 3; i++ ) {
      satcorr[i] = sat[i]*mul_corr;
    }

    float* pin;
    float* pout;
    int x, y;

    //std::cout<<"[RawHLRecoProc::render] mode="<<opar->get_hlreco_mode()<<std::endl;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      switch( opar->get_hlreco_mode() ) {
      case HLRECO_BLEND: {
        //std::cout<<"[RawHLRecoProc::render] using blend HL reco mode"<<std::endl;
        for( x = 0; x < line_size; x+=3 ) {
          pout[x] = pin[x]*mul_corr;
          pout[x+1] = pin[x+1]*mul_corr;
          pout[x+2] = pin[x+2]*mul_corr;
        }
        //HLRecovery_blend( line, width, 1.0f, sat );
        HLRecovery_blend( pout, y, width, 1.f, satcorr );
        break;
      }
      case HLRECO_CLIP: {
        for( x = 0; x < line_size; x+=3 ) {
          pout[x] = PF_CLIP( pin[x], sat_min )*mul_corr;
          pout[x+1] = PF_CLIP( pin[x+1], sat_min )*mul_corr;
          pout[x+2] = PF_CLIP( pin[x+2], sat_min )*mul_corr;
        }
        break;
      }
      default: {
        for( x = 0; x < line_size; x+=3 ) {
          pout[x] = pin[x]*mul_corr;
          pout[x+1] = pin[x+1]*mul_corr;
          pout[x+2] = pin[x+2]*mul_corr;
        }
      }
      }
      //std::cout<<"[RawHLRecoProc::render] pin[0]="<<pin[0]<<"  pout[0]="<<pout[0]<<std::endl;
    }
  }
};





PF::ProcessorBase* PF::new_raw_hl_reco()
{
  return new PF::Processor<PF::RawHLRecoPar,RawHLRecoProc>();
}
