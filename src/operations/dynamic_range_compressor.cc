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
#include "gaussblur.hh"
#include "gmic/blur_bilateral.hh"
#include "blur_bilateral.hh"
#include "dynamic_range_compressor.hh"



class LogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
public:
  LogLumiPar():
    PF::OpParBase(), profile(NULL) {

  }

  PF::ICCProfile* get_profile() { return profile; }


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
    printf("LogLumiPar::build(): out=%p\n", out);

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

    float bias = 1.0f/profile->perceptual2linear(0.5);
    float scale = 10; //100.f/12.f;
    //float lbias = log(bias) * inv_log_base;

    float L, pL;
    float* pin;
    float* pout;
    int x, y;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < width; x++, pin+=3, pout++ ) {
        L = profile->get_lightness(pin[0], pin[1], pin[2]);
        L *= bias;

        //if( profile->is_linear() )
        //  pL = profile->linear2perceptual(L);
        //else pL = L;

        if(L <= 1.0e-6) pL = -6;
        else pL = log(L) * inv_log_base;
        if( std::isnan(pL) ) { std::cout<<"pL isnan, L="<<L<<std::endl; pL = -6; }
        //pL = L;

        if(false && x<8 && y==0 && r->left==0 && r->top==0)
          std::cout<<"L="<<L<<"  pL="<<pL<<std::endl;
        //pout[0] = pout[1] = pout[2] = pL;
        pout[0] = (pL+6) * scale;
      }
    }
  }
};





PF::DynamicRangeCompressorPar::DynamicRangeCompressorPar():
  OpParBase(), 
  amount("amount",this,1),
  enable_equalizer("enable_equalizer",this,false),
  blacks_amount("blacks_amount",this,0),
  shadows_amount("shadows_amount",this,0.7),
  midtones_amount("midtones_amount",this,1),
  highlights_amount("highlights_amount",this,0.7),
  whites_amount("whites_amount",this,0),
  bilateral_iterations("bilateral_iterations",this,1),
  bilateral_sigma_s("bilateral_sigma_s",this,5),
  bilateral_sigma_r("bilateral_sigma_r",this,5),
  strength_s("strength_s", this, 50),
  strength_h("strength_h", this, 50),
  local_contrast("local_contrast", this, 0),
  caching(false)
{
  loglumi = new PF::Processor<LogLumiPar,LogLumiProc>();
  //bilateral = new_gmic_blur_bilateral();
  bilateral = new_blur_bilateral();

  // The tone curve is initialized as a bell-like curve with the
  // mid-tones point at 100% and the shadows/highlights points at 0%
  float xpt = 0.0, ypt = blacks_amount.get();
  tone_curve.set_point( 0, xpt, ypt );
  xpt = 1.0; ypt = whites_amount.get();
  tone_curve.set_point( 1, xpt, ypt );

  tone_curve.add_point( 0.25, shadows_amount.get() );
  tone_curve.add_point( 0.5, midtones_amount.get() );
  tone_curve.add_point( 0.75, highlights_amount.get() );

  set_type("dynamic_range_compressor" );

  set_default_name( _("dynamic range compressor") );
}



void PF::DynamicRangeCompressorPar::propagate_settings()
{
  //GmicBlurBilateralPar* bilateralpar = dynamic_cast<GmicBlurBilateralPar*>( bilateral->get_par() );
  BlurBilateralPar* bilateralpar = dynamic_cast<BlurBilateralPar*>( bilateral->get_par() );
  if( bilateralpar ) {
    //float ss = 0.01 * bilateral_sigma_s.get() * MIN(full_res->Xsize, full_res->Ysize);
    //float ss = bilateral_sigma_s.get()*10;
    //bilateralpar->set_sigma_s( ss );
    //bilateralpar->set_sigma_r( bilateral_sigma_r.get() );
    bilateralpar->propagate_settings();
  }
}



void PF::DynamicRangeCompressorPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  //GmicBlurBilateralPar* bilateralpar = dynamic_cast<GmicBlurBilateralPar*>( bilateral->get_par() );
  BlurBilateralPar* bilateralpar = dynamic_cast<BlurBilateralPar*>( bilateral->get_par() );
  if( bilateralpar ) {
    float ss = 0.01 * bilateral_sigma_s.get() * MIN(full_res->Xsize, full_res->Ysize);
    //float ss = bilateral_sigma_s.get()*10;
    bilateralpar->set_sigma_s( ss );
    bilateralpar->set_sigma_r( bilateral_sigma_r.get() );
    bilateralpar->compute_padding(full_res, id, level);
    set_padding( bilateralpar->get_padding(id), id );
    std::cout<<"DynamicRangeCompressorPar()::compute_padding(): sigma_s="<<bilateral_sigma_s.get()
        <<"  level="<<level<<"  padding="<<get_padding(id)<<std::endl;
  }
}



VipsImage* PF::DynamicRangeCompressorPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  profile = PF::get_icc_profile( in[0] );
  if( !profile ) {printf("DynamicRangeCompressorPar::build(): profile==NULL\n"); return NULL;}

  /*
  tone_curve.lock();
  float xpt, ypt;
  xpt=0.0; ypt=blacks_amount.get(); tone_curve.set_point( 0, xpt, ypt );
  xpt=0.25; ypt=shadows_amount.get(); tone_curve.set_point( 1, xpt, ypt );
  xpt=0.5; ypt=midtones_amount.get(); tone_curve.set_point( 2, xpt, ypt );
  xpt=0.75; ypt=highlights_amount.get(); tone_curve.set_point( 3, xpt, ypt );
  xpt=1.0; ypt=whites_amount.get(); tone_curve.set_point( 4, xpt, ypt );

  for(int i = 0; i <= FormatInfo<unsigned char>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned char>::RANGE;
    vec8[i] = tone_curve.get_value( x );
    //vec8[i] = (short int)(y*FormatInfo<unsigned char>::RANGE);
    //std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec8[i]="<<vec8[i]<<std::endl;
  }
  for(unsigned int i = 0; i <= FormatInfo<unsigned short int>::RANGE; i++) {
    float x = ((float)i)/FormatInfo<unsigned short int>::RANGE;
    vec16[i] = tone_curve.get_value( x );
    //vec16[i] = (int)(y*FormatInfo<unsigned short int>::RANGE);
   //if(i%1000 == 0)
    //if(curve.get().get_points().size()>100)
   //   std::cout<<"i="<<i<<"  x="<<x<<"  y="<<y<<"  vec16[i]="<<vec16[i]<<"  points="<<curve.get().get_points().size()<<std::endl;
  }
  tone_curve.unlock();
  */

  std::vector<VipsImage*> in2;
  LogLumiPar* logpar = dynamic_cast<LogLumiPar*>( loglumi->get_par() );
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    logimg = logpar->build( in, first, imap, omap, level );
  }

  std::cout<<"DynamicRangeCompressorPar::build(): logimg="<<logimg<<std::endl;

  if( !logimg ) return NULL;

  in2.clear();
  in2.push_back(logimg);
  VipsImage* smoothed = NULL;
  //GmicBlurBilateralPar* bilateralpar = dynamic_cast<GmicBlurBilateralPar*>( bilateral->get_par() );
  BlurBilateralPar* bilateralpar = dynamic_cast<BlurBilateralPar*>( bilateral->get_par() );
  if( bilateralpar ) {
    bilateralpar->set_image_hints( in2[0] );
    bilateralpar->set_format( get_format() );
    //bilateralpar->set_sigma_s( 0.01 * bilateral_sigma_s.get() * MIN(logimg->Xsize, logimg->Ysize) );
    //bilateralpar->set_sigma_s( bilateral_sigma_s.get() * 10 );
    bilateralpar->set_sigma_r( bilateral_sigma_r.get() );
    smoothed = bilateralpar->build( in2, first, imap, omap, level );
    PF_UNREF(logimg, "DynamicRangeCompressorPar::build(): logimg unref");
  }

  if( !smoothed ) {
    std::cout<<"DynamicRangeCompressorPar::build(): NULL local contrast enhanced image"<<std::endl;
    return NULL;
  }

  in2.clear();
  in2.push_back(smoothed);
  in2.push_back(logimg);
  in2.push_back(in[0]);
  VipsImage* out = OpParBase::build( in2, 0, imap, omap, level );

#ifndef NDEBUG
  std::cout<<"DynamicRangeCompressorPar::build(): out="<<out<<std::endl;
#endif
  PF_UNREF( smoothed, "DynamicRangeCompressorPar::build(): smoothed unref" );

  return out;
}


PF::ProcessorBase* PF::new_dynamic_range_compressor()
{ return( new PF::Processor<PF::DynamicRangeCompressorPar,PF::DynamicRangeCompressorProc>() ); }
