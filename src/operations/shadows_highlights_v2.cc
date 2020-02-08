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
#include "median_filter.hh"
#include "guided_filter.hh"
#include "sharpen.hh"
#include "shadows_highlights_v2.hh"


#define MEDIAN_RADIUS_SCALE 5.0f
#define GUIDED_RADIUS_SCALE 1.0f
#define GUIDED_THRESHOLD_SCALE 0.1f


class ShaHiLogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
public:
  ShaHiLogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("ShaHiLogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("ShaHiLogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("ShaHiLogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("LogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class ShaHiLogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class ShaHiLogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    ShaHiLogLumiPar* opar = dynamic_cast<ShaHiLogLumiPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    const float bias = 1.0f/profile->perceptual2linear(opar->get_anchor());

#ifndef NDEBUG
    std::cout<<"[ShaHiLogLumi]:\n  ireg[0] = "<<ireg[0]<<" -> ("<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<") x ("<<ireg[0]->valid.width<<","<<ireg[0]->valid.height<<")\n";
    std::cout<<"  oreg =    "<<oreg<<" -> ("<<oreg->valid.left<<","<<oreg->valid.top<<") x ("<<oreg->valid.width<<","<<oreg->valid.height<<")\n";
    std::cout<<"  pin[0] =  "<<(void*)VIPS_REGION_ADDR(ireg[0], ireg[0]->valid.left, ireg[0]->valid.top)
        <<" -> "<<*((float*)VIPS_REGION_ADDR(ireg[0], ireg[0]->valid.left, ireg[0]->valid.top))<<std::endl;
#endif

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
        //if(true && r->top<150 && r->left<700 && x==0 && y==0)
        //std::cout<<"ShaHiLogLumi: [x,y]="<<x+r->left<<","<<y+r->top<<"  pin: "<<pin[0]<<"  L: "<<L<<"  bias: "<<bias<<"  pout="<<pout[0]<<std::endl;
      }
    }
#ifndef NDEBUG
    std::cout<<"  pout[0] = "<<(void*)VIPS_REGION_ADDR(oreg, oreg->valid.left, oreg->valid.top)
        <<" -> "<<*((float*)VIPS_REGION_ADDR(oreg, oreg->valid.left, oreg->valid.top))<<std::endl;
#endif
  }
};




PF::ShadowsHighlightsV2Par::ShadowsHighlightsV2Par():
OpParBase(),
amount("amount",this,pow(10, 0.5)),
shadows("shadows",this,pow(10, 0.0)),
shadows_range("shadows_range",this,5.0),
highlights("highlights",this,pow(10, 0.0)),
highlights_range("highlights_range",this,0.5),
contrast("constrast",this,0),
contrast_threshold("constrast_threshold",this,1),
anchor("anchor",this,0.5),
radius("sh_radius",this,128),
threshold("sh_threshold",this,0.2),
show_residual("show_residual", this, false),
single_scale_blur("single_scale_blur", this, false),
median_radius(5),
in_profile( NULL )
{
  loglumi = new PF::Processor<ShaHiLogLumiPar,ShaHiLogLumiProc>();
  int ts = 1;
  for(int gi = 0; gi < 10; gi++) {
    guided[gi] = new_guided_filter();
    threshold_scale[gi] = ts;
    ts *= 2;
  }

  set_type("shadows_highlights_v2" );

  set_default_name( _("shadows/highlights") );
}


bool PF::ShadowsHighlightsV2Par::needs_caching()
{
  return true;
}


static void fill_rv(int* rv, float* tv, float radius, int level)
{
  int gi = 0;
  for(gi = 0; gi < 10; gi++) rv[gi] = 0;

  int r = 1;
  float ts = 1;

  for(gi = 0; gi < 9; gi++) {
    if( r >= radius ) break;
    rv[gi] = r;
    tv[gi] = ts;
    ts *= 1.5;
    if( (r*4) > radius ) break;
    r *= 4;
  }
  rv[gi] = radius;
  tv[gi] = ts;
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
  int tot_padding = 0;
  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = radius.get(); tv[0] = 1;
  if( single_scale_blur.get() == false )
    fill_rv(rv, tv, radius.get(), level);

  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
#ifndef NDEBUG
    std::cout<<"ShadowsHighlightsV2Par::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
#endif
    if(rv[gi] == 0) break;
    guidedpar->set_radius( rv[gi] );
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

  set_padding( tot_padding, id );
}




VipsImage* PF::ShadowsHighlightsV2Par::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  std::vector<VipsImage*> in2;

  //if( (get_render_mode() == PF_RENDER_PREVIEW) /*&& is_editing()*/ )
    show_residual_ = show_residual.get();
  //else
  //  show_residual_ = false;

  in_profile = PF::get_icc_profile( in[0] );

#ifndef NDEBUG
  std::cout<<"ShadowsHighlightsV2Par::build(): level="<<level
      <<"  amount="<<amount.get()<<"  shadows="<<shadows.get()<<"  shadows_range="<<shadows_range.get()
      <<"  highlights="<<highlights.get()<<"  highlights_range="<<highlights_range.get()
      <<"  contrast="<<contrast.get()<<"  contrast_threshold="<<contrast_threshold.get()
      <<"  anchor="<<anchor.get()<<"  radius="<<radius.get()<<"  threshold="<<threshold.get()
      <<"  show_residual="<<show_residual.get()<<"  single_scale_blur="<<single_scale_blur.get()
      <<std::endl;
#endif


  ShaHiLogLumiPar* logpar = dynamic_cast<ShaHiLogLumiPar*>( loglumi->get_par() );
#ifndef NDEBUG
  std::cout<<"ShadowsHighlightsV2Par::build(): logpar="<<logpar<<std::endl;
#endif
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( anchor.get() );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  } else {
    logimg = in[0];
    PF_REF(logimg, "ShadowsHighlightsV2Par::build: in[0] ref");
  }

#ifndef NDEBUG
  std::cout<<"ShadowsHighlightsV2Par::build(): logimg="<<logimg<<std::endl;
#endif

  if( !logimg ) return NULL;




  //std::cout<<"ShadowsHighlightsV2Par::build(): ts="<<ts<<std::endl;


  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = radius.get(); tv[0] = 1;
  if( single_scale_blur.get() == false )
    fill_rv(rv, tv, radius.get(), level);

  VipsImage* timg = logimg;
  VipsImage* smoothed = logimg;
#ifndef NDEBUG
  std::cout<<"ShadowsHighlightsV2Par::build(): radius="<<radius.get()<<std::endl;
#endif


  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( timg );
    guidedpar->set_format( get_format() );
    std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="<<rv[gi]<<"  threshold="<<threshold.get() / tv[gi]<<std::endl;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(threshold.get() / tv[gi]);
    int subsampling = 1;
    while( subsampling <= 16 ) {
      if( (subsampling*8) >= rv[gi] ) break;
      subsampling *= 2;
    }
    guidedpar->set_subsampling(subsampling);
    guidedpar->set_convert_to_perceptual( false );
    guidedpar->propagate_settings();
    guidedpar->compute_padding(timg, 0, level);

#ifndef NDEBUG
    std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="
        <<rv[gi]<<"  padding="<<guidedpar->get_padding(0)
        <<"  logimg="<<logimg<<"  smoothed="<<smoothed<<std::endl;
#endif
    if( guidedpar->get_padding(0) > 64 ) {
      VipsAccess acc = VIPS_ACCESS_RANDOM;
      int threaded = 1, persistent = 1;
      VipsImage* cached = NULL;
      if( phf_tilecache(timg, &cached,
          "access", acc, "threaded", threaded,
          "persistent", persistent, NULL) ) {
        std::cout<<"ShadowsHighlightsV2Par::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( timg, "ShadowsHighlightsV2Par::build(): cropped unref" );
      timg = cached;
    }

    in2.clear();
    in2.push_back( timg );
    if( gi==0 && smoothed != logimg ) {
#ifndef NDEBUG
      std::cout<<"ShadowsHighlightsV2Par::build(): adding smoothed guide image"<<std::endl;
#endif
      in2.push_back( smoothed );
    }
    smoothed = guidedpar->build( in2, first, NULL, NULL, level );
    if( !smoothed ) {
      std::cout<<"ShadowsHighlightsV2Par::build(): NULL local contrast enhanced image"<<std::endl;
      return NULL;
    }
    PF_UNREF(timg, "ShadowsHighlightsV2Par::build(): timg unref");
    timg = smoothed;
    //std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  logimg="<<logimg<<"  smoothed="<<smoothed<<std::endl;
  }


  if( !smoothed ) {
    std::cout<<"ShadowsHighlightsV2Par::build(): null smoothed image"<<std::endl;
    PF_REF( in[0], "ShadowsHighlightsV2Par::build(): null smoothed image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed);
  in2.push_back(logimg);
  in2.push_back(in[0]);
  VipsImage* shahi = OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( smoothed, "ShadowsHighlightsV2Par::build() smoothed unref" );
  set_image_hints( in[0] );

  return shahi;
}



PF::ProcessorBase* PF::new_shadows_highlights_v2()
{ return new PF::Processor<PF::ShadowsHighlightsV2Par,PF::ShadowsHighlightsV2Proc>(); }
