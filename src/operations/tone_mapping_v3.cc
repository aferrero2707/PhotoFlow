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

//#ifdef __SSE2__
//#include "../rt/rtengine/sleefsseavx.c"
//#else
#include "../rt/rtengine/sleef.c"
//#endif

#include "../base/processor.hh"
#include "../base/processor_imp.hh"
#include "guided_filter.hh"
#include "tone_mapping_v3.hh"


//const int PF::GamutMapNYPoints = 1000;


#define GUIDED_RADIUS_SCALE 1.0f
#define GUIDED_THRESHOLD_SCALE 0.1f


class TMV3LogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
public:
  TMV3LogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("TMV3LogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("TMV3LogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("TMV3LogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("LogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class TMV3LogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class TMV3LogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    TMV3LogLumiPar* opar = dynamic_cast<TMV3LogLumiPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    const float bias = 1.0f/opar->get_anchor();

#ifndef NDEBUG
    std::cout<<"[TMV3LogLumi]:\n  ireg[0] = "<<ireg[0]<<" -> ("<<ireg[0]->valid.left<<","<<ireg[0]->valid.top<<") x ("<<ireg[0]->valid.width<<","<<ireg[0]->valid.height<<")\n";
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
        //L = MAX3(pin[0], pin[1], pin[2]);
        L *= bias;
        pL = (L>1.0e-16) ? xlog10( L ) : -16;

        pout[0] = pL;
        //if(true && r->top<150 && r->left<700 && x==0 && y==0)
        //std::cout<<"TMV3LogLumi: [x,y]="<<x+r->left<<","<<y+r->top<<"  pin: "<<pin[0]<<"  L: "<<L<<"  bias: "<<bias<<"  pout="<<pout[0]<<std::endl;
      }
    }
#ifndef NDEBUG
    std::cout<<"  pout[0] = "<<(void*)VIPS_REGION_ADDR(oreg, oreg->valid.left, oreg->valid.top)
        <<" -> "<<*((float*)VIPS_REGION_ADDR(oreg, oreg->valid.left, oreg->valid.top))<<std::endl;
#endif
  }
};


#define Float_t float
#define Double_t float


float PF::TM_V3::lin2log(float lin)
{
  return static_cast<float>(xlog10(lin * midGrayInv));
}

float PF::TM_V3::log2lin(float l)
{
  return (xpow(10, l) * midGray);
}

void PF::TM_V3::init(float s, float s2, float s3, float wp, float e, float lat)
{
  slope = s;
  slope2 = s2;
  slope3 = s3;

  whitePoint = wp;
  exposure = e;
  latitude = lat;

  midGray = 0.1845;
  midGrayInv = 1.0f / midGray;

  float xmax = midGray + (whitePoint - midGray) * latitude;
  logXmax = lin2log(xmax);
  logYmax = logXmax * slope;

  float xmin = midGray - midGray * latitude;
  logXmin = lin2log(xmin);
  logYmin = logXmin * slope;
  logXblack = logXmin-1.;
  logYblack = logXblack * slope + (logXblack - logXblack * slope) * 0.5;

  logDelta = static_cast<float>(xlog10(exposure));
  logWhite = lin2log(1);// - logDelta;
  logWP = lin2log(whitePoint) + logDelta;

  float lwp = logWP;
  float nmin = 0, nmax = 100000, n = 1;

  //std::cout<<"[TM_V3::init] wp="<<wp<<"  lwp="<<lwp<<std::endl;

  /*
  // search for white clipping point
  int i = 0;
  while(true) {
    set_norm(n);
    float owp = logistic(lwp);
    std::cout<<"lwp "<<lwp<<"  min "<<nmin<<"  max "<<nmax<<"  owp "<<owp<<"  norm "<<norm<<std::endl;
    if(owp > logWhite) {
      nmax = n;
      n = (nmin + nmax) / 2;
    } else {
      nmin = n;
      n = (nmin + nmax) / 2;
    }
    i++;
    if(fabs(owp-logWhite) < 1.0e-10) break;
    if(i > 100) break;
  }
  */
  {
    // computation of the cubic spline coefficients
    float x0 = logXmax, y0 = logYmax, x1 = lwp, y1 = logWhite;
    float xd = x0 - x1;
    float xd3 = xd * xd * xd;
    float m0 = slope, m1 = slope2;

    //a&=& \frac{(m0+m1) (x0-x1)-2 y0+2 y1}{(x0-x1){}^3}
    sa = ((m0+m1) * xd - 2*y0 + 2*y1) / xd3;

    //b&=& \frac{-m0 (x0-x1) (x0+2 x1)+m1 (-2 x0^2+x1 x0+x1^2)+3 (x0+x1)
    //   (y0-y1)}{(x0-x1){}^3}
    sb = (-m0 * xd * (x0 + 2*x1) + m1 * (-2 * x0*x0 + x0*x1 + x1*x1) + 3 * (x0+x1) * (y0-y1)) / xd3;

    //c&=& \frac{m1 x0 (x0-x1) (x0+2 x1)-x1 (m0 (-2 x0^2+x1 x0+x1^2)+6 x0
    //   (y0-y1))}{(x0-x1){}^3}
    sc = (m1 * x0 * xd * (x0 + 2*x1) - x1 * (m0 * (-2 * x0*x0 + x0*x1 + x1*x1) + 6 * x0 * (y0-y1))) / xd3;

    //d&=& \frac{(x0-3 x1) y1 x0^2+x1 (x0 (x1-x0) (m1 x0+m0 x1)-x1 (x1-3 x0)
    //   y0)}{(x0-x1){}^3}
    sd = ((x0 - 3*x1) * y1 * x0*x0 + x1 * (x0 * (x1-x0) * (m1*x0 + m0*x1) - x1 * (x1 - 3*x0) * y0)) / xd3;

    std::cout<<"x1 = "<<x1<<"  y1 = "<<sa*x1*x1*x1 + sb*x1*x1 + sc*x1 + sd<<std::endl;
  }

  {
    // computation of the cubic spline coefficients
    float x0 = logXmin;
    float y0 = logYmin;
    float x1 = logXblack;
    float y1 = logYblack;
    float xd = x0 - x1;
    float xd3 = xd * xd * xd;
    float m0 = slope, m1 = slope3;

    //a&=& \frac{(m0+m1) (x0-x1)-2 y0+2 y1}{(x0-x1){}^3}
    sa2 = ((m0+m1) * xd - 2*y0 + 2*y1) / xd3;

    //b&=& \frac{-m0 (x0-x1) (x0+2 x1)+m1 (-2 x0^2+x1 x0+x1^2)+3 (x0+x1)
    //   (y0-y1)}{(x0-x1){}^3}
    sb2 = (-m0 * xd * (x0 + 2*x1) + m1 * (-2 * x0*x0 + x0*x1 + x1*x1) + 3 * (x0+x1) * (y0-y1)) / xd3;

    //c&=& \frac{m1 x0 (x0-x1) (x0+2 x1)-x1 (m0 (-2 x0^2+x1 x0+x1^2)+6 x0
    //   (y0-y1))}{(x0-x1){}^3}
    sc2 = (m1 * x0 * xd * (x0 + 2*x1) - x1 * (m0 * (-2 * x0*x0 + x0*x1 + x1*x1) + 6 * x0 * (y0-y1))) / xd3;

    //d&=& \frac{(x0-3 x1) y1 x0^2+x1 (x0 (x1-x0) (m1 x0+m0 x1)-x1 (x1-3 x0)
    //   y0)}{(x0-x1){}^3}
    sd2 = ((x0 - 3*x1) * y1 * x0*x0 + x1 * (x0 * (x1-x0) * (m1*x0 + m0*x1) - x1 * (x1 - 3*x0) * y0)) / xd3;

    //std::cout<<"x1 = "<<x1<<"  y1 = "<<sa*x1*x1*x1 + sb*x1*x1 + sc*x1 + sd<<std::endl;
    std::cout<<"slope="<<slope<<"  y1="<<y1<<std::endl;
  }
}

/*
float PF::TM_V3::logistic(float x, bool verbose)
{
  x += logDelta;
  float x2 = x*x;
  float x3 = x2*x;

  float l;
  if(x < logXblack) l = (x - logXblack + logYblack);
  else if(x < logXmin) l = (sa2 * x3 + sb2 * x2 + sc2 * x + sd2);
  else if(x < logXmax) l = slope * x;
  else if(x < logWP) l = (sa * x3 + sb * x2 + sc * x + sd);
  else l = ((x-logWP) * slope2 + logWhite);

  //float l = (x > logXmax) ? ((x < logWP) ? sa * x3 + sb * x2 + sc * x + sd : (x-logWP) * slope2 + logWhite) : slope * x;

  //float l = (x > logXmax) ? Yscale * (2.0f / (xexpf(-2.0f * slope * Xscale * (x - logXmax)) + 1.0f) - 1.0f) + logYmax : x * slope;
  //l += logDelta;
  //float l = (x > logXmax) ? Yscale * (2.0f / (xexpf(-2.0f * slope * Xscale * (x - logXmax)) + 1.0f) - 1.0f) + logYmax : x * slope;
  if(verbose) std::cout<<"logistic: "<<" "<<x<<" "<<slope<<" "<<logXmax<<" "<<logYmax<<" "<<l<<" "<<"\n";
  return l;
}*/

float PF::TM_V3::get(float x, bool verbose)
{
  float lx = lin2log(x);
  float ly = get_log(lx, verbose);
  if(verbose) std::cout<<"[TM_V3::get] x="<<x<<"  lx="<<lx<<"  ly="<<ly<<"  y="<<log2lin(ly)<<std::endl;
  return log2lin(ly);
}

float PF::TM_V3::get_log(float x, bool verbose)
{
  x += logDelta;

  float l;
  //if(x < logXblack) l = (x - logXblack) * slope3 + logYblack;
  if(x < logXblack) {
    l = (x - logXblack) * slope3 + logYblack;
    //std::cout<<"logistic: "<<" "<<x<<" "<<slope3<<" "<<logXblack<<" "<<logYblack<<" "<<l<<" "<<"\n";
  }
  else if(x < logXmin) {
    float x2 = x*x;
    float x3 = x2*x;
    l = (sa2 * x3 + sb2 * x2 + sc2 * x + sd2);
  } else if(x < logXmax) l = slope * x;
  else if(x < logWP) {
    float x2 = x*x;
    float x3 = x2*x;
    l = (sa * x3 + sb * x2 + sc * x + sd);
  } else l = ((x-logWP) * slope2 + logWhite);

  //float l = (x > logXmax) ? ((x < logWP) ? sa * x3 + sb * x2 + sc * x + sd : (x-logWP) * slope2 + logWhite) : slope * x;

  //float l = (x > logXmax) ? Yscale * (2.0f / (xexpf(-2.0f * slope * Xscale * (x - logXmax)) + 1.0f) - 1.0f) + logYmax : x * slope;
  //l += logDelta;
  //float l = (x > logXmax) ? Yscale * (2.0f / (xexpf(-2.0f * slope * Xscale * (x - logXmax)) + 1.0f) - 1.0f) + logYmax : x * slope;
  if(verbose) std::cout<<"logistic: "<<" "<<x<<" "<<slope<<" "<<logXmax<<" "<<logYmax<<" "<<l<<" "<<"\n";
  return l;
  //return logistic(lx, verbose);
}

float PF::TM_V3::get_slope(float x)
{
  x += logDelta;

  float s;
  if(x < logXblack) s = slope3;
  else if(x < logXmin) {
    float x2 = x*x;
    //l = (sa2 * x3 + sb2 * x2 + sc2 * x + sd2);
    s = sa2 * 3 * x2 + sb2 * 2 * x + sc2;
  } else if(x < logXmax) s = slope;
  else if(x < logWP) {
    float x2 = x*x;
    //l = (sa * x3 + sb * x2 + sc * x + sd);
    s = sa * 3 * x2 + sb * 2 * x + sc;
  } else s = slope2;
  return s;
}




PF::ToneMappingParV3::ToneMappingParV3():
  OpParBase(),
  //preset("preset",this,PF::TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST,"TONE_MAPPING_PRESET_MEDIUM_HIGH_CONTRAST","Medium High Contrast"),
  hue_protection("hue_protection",this,false),
  exposure("exposure",this,1),
  latitude("latitude",this,0),
  slope("slope",this,1),
  slope2("slope2",this,pow(10,-0.2)),
  slope3("slope3",this,1.0),
  wp("white_point",this,2),
  lc_enable("local_contrast_enable",this,false),
  lc_amount("local_contrast_amount",this,0.0),
  lc_radius("local_contrast_radius",this,128),
  lc_threshold("local_contrast_threshold",this,0.2),
  lock_exposure_wp("lock_exposure_wp",this,true),
  lumi_blend_frac("lumi_blend_frac",this,0),
  sh_desaturation("sh_desaturation",this,0.5),
  hl_desaturation("hl_desaturation",this,0.5),
  lc_curve( "lc_curve", this ),
  icc_data( NULL )
{
  loglumi = new PF::Processor<TMV3LogLumiPar,TMV3LogLumiProc>();
  int ts = 1;
  for(int gi = 0; gi < 10; gi++) {
    guided[gi] = new_guided_filter();
    threshold_scale[gi] = ts;
    ts *= 2;
  }

  float px, py;

  px = 0;
  py = 1;
  lc_curve.get().set_point(0, px, py);

  px = 1;
  py = 1;
  lc_curve.get().set_point(1, px, py);

  lc_curve.get().add_point(0.1, 1); // -4 EV
  lc_curve.get().add_point(0.2, 1); // -3 EV
  lc_curve.get().add_point(0.3, 1); // -2 EV
  lc_curve.get().add_point(0.4, 1); // -1 EV
  lc_curve.get().add_point(0.5, 1); //  0 EV
  lc_curve.get().add_point(0.6, 1); // +1 EV
  lc_curve.get().add_point(0.7, 1); // +2 EV
  lc_curve.get().add_point(0.8, 1); // +3 EV
  lc_curve.get().add_point(0.9, 1); // +4 EV

  set_type("tone_mapping_v3" );

  set_default_name( _("tone mapping v3") );
}


void PF::ToneMappingParV3::pre_build( rendermode_t mode )
{
  float mid_grey = 0.1845;
  tm.init(get_slope(), get_slope2(), get_slope3(), get_white_point(), get_exposure(), get_latitude());
}


bool PF::ToneMappingParV3::needs_caching()
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


void PF::ToneMappingParV3::propagate_settings()
{
  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    guidedpar->propagate_settings();
  }
}


void PF::ToneMappingParV3::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  int tot_padding = 0;
  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = lc_radius.get(); tv[0] = 1;
  fill_rv(rv, tv, lc_radius.get(), level);

  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
#ifndef NDEBUG
    std::cout<<"ToneMappingParV3::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
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


VipsImage* PF::ToneMappingParV3::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  ICCProfile* new_icc_data = PF::get_icc_profile( in[0] );
#ifndef NDEBUG
  std::cout<<"ToneMappingParV3::build: new_icc_data="<<new_icc_data<<std::endl;

  std::cout<<"ToneMappingParV3::build: gain="<<LE_gain.get()
          <<"  slope="<<LE_slope.get()
          <<"  LE_lin_max="<<LE_lin_max.get()
          <<"  LE_knee_strength="<<LE_knee_strength.get()
          <<"  LE_compression="<<LE_compression.get()
          <<"  LE_shoulder_slope="<<LE_shoulder_slope.get()
          <<"  lumi_blend_frac="<<lumi_blend_frac.get()
          <<"  sh_desaturation="<<sh_desaturation.get()
          <<"  hl_desaturation="<<hl_desaturation.get()
      <<std::endl;
#endif
  float mid_grey = 0.1845;
  tm.init(get_slope(), get_slope2(), get_slope3(), get_white_point(), get_exposure(), get_latitude());

  icc_data = new_icc_data;
  std::vector<VipsImage*> in2;

  // log-luminance image
  TMV3LogLumiPar* logpar = dynamic_cast<TMV3LogLumiPar*>( loglumi->get_par() );
#ifndef NDEBUG
  std::cout<<"ToneMappingParV3::build(): logpar="<<logpar<<std::endl;
#endif
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( mid_grey );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  }

#ifndef NDEBUG
  std::cout<<"ToneMappingParV3::build(): logimg="<<logimg<<std::endl;
#endif
  if( !logimg ) {
    std::cout<<"ToneMappingParV3::build(): null logimg image"<<std::endl;
    PF_REF( in[0], "ToneMappingParV3::build(): null logimg image" );
    return in[0];
  }

  // Iterative guided filter
  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = lc_radius.get(); tv[0] = 1;
  fill_rv(rv, tv, lc_radius.get(), level);

  VipsImage* timg = logimg;
  VipsImage* smoothed = logimg;
#ifndef NDEBUG
  std::cout<<"ToneMappingParV3::build(): radius="<<lc_radius.get()<<std::endl;
#endif

  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( timg );
    guidedpar->set_format( get_format() );
    std::cout<<"ToneMappingParV3::build(): gi="<<gi<<"  radius="<<rv[gi]<<"  threshold="<<lc_threshold.get() / tv[gi]<<std::endl;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(lc_threshold.get() / tv[gi]);
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
    std::cout<<"ToneMappingParV3::build(): gi="<<gi<<"  radius="
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
        std::cout<<"ToneMappingParV3::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( timg, "ToneMappingParV3::build(): cropped unref" );
      timg = cached;
    }

    in2.clear();
    in2.push_back( timg );
    if( gi==0 && smoothed != logimg ) {
#ifndef NDEBUG
      std::cout<<"ToneMappingParV3::build(): adding smoothed guide image"<<std::endl;
#endif
      in2.push_back( smoothed );
    }
    smoothed = guidedpar->build( in2, first, NULL, NULL, level );
    if( !smoothed ) {
      std::cout<<"ToneMappingParV3::build(): NULL local contrast enhanced image"<<std::endl;
      return NULL;
    }
    PF_UNREF(timg, "ToneMappingParV3::build(): timg unref");
    timg = smoothed;
    //std::cout<<"ToneMappingParV3::build(): gi="<<gi<<"  logimg="<<logimg<<"  smoothed="<<smoothed<<std::endl;
  }


  if( !smoothed ) {
    std::cout<<"ToneMappingParV3::build(): null smoothed image"<<std::endl;
    PF_REF( in[0], "ToneMappingParV3::build(): null smoothed image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed);
  in2.push_back(logimg);
  in2.push_back(in[0]);

  rgb_image(in[0]->Xsize, in[0]->Ysize);
  VipsImage* out = OpParBase::build( in2, 0, NULL, NULL, level );
  PF_UNREF(smoothed, "ToneMappingParV3::build(): smoothed unref");

  return out;
}



using namespace PF;

template < OP_TEMPLATE_DEF >
class ToneMappingV3
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
  }
};



template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class ToneMappingV3< float, BLENDER, PF_COLORSPACE_RGB, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
  ICCProfile* prof;
  float get_norm(float R, float G, float B)
  {
    return MAX3(R, G, B);
  }
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingParV3* opar = dynamic_cast<ToneMappingParV3*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;


    float lumi_blend_frac[2]; // = opar->get_lumi_blend_frac();
    prof = opar->get_icc_data();

    TM_V3& tm = opar->get_tm();

    float* lin;
    float* sin;
    float* pin;
    float* pout;
    float RGB[5];
    float RGBin[4];
    float dRGB[4];
    float rRGB[4];
    float Lab[3];
    float D, R;
    int x, x0, y, k;

    const float minus = -1.f;

    float exposure = opar->get_exposure();
    float SH_desat = opar->get_sh_desaturation();
    float HL_desat = opar->get_hl_desaturation();
    float lc_amount = opar->get_lc_amount();

    lumi_blend_frac[0] = SH_desat;
    lumi_blend_frac[1] = 1.0f - HL_desat;

    ICCProfile* labprof = ICCStore::Instance().get_Lab_profile();
    ICCTransform rgb2lab, lab2rgb;
    rgb2lab.init(prof, labprof, VIPS_FORMAT_FLOAT);
    lab2rgb.init(labprof, prof, VIPS_FORMAT_FLOAT);

    //std::cout<<"exposure = "<<exposure<<std::endl;
    //std::cout<<"log(0.2*gamma+1) / gamma_scale = "<<log(0.2*gamma+1) / gamma_scale<<std::endl;
    //std::cout<<"log(gamma+1) / gamma_scale = "<<log(gamma+1) / gamma_scale<<std::endl;

    for( y = 0; y < height; y++ ) {
      sin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      lin = (float*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
      pin = (float*)VIPS_REGION_ADDR( ireg[2], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0, x0 = 0; x < line_size; x+=3, x0++ ) {

        //pout[x] = psmooth[x];
        //pout[x+1] = psmooth[x+1];
        //pout[x+2] = psmooth[x+2];
        //continue;

        float Rin = pin[x];
        float Gin = pin[x+1];
        float Bin = pin[x+2];

        RGB[0] = pin[x];
        RGB[1] = pin[x+1];
        RGB[2] = pin[x+2];
        RGB[3] = lin[x0];
        RGB[4] = sin[x0];
        if( RGB[0] < 0 ) RGB[0] = 0;
        if( RGB[1] < 0 ) RGB[1] = 0;
        if( RGB[2] < 0 ) RGB[2] = 0;
        D = (lin[x0] - sin[x0]);
        //std::cout<<"[TM] [x,y]="<<x0+r->left<<","<<y+r->top<<"  l1: "<<lin[x0]<<"  l2: "<<sin[x0]<<"  delta="<<D<<std::endl;

        float min = MIN3(RGB[0], RGB[1], RGB[2]);
        float max = MAX3(RGB[0], RGB[1], RGB[2]);
        int max_id = 0;
        if( RGB[1] > RGB[max_id]) max_id = 1;
        if( RGB[2] > RGB[max_id]) max_id = 2;

        RGB[0] = tm.get(RGB[0]);
        RGB[1] = tm.get(RGB[1]);
        RGB[2] = tm.get(RGB[2]);
        float slope = tm.get_slope(RGB[4]);
        //float slope2 = tm.get_slope(RGB[3]);
        //std::cout<<"RGB[3]="<<RGB[3]<<" slope="<<slope<<"    RGB[4]="<<RGB[4]<<" slope="<<slope2<<std::endl;

        float EVin = (RGB[4] / xlog10(2) + 5.0f) / 10.0f;
        float lceq = opar->get_lc_curve().get_value(EVin);
        float lc_amount2 = lc_amount * lceq;
        //std::cout<<"RGB[4]="<<RGB[4]<<"  EVin="<<EVin<<"  lceq="<<lceq<<"  lc_amount="<<lc_amount<<"  lc_amount2="<<lc_amount2<<std::endl;

        //RGB[3] = tm.get_log(RGB[3]);
        RGB[4] = tm.get_log(RGB[4]);

        // convert the input luminance back to linear
        float Lin = tm.log2lin(RGB[3]);

        // simple output of the channel-by-channel tone-mapping
        float Rtm = RGB[0];
        float Gtm = RGB[1];
        float Btm = RGB[2];
        // get the luminance of the tone mapped image
        float Ltm = prof->get_lightness(Rtm, Gtm, Btm);

        // luminance ratio for channel-by-channel tone-mapping
        float rtm = Ltm / Lin;

        // channel-by-channel tone-mapping luminance with original colors
        float Rtmsat = (slope < 1) ? pin[x] * rtm : Rtm;
        float Gtmsat = (slope < 1) ? pin[x+1] * rtm : Gtm;
        float Btmsat = (slope < 1) ? pin[x+2] * rtm : Btm;

        // local-contrast-enhanced version
        // add the local contrast to the blurred version after TM, and convert back to linear
        if(lc_amount2 > 1) {
          D *= lc_amount2;
          lc_amount2 = 1;
        }
        float Llc = tm.log2lin(RGB[4] + D);

        // luminance ratio between local-contrast-enhanced and channel-by-channel tone-mapping images
        float rtmlc = Llc / Ltm;

        // local-contrast-enhanced image with channel-by-channel tone-mapping colors
        float Rtmlc = RGB[0] * rtmlc;
        float Gtmlc = RGB[1] * rtmlc;
        float Btmlc = RGB[2] * rtmlc;

        // luminance ratio between local-contrast-enhanced and input images
        float rlc = Llc / Lin;

        // local-contrast-enhanced image with original colors
        float Rtmlcsat = (slope < 1) ? pin[x] * rlc : Rtmlc;
        float Gtmlcsat = (slope < 1) ? pin[x+1] * rlc : Gtmlc;
        float Btmlcsat = (slope < 1) ? pin[x+2] * rlc : Btmlc;

        // mix the local-contrast-enhanced and direct images, but only when slope < 1, otherwise take the direct image
        float Rtmout =  (slope < 1) ? lc_amount2 * Rtmlc + (1.0f - lc_amount2) * Rtm : Rtm;
        float Gtmout =  (slope < 1) ? lc_amount2 * Gtmlc + (1.0f - lc_amount2) * Gtm : Gtm;
        float Btmout =  (slope < 1) ? lc_amount2 * Btmlc + (1.0f - lc_amount2) * Btm : Btm;

        float Rsatout =  (slope < 1) ? lc_amount2 * Rtmlcsat + (1.0f - lc_amount2) * Rtmsat : Rtmsat;
        float Gsatout =  (slope < 1) ? lc_amount2 * Gtmlcsat + (1.0f - lc_amount2) * Gtmsat : Gtmsat;
        float Bsatout =  (slope < 1) ? lc_amount2 * Btmlcsat + (1.0f - lc_amount2) * Btmsat : Btmsat;

        float Rout = lumi_blend_frac[1] * Rsatout + (1.0f - lumi_blend_frac[1]) * Rtmout;
        float Gout = lumi_blend_frac[1] * Gsatout + (1.0f - lumi_blend_frac[1]) * Gtmout;
        float Bout = lumi_blend_frac[1] * Bsatout + (1.0f - lumi_blend_frac[1]) * Btmout;

        RGB[0] = Rout;
        RGB[1] = Gout;
        RGB[2] = Bout;

        //RGB[0] = lceq; //Rout;
        //RGB[1] = lceq; //Gout;
        //RGB[2] = lceq; //Bout;
        //std::cout<<"[TM] [x,y]="<<x0+r->left<<","<<y+r->top<<"  l1: "<<Lin<<"  out: "<<Lout<<"  R="<<R<<std::endl;


        float saturation = 1;
        //std::cout<<"LIN_POW: "<<LE_midgray<<" -> "<<RGB[3]<<std::endl;
        if( false && saturation != 1 ) {
          for( k=0; k < 3; k++) {
            RGB[k] = RGB[max_id] + saturation * (RGB[k] - RGB[max_id]);
          }
        }

        if( prof && opar->get_hue_protection() ) {
          float Jab_in[3], JCH_in[3];
          float Jab[3], JCH[3];
          float Jz, az, bz, C, H;
          // convert to Jzazbz in polar coordinates
          prof->to_Jzazbz(Rin, Gin, Bin, Jab_in[0], Jab_in[1], Jab_in[2]);
          PF::Lab2LCH(Jab_in, JCH_in, 1);

          //std::cout<<"RGBin:  "<<RGBin[0]<<" "<<RGBin[1]<<" "<<RGBin[2]<<" "<<std::endl;
          //std::cout<<"Jabin:  "<<Jab_in[0]<<" "<<Jab_in[1]<<" "<<Jab_in[2]<<" "<<std::endl;
          //std::cout<<"JCHin:  "<<JCH_in[0]<<" "<<JCH_in[1]<<" "<<JCH_in[2]<<" "<<std::endl;

          float max_before = MAX3(RGB[0], RGB[1], RGB[2]);
          prof->to_Jzazbz(RGB[0], RGB[1], RGB[2], Jab[0], Jab[1], Jab[2]);
          PF::Lab2LCH(Jab, JCH, 1);

          JCH[2] = JCH_in[2];
          //if( opar->get_gamut_compression() )
          //  prof->chroma_compression(JCH[0], JCH[1], JCH[2]);

          //std::cout<<"JCHout: "<<JCH[0]<<" "<<JCH[1]<<" "<<JCH[2]<<" "<<std::endl;
          PF::LCH2Lab(JCH, Jab, 1);
          //std::cout<<"Jabout: "<<Jab[0]<<" "<<Jab[1]<<" "<<Jab[2]<<" "<<std::endl;
          prof->from_Jzazbz( Jab[0], Jab[1], Jab[2], RGB[0], RGB[1], RGB[2] );
          //std::cout<<"RGBout: "<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<" "<<std::endl;
          float max_after = MAX3(RGB[0], RGB[1], RGB[2]);

          if(max_after > 1.0e-10) {
            float R = max_before / max_after;
            RGB[0] *= R;
            RGB[1] *= R;
            RGB[2] *= R;
          }

          //pout[x] = RGB[0];
          //pout[x+1] = RGB[1];
          //pout[x+2] = RGB[2];
        }

        if(false && opar->get_hue_protection()) {
          float li = xexp10( lin[x0] );
          float lo = xexp10( RGB[3] );
          float R = lo / li;

          pout[x] = pin[x] * R;
          pout[x+1] = pin[x+1] * R;
          pout[x+2] = pin[x+2] * R;
        } else {
          pout[x] = RGB[0];
          pout[x+1] = RGB[1];
          pout[x+2] = RGB[2];
        }
        //std::cout<<"[TM] [x,y]="<<x0+r->left<<","<<y+r->top<<"  pin0: "<<pin[x]<<"  pin1: "<<lin[x0]<<"  pin2: "<<sin[x0]<<"  delta: "<<D<<"  R="<<R<<"  pout="<<pout[x]<<std::endl;
        //std::cout<<"dRGB: "<<dRGB[0]<<" "<<dRGB[1]<<" "<<dRGB[2]<<" "<<std::endl;
        //std::cout<<"pout: "<<pout[x+0]<<" "<<pout[x+1]<<" "<<pout[x+2]<<" "<<std::endl;
      }
    }
  }
};




template < OP_TEMPLATE_DEF_CS_SPEC >
class ToneMappingV3< OP_TEMPLATE_IMP_CS_SPEC(PF_COLORSPACE_LAB) >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, OpParBase* par)
  {
    ToneMappingParV3* opar = dynamic_cast<ToneMappingParV3*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands;
    //int width = r->width;
    int height = r->height;

    T* pin;
    T* pout;
    int x, y;

    float a, b;

    for( y = 0; y < height; y++ ) {
      pin = (T*)VIPS_REGION_ADDR( ireg[in_first], r->left, r->top + y );
      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < line_size; x+=3 ) {

        pout[x] = pin[x];
        pout[x+1] = pin[x+1];
        pout[x+2] = pin[x+2];
      }
    }
  }
};




PF::ProcessorBase* PF::new_tone_mapping_v3()
{
  return( new PF::Processor<PF::ToneMappingParV3,ToneMappingV3>() );
}
