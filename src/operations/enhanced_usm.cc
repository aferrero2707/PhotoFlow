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
#include "enhanced_usm.hh"


class EUSMLogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
  bool linear;
public:
  EUSMLogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5), linear(false) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }

  bool do_linear() { return linear; }
  void set_linear(bool l) { linear = l; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("EUSMLogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("EUSMLogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("EUSMLogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("EUSMLogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class EUSMLogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class EUSMLogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    EUSMLogLumiPar* opar = dynamic_cast<EUSMLogLumiPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;
    bool linear = opar->do_linear();

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
        //L *= bias;
        if( !linear ) {
          pL = (L>1.0e-16) ? xlog10( L ) : -16;
          pout[0] = pL;
        } else {
          L /= 0.05;
          pL = (L > 1) ? xlog10( L ) : (L - 1);
          pout[0] = pL;
        }
      }
    }
  }
};



PF::EnhancedUnsharpMaskPar::EnhancedUnsharpMaskPar():
  OpParBase(), 
  do_sum("do_sum",this,true),
  linear("linear",this,false),
  amount("amount",this,1),
  radius("radius",this,3),
  threshold_l("threshold_l",this,0.005),
  threshold_h("threshold_h",this,0.02)
{
  loglumi = new PF::Processor<EUSMLogLumiPar,EUSMLogLumiProc>();
  guided[0] = new_guided_filter();
  guided[1] = new_guided_filter();

  set_type("enhanced_usm" );

  set_default_name( _("enhanced usm") );
}



void PF::EnhancedUnsharpMaskPar::propagate_settings()
{
  for(int gi = 0; gi < 2; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    guidedpar->propagate_settings();
  }
}



void PF::EnhancedUnsharpMaskPar::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  threshold_real_l = threshold_l.get();
  threshold_real_h = threshold_h.get();
  if( get_linear() ) {
    threshold_real_l /= 1;
    threshold_real_h /= 1;
  }

  int padding = 0;
  int rv[2] = { radius.get(), radius.get() };
  float tv[2] = { threshold_real_l, threshold_real_h };

  for(int gi = 0; gi < 1; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
#ifndef NDEBUG
    std::cout<<"EnhancedUnsharpMaskPar::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
#endif
    if(rv[gi] == 0) break;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold( tv[gi] );
    int subsampling = 1;
    guidedpar->set_subsampling(subsampling);
    guidedpar->propagate_settings();
    guidedpar->compute_padding(full_res, id, level);

    padding = guidedpar->get_padding(id);
  }

  set_padding( padding, id );
}



VipsImage* PF::EnhancedUnsharpMaskPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;


  std::vector<VipsImage*> in2;
  in_profile = PF::get_icc_profile( in[0] );

  threshold_real_l = threshold_l.get();
  threshold_real_h = threshold_h.get();
  if( get_linear() ) {
    threshold_real_l /= 1;
    threshold_real_h /= 1;
  }

  EUSMLogLumiPar* logpar = dynamic_cast<EUSMLogLumiPar*>( loglumi->get_par() );
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( 0.5 );
    logpar->set_linear( get_linear() );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  } else {
    logimg = in[0];
    PF_REF(logimg, "EnhancedUnsharpMaskPar::build: in[0] ref");
  }

#ifndef NDEBUG
  std::cout<<"EnhancedUnsharpMaskPar::build(): logimg="<<logimg<<std::endl;
#endif

  if( !logimg ) return NULL;




  int rv[2] = { radius.get(), radius.get() };
  float tv[2] = { threshold_real_l, threshold_real_h };

  //VipsImage* timg = logimg;
  VipsImage* smoothed[2] = { NULL };
#ifndef NDEBUG
  std::cout<<"EnhancedUnsharpMaskPar::build(): radius="<<radius.get()<<std::endl;
#endif

  for(int gi = 0; gi < 2; gi++) {
    std::cout<<"EnhancedUnsharpMaskPar::build(): gi="<<gi<<"  radius="<<rv[gi]<<std::endl;
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( logimg );
    guidedpar->set_format( get_format() );
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(tv[gi]);
    int subsampling = 1;
    guidedpar->set_subsampling(subsampling);
    guidedpar->set_convert_to_perceptual( false );
    //guidedpar->propagate_settings();
    //guidedpar->compute_padding(logimg, 0, level);
    VipsImage* tmpimg = logimg;
    PF_REF(tmpimg, "EnhancedUnsharpMaskPar::build(): tmpimg ref");
    in2.clear();
    in2.push_back( tmpimg );
    smoothed[gi] = guidedpar->build( in2, first, NULL, NULL, level );
    PF_UNREF(tmpimg, "EnhancedUnsharpMaskPar::build(): tmpimg unref");

    //#ifndef NDEBUG
    std::cout<<"EnhancedUnsharpMaskPar::build(): gi="<<gi<<"  radius="
        <<rv[gi]<<"  padding="<<guidedpar->get_padding(0)
        <<"  logimg="<<logimg<<"  smoothed="<<smoothed[gi]<<std::endl;
    //#endif

    //if( smoothed[gi] ) {
    //  PF_UNREF(logimg, "EnhancedUnsharpMaskPar::build(): timg unref");
    //}
  }


  if( !smoothed[0] || !smoothed[1] ) {
    std::cout<<"EnhancedUnsharpMaskPar::build(): null smoothed image"<<std::endl;
    PF_REF( in[0], "EnhancedUnsharpMaskPar::build(): null smoothed image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed[0]);
  in2.push_back(smoothed[1]);
  in2.push_back(logimg);
  in2.push_back(in[0]);
  VipsImage* out = OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( smoothed[0], "EnhancedUnsharpMaskPar::build() smoothed[0] unref" );
  PF_UNREF( smoothed[1], "EnhancedUnsharpMaskPar::build() smoothed[1] unref" );
  PF_UNREF( logimg, "EnhancedUnsharpMaskPar::build() logimg unref" );
  set_image_hints( in[0] );

  return out;
}


PF::ProcessorBase* PF::new_enhanced_usm()
{ return new PF::Processor<PF::EnhancedUnsharpMaskPar,PF::EnhancedUnsharpMaskProc>(); }
