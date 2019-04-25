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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/processor_imp.hh"
#include "guided_filter.hh"
#include "sharpen.hh"
#include "shadows_highlights_v2.hh"


#define MEDIAN_RADIUS_SCALE 5.0f
#define GUIDED_RADIUS_SCALE 1.0f
#define GUIDED_THRESHOLD_SCALE 0.1f


class LogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
public:
  LogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("LogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("LogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("LogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("LogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class LogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class LogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    LogLumiPar* opar = dynamic_cast<LogLumiPar*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    const float bias = 1.0f/profile->perceptual2linear(opar->get_anchor());

    float L, pL;
    float* pin;
    float* pout;
    int x, y, R;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < width; x++, pin+=3, pout++ ) {
        L = profile->get_lightness(pin[0], pin[1], pin[2]);
        L *= bias;
        pL = (L>1.0e-16) ? xlog10( L ) : -16;

        pout[0] = pL;
      }
    }
  }
};





class MedianSmoothingPar: public PF::OpParBase
{
  bool enabled;
  float threshold, gain, exponent;
public:
  MedianSmoothingPar():
    PF::OpParBase() {

  }

  void set_enabled(bool e) { enabled = e; }
  bool get_enabled() { return enabled; }
  void set_threshold(float t) { threshold = t; }
  float get_threshold() { return threshold; }
  void set_gain(float g) { gain = g; }
  float get_gain() { return gain; }
  void set_exponent(float e) { exponent = e; }
  float get_exponent() { return exponent; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("LogLumiPar::build(): in.empty()\n"); return NULL;}
    VipsImage* image = in[0];
    if( !image ) {printf("LogLumiPar::build(): image==NULL\n"); return NULL;}
    grayscale_image( get_xsize(), get_ysize() );
    VipsImage* out = PF::OpParBase::build( in, first, NULL, NULL, level );
    return out;
  }
};


template < OP_TEMPLATE_DEF >
class MedianSmoothingProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class MedianSmoothingProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    MedianSmoothingPar* opar = dynamic_cast<MedianSmoothingPar*>(par);
    if( !opar ) return;
    Rect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;

    const float SH_LOG_SCALE_RANGE = SH_LOG_SCALE_MAX - SH_LOG_SCALE_MIN;
    //const float bias = profile->perceptual2linear(0.5);

    float* pin1;
    float* pin2;
    float* pout;
    int x, y;

    for( y = 0; y < height; y++ ) {
      // input image
      //float* pin0_y2m = (float*)VIPS_REGION_ADDR( ireg[2], r->left-2, r->top + y - 2 );
      //float* pin0 = (float*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
      //float* pin0_y2p = (float*)VIPS_REGION_ADDR( ireg[2], r->left-2, r->top + y + 2 );
      // log-lumi image
      pin1 = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      // blurred log-lumi image
      pin2 = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      // output image
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < width; x++, pin1++, pin2++, pout++ ) {
        float l2 = pin2[0];
        if( opar->get_enabled() ) {
          //l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
          //l2 = xexp10( l2 );
          //l2 *= bias;
          //l2 = floorf(l2 * 255) / 255.0f;
          float l1 = pin1[0];
          //l1 = ( l1*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
          //l1 = xexp10( l1 );
          //l1 *= bias;
          float diff = l1 - l2;
          //std::cout<<"l1="<<l1<<"  l2="<<l2<<"  diff="<<diff<<std::endl;
          float diff_gain = opar->get_gain();// / opar->get_threshold();
          diff *= diff_gain;
          if( std::fabs(diff) < 1.0f ) {
            float exp_max = opar->get_exponent();
            float exponent = (exp_max - 1) * (1.0f - std::fabs(diff)) + 1.0f;
            if(diff >= 0) {
              diff = pow(diff, exponent);
            } else {
              diff = -1.0f * pow(-1.0f * diff, exponent);
            }
          }
          diff /= diff_gain;
          l2 += diff;
          //std::cout<<"diff(2)="<<diff<<"  l2(2)="<<l2<<std::endl;
        }

        l2 = ( l2*SH_LOG_SCALE_RANGE ) + SH_LOG_SCALE_MIN;
        pout[0] = xexp10(l2);
      }
    }
  }
};





PF::ShadowsHighlightsV2Par::ShadowsHighlightsV2Par():
OpParBase(),
amount("amount",this,pow(10, 0.5)),
shadows("shadows",this,pow(10, 0.5)),
shadows_range("shadows_range",this,5),
highlights("highlights",this,pow(10, 0.5)),
highlights_range("highlights_range",this,5),
anchor("anchor",this,0.5),
radius("sh_radius",this,100),
threshold("sh_threshold",this,0.2),
show_residual("show_residual", this, false),
in_profile( NULL )
{
  loglumi = new PF::Processor<LogLumiPar,LogLumiProc>();
  int ts = 1;
  for(int gi = 0; gi < 10; gi++) {
    guided[gi] = new_guided_filter();
    threshold_scale[gi] = ts;
    ts *= 2;
  }
  int rs = 1;
  for(int gi = 9; gi >= 0; gi--) {
    radius_scale[gi] = rs;
    rs *= 4;
  }

  usm = new_sharpen();

  set_type("shadows_highlights_v2" );

  set_default_name( _("shadows/highlights") );
}


bool PF::ShadowsHighlightsV2Par::needs_caching()
{
  return true;
}


static void fill_rv(int* rv, float radius, int level)
{
  int gi = 0;
  for(gi = 0; gi < 10; gi++) rv[gi] = 0;

  int r = 1;
  for( unsigned int l = 1; l <= level; l++ )
    r *= 2;

  for(gi = 0; gi < 9; gi++) {
    rv[gi] = r;
    if( (r*4) > radius ) break;
    r *= 4;
  }
  rv[gi] = radius;
}



void PF::ShadowsHighlightsV2Par::propagate_settings()
{
  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    guidedpar->propagate_settings();
  }
}


void PF::ShadowsHighlightsV2Par::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  int rv[10] = { 0 };
  fill_rv(rv, radius.get(), level);

  int tot_padding = 0;
  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    std::cout<<"ShadowsHighlightsV2Par::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
    if(rv[gi] == 0) break;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold( threshold.get() / threshold_scale[gi] );
    int subsampling = 1;
    while( subsampling <= 16 ) {
      if( (subsampling*8) >= rv[gi] ) break;
      subsampling *= 2;
    }
    guidedpar->set_subsampling(subsampling);
    guidedpar->propagate_settings();
    guidedpar->compute_padding(full_res, id, level);
    tot_padding += guidedpar->get_padding(id);
  }
/*
  PF::SharpenPar* usmpar = dynamic_cast<PF::SharpenPar*>( usm->get_par() );
  if( usmpar ) {
    usmpar->set_usm_radius(4);
    usmpar->propagate_settings();
    usmpar->compute_padding(full_res, id, level);
    tot_padding += usmpar->get_padding(id);
  }
*/
  set_padding( tot_padding, id );
}




VipsImage* PF::ShadowsHighlightsV2Par::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  std::vector<VipsImage*> in2;

  if( (get_render_mode() == PF_RENDER_PREVIEW) /*&& is_editing()*/ )
    show_residual_ = show_residual.get();
  else
    show_residual_ = false;

  in_profile = PF::get_icc_profile( in[0] );


  LogLumiPar* logpar = dynamic_cast<LogLumiPar*>( loglumi->get_par() );
  std::cout<<"DynamicRangeCompressorV2Par::build(): logpar="<<logpar<<std::endl;
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( anchor.get() );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  } else {
    logimg = in[0];
    PF_REF(logimg, "DynamicRangeCompressorV2Par::build: in[0] ref");
  }

  std::cout<<"DynamicRangeCompressorV2Par::build(): logimg="<<logimg<<std::endl;

  if( !logimg ) return NULL;




  //std::cout<<"DynamicRangeCompressorV2Par::build(): ts="<<ts<<std::endl;


  int rv[10] = { 0 };
  fill_rv(rv, radius.get(), level);

  VipsImage* timg = logimg;
  VipsImage* smoothed = logimg;
  std::cout<<"ShadowsHighlightsV2Par::build(): radius="<<radius.get()<<std::endl;
  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( timg );
    guidedpar->set_format( get_format() );
    //std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="<<rv[gi]<<std::endl;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(threshold.get() / threshold_scale[gi]);
    int subsampling = 1;
    while( subsampling <= 16 ) {
      if( (subsampling*8) >= rv[gi] ) break;
      subsampling *= 2;
    }
    guidedpar->set_subsampling(subsampling);
    guidedpar->set_convert_to_perceptual( false );
    guidedpar->propagate_settings();
    guidedpar->compute_padding(timg, 0, level);

    std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="<<rv[gi]<<"  padding="<<guidedpar->get_padding(0)<<std::endl;
    if( guidedpar->get_padding(0) > 64 ) {
      int ts = 128;
      int tr = guidedpar->get_padding(0) / ts + 1;
      int nt = (timg->Xsize/ts) * tr + 1;
      VipsAccess acc = VIPS_ACCESS_RANDOM;
      int threaded = 1, persistent = 0;
      VipsImage* cached = NULL;
      if( phf_tilecache(timg, &cached,
          "tile_width", ts,
          "tile_height", ts,
          "max_tiles", nt,
          "access", acc, "threaded", threaded,
          "persistent", persistent, NULL) ) {
        std::cout<<"DynamicRangeCompressorV2Par::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( timg, "DynamicRangeCompressorV2Par::build(): cropped unref" );
      timg = cached;
    }

    in2.clear();
    in2.push_back( timg );
    smoothed = guidedpar->build( in2, first, NULL, NULL, level );
    if( !smoothed ) {
      std::cout<<"DynamicRangeCompressorV2Par::build(): NULL local contrast enhanced image"<<std::endl;
      return NULL;
    }
    PF_UNREF(timg, "DynamicRangeCompressorV2Par::build(): timg unref");
    timg = smoothed;
  }

/*
  timg = smoothed;
  PF::SharpenPar* usmpar = dynamic_cast<PF::SharpenPar*>( usm->get_par() );
  if( usmpar ) {
    in2.clear();
    in2.push_back( timg );
    usmpar->set_image_hints( timg );
    usmpar->set_format( get_format() );
    usmpar->set_usm_radius(4);
    usmpar->propagate_settings();
    smoothed = usmpar->build( in2, first, NULL, NULL, level );
    if( !smoothed ) {
      std::cout<<"DynamicRangeCompressorV2Par::build(): NULL local contrast enhanced image"<<std::endl;
      return NULL;
    }
    PF_UNREF(timg, "DynamicRangeCompressorV2Par::build(): timg unref");
  }
*/


  if( !smoothed ) {
    std::cout<<"ShadowsHighlightsV2Par::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "ShadowsHighlightsV2Par::build(): null Lab image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed);
  //in2.push_back(cached);
  in2.push_back(in[0]);
  VipsImage* shahi = OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( smoothed, "ShadowsHighlightsV2Par::build() smoothed unref" );
  set_image_hints( in[0] );

  return shahi;
}



PF::ProcessorBase* PF::new_shadows_highlights_v2()
{ return new PF::Processor<PF::ShadowsHighlightsV2Par,PF::ShadowsHighlightsV2Proc>(); }
