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
#include "blur_bilateral.hh"


PF::BlurBilateralPar::BlurBilateralPar():
PF::OpParBase(),
sigma_s("sigma_s",this,50),
sigma_r("sigma_r",this,20)
{
  bsplat = new PF::Processor<PF::BlurBilateralSplatPar,PF::BlurBilateralSplatProc>();
  blur = new PF::Processor<PF::BlurBilateralBlurPar,PF::BlurBilateralBlurProc>();
  slice = new PF::Processor<PF::BlurBilateralSlicePar,PF::BlurBilateralSliceProc>();
  //balgo = new PF::Processor<PF::BlurBilateralAlgoPar,PF::BlurBilateralAlgoProc>();
  //map_properties( balgo->get_par()->get_properties() );
  set_type( "blur_bilateral" );
  set_default_name( _("blilateral blur") );
}


//void set_iterations( int i ) { iterations.set( i ); }
void PF::BlurBilateralPar::set_sigma_s( float s )
{
  sigma_s.set( s );
  //BlurBilateralAlgoPar* bop = dynamic_cast<BlurBilateralAlgoPar*>( balgo->get_par() );
  //if(bop) bop->set_sigma_s( s );
}
void PF::BlurBilateralPar::set_sigma_r( float s )
{
  sigma_r.set( s );
  //BlurBilateralAlgoPar* bop = dynamic_cast<BlurBilateralAlgoPar*>( balgo->get_par() );
  //if(bop) bop->set_sigma_r( s );
}



VipsImage* PF::BlurBilateralPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;

  if( !srcimg ) return NULL;

  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );
  //std::cout<<"BlurBilateralPar::build(): csin = "<<csin<<std::endl;

  float ss, sr;
  ss = sigma_s.get();
  for( int l = 1; l <= level; l++ ) {
    ss /= 2;
  }
  sr = sigma_r.get();
  ss = roundf(ss); //ss = 100;
  sr = roundf(sr);
  int iss = ss, isr = sr;
  if(iss < 1) iss = 1;
  if(isr < 1) isr = 1;


  int sliced_ts = (iss*2) > 64 ? 2 : (64/iss)+1;
  int sliced_pad = 5;
  int extend_pad = (sliced_ts+sliced_pad)*2*iss;

  VipsImage *t[16];
  VipsImage *t2[16];

  for(int bi = 0; bi < srcimg->Bands; bi++) {

    if( vips_extract_band(srcimg, &t[bi], bi, NULL) ) {
      std::cout<<"BlurBilateralPar::build(): failed to extract band "<<bi<<std::endl;
      PF_REF(srcimg, "BlurBilateralPar::build(): srcimg ref after failed vips_extract_band");
      return srcimg;
    }

    // Extend the original image by padding pixels
    VipsImage* extended;
    VipsExtend extend = VIPS_EXTEND_COPY;
    //VipsExtend extend = VIPS_EXTEND_BLACK;
    if( vips_embed(t[bi], &extended, extend_pad, extend_pad,
        srcimg->Xsize+extend_pad*2, srcimg->Ysize+extend_pad*2,
        "extend", extend, NULL) ) {
      std::cout<<"BlurBilateralPar::build(): vips_embed() failed."<<std::endl;
      PF_REF( srcimg, "BlurBilateralPar::build(): vips_embed() failed." );
      return srcimg;
    }
    PF_UNREF( t[bi], "BlurBilateralPar::build(): t[bi] unref" );

    //std::cout<<"BlurBilateralPar::build(): level="<<level<<"  bi="<<bi<<"  extend_pad="<<extend_pad
    //    <<"  extended="<<(void*)extended<<"  size="<<extended->Xsize<<"x"<<extended->Ysize
    //    <<std::endl;



    BlurBilateralSplatPar* sop = dynamic_cast<BlurBilateralSplatPar*>( bsplat->get_par() );
    if(sop) {
      sop->set_sigma_s(iss);
      sop->set_sigma_r(isr);
    } else {
      std::cout<<"BlurBilateralPar::build(): cannot cast to BlurBilateralSplatPar*"<<std::endl;
      return NULL;
    }
    //std::cout<<"BlurBilateralPar::build(): sigma_s="<<get_sigma_s()<<"  level="<<level<<std::endl;
    //std::cout<<"                           iss="<<iss<<"  isr="<<isr<<std::endl;
    bsplat->get_par()->set_image_hints( extended );
    //bsplat->get_par()->grayscale_image( extended->Xsize, extended->Ysize );
    bsplat->get_par()->set_format( get_format() );
    std::vector<VipsImage*> in2;
    in2.clear();
    in2.push_back(extended);
    VipsImage* splatted = bsplat->get_par()->build( in2, 0, imap, omap, level );
    PF_UNREF( extended, "BlurBilateralPar::build(): extended unref" );




    int ts = 5;
    int nt = (splatted->Xsize * splatted->Ysize) / ts + 1;
    //VipsAccess acc = VIPS_ACCESS_SEQUENTIAL;
    VipsAccess acc = VIPS_ACCESS_RANDOM;
    int threaded = 1, persistent = 0;
    VipsImage* csplatted;
    if( phf_tilecache(splatted, &csplatted,
        "tile_width", ts,
        "tile_height", ts,
        "max_tiles", nt,
        "access", acc, "threaded", threaded,
        "persistent", persistent, NULL) ) {
      std::cout<<"BlurBilateralPar::build(): vips_tilecache() failed."<<std::endl;
      return csplatted;
    }
    PF_UNREF( splatted, "BlurBilateralPar::build(): splatted unref" );



    blur->get_par()->set_image_hints( csplatted );
    blur->get_par()->set_format( get_format() );
    //blur->get_par()->grayscale_image( csplatted->Xsize, csplatted->Ysize );
    in2.clear();
    in2.push_back(csplatted);
    VipsImage* blurred = blur->get_par()->build( in2, 0, imap, omap, level );
    PF_UNREF( csplatted, "BlurBilateralPar::build(): csplatted unref" );




    BlurBilateralSlicePar* slop = dynamic_cast<BlurBilateralSlicePar*>( slice->get_par() );
    if(slop) {
      slop->set_sigma_s(iss);
      slop->set_sigma_r(isr);
    } else {
      std::cout<<"BlurBilateralPar::build(): cannot cast to BlurBilateralSlicePar*"<<std::endl;
      return NULL;
    }
    slice->get_par()->set_image_hints( extended );
    slice->get_par()->set_format( get_format() );
    slice->get_par()->grayscale_image( extended->Xsize, extended->Ysize );
    in2.clear();
    in2.push_back(blurred);
    in2.push_back(extended);
    VipsImage* sliced = slice->get_par()->build( in2, 0, imap, omap, level );
    PF_UNREF( blurred, "BlurBilateralPar::build(): splatted unref" );
    //PF_UNREF( extended, "BlurBilateralPar::build(): extended unref" );




    int ts2 = iss*sliced_ts;
    int nt2 = (sliced->Xsize/ts2) * 1 + 1;
    //VipsAccess acc = VIPS_ACCESS_SEQUENTIAL;
    //VipsAccess acc = VIPS_ACCESS_RANDOM;
    //int threaded = 1, persistent = 0;
    VipsImage* cached;
    if( phf_tilecache(sliced, &cached,
        "tile_width", ts2,
        "tile_height", ts2,
        "max_tiles", nt2,
        "access", acc, "threaded", threaded,
        "persistent", persistent, NULL) ) {
      std::cout<<"BlurBilateralPar::build(): vips_tilecache() failed."<<std::endl;
      return sliced;
    }
    PF_UNREF( sliced, "BlurBilateralPar::build(): sliced unref" );



    //padding=0;
    // Final cropping to remove the padding pixels
    VipsImage* cropped;
    if( vips_crop(cached, &cropped, extend_pad, extend_pad,
        srcimg->Xsize, srcimg->Ysize, NULL) ) {
      std::cout<<"BlurBilateralPar::build(): vips_crop() failed."<<std::endl;
      PF_UNREF( sliced, "BlurBilateralPar::build(): sliced unref" );
      PF_REF( srcimg, "BlurBilateralPar::build(): vips_crop() failed" );
      return srcimg;
    }
    PF_UNREF( cached, "BlurBilateralPar::build(): cached unref" );
    t2[bi] = cropped;
  }

  VipsImage* joined;
  if( vips_bandjoin(t2, &joined, srcimg->Bands, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_bandjoin failed"<<std::endl;
    PF_REF(srcimg, "BlurBilateralPar::build(): srcimg ref after failed vips_bandjoin");
    return srcimg;
  }
  for(int bi = 0; bi < srcimg->Bands; bi++) {
    VIPS_UNREF(t2[bi]);
  }

  vips_image_init_fields( joined,
      srcimg->Xsize, srcimg->Ysize,
      srcimg->Bands, get_format(),
      get_coding(),
      get_interpretation(),
      1.0, 1.0);



  return joined;
}



VipsImage* PF::BlurBilateralPar::build_(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;

  if( !srcimg ) return NULL;

  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );
  std::cout<<"BlurBilateralPar::build(): csin = "<<csin<<std::endl;

  float ss, sr;
  ss = sigma_s.get();
  for( int l = 1; l <= level; l++ ) {
    ss /= 2;
  }
  sr = sigma_r.get();
  ss = roundf(ss); //ss = 20;
  sr = roundf(sr);
  int iss = ss, isr = sr;
  if(iss < 1) iss = 1;
  if(isr < 1) isr = 1;


  int bpad = 50;
  //BlurBilateralBlurPar* bop = dynamic_cast<BlurBilateralBlurPar*>( blur->get_par() );
  //if(bop) bpad = bop->get_padding();


  int padding = iss*bpad;
  std::cout<<"BlurBilateralPar::build(): level="<<level<<"  padding="<<padding<<std::endl;
  // Extend the original image by padding pixels
  VipsImage* extended;
  //VipsExtend extend = VIPS_EXTEND_COPY;
  VipsExtend extend = VIPS_EXTEND_BLACK;
  if( vips_embed(srcimg, &extended, padding, padding,
      srcimg->Xsize+padding*2, srcimg->Ysize+padding*2,
      "extend", extend, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_embed() failed."<<std::endl;
    PF_REF( in[0], "BlurBilateralPar::build(): vips_embed() failed." );
    return in[0];
  }
  //PF_UNREF( splatted, "BlurBilateralPar::build(): splatted unref" );


  BlurBilateralSplatPar* sop = dynamic_cast<BlurBilateralSplatPar*>( bsplat->get_par() );
  if(sop) {
    sop->set_sigma_s(iss);
    sop->set_sigma_r(isr);
  } else {
    std::cout<<"BlurBilateralPar::build(): cannot cast to BlurBilateralSplatPar*"<<std::endl;
    return NULL;
  }
  std::cout<<"BlurBilateralPar::build(): sigma_s="<<get_sigma_s()<<"  level="<<level<<std::endl;
  bsplat->get_par()->set_image_hints( extended );
  bsplat->get_par()->set_format( get_format() );
  std::vector<VipsImage*> in2;
  in2.clear();
  in2.push_back(extended);
  VipsImage* splatted = bsplat->get_par()->build( in2, 0, imap, omap, level );
  PF_UNREF( extended, "BlurBilateralPar::build(): extended unref" );




  int ts = 4;
  int nt = (splatted->Xsize * splatted->Ysize) / ts + 1;
  //VipsAccess acc = VIPS_ACCESS_SEQUENTIAL;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  int threaded = 1, persistent = 0;
  VipsImage* csplatted;
  if( phf_tilecache(splatted, &csplatted,
      "tile_width", ts,
      "tile_height", ts,
      "max_tiles", nt,
      "access", acc, "threaded", threaded,
      "persistent", persistent, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_tilecache() failed."<<std::endl;
    return csplatted;
  }
  PF_UNREF( splatted, "BlurBilateralPar::build(): splatted unref" );



  blur->get_par()->set_image_hints( csplatted );
  blur->get_par()->set_format( get_format() );
  in2.clear();
  in2.push_back(csplatted);
  VipsImage* blurred = blur->get_par()->build( in2, 0, imap, omap, level );
  PF_UNREF( csplatted, "BlurBilateralPar::build(): csplatted unref" );

  //blurred = splatted;

  BlurBilateralSlicePar* slop = dynamic_cast<BlurBilateralSlicePar*>( slice->get_par() );
  if(slop) {
    slop->set_sigma_s(iss);
    slop->set_sigma_r(isr);
  } else {
    std::cout<<"BlurBilateralPar::build(): cannot cast to BlurBilateralSlicePar*"<<std::endl;
    return NULL;
  }
  slice->get_par()->set_image_hints( extended );
  slice->get_par()->set_format( get_format() );
  in2.clear();
  //in2.push_back(blurred);
  in2.push_back(splatted);
  in2.push_back(extended);
  VipsImage* sliced = slice->get_par()->build( in2, 0, imap, omap, level );
  PF_UNREF( blurred, "BlurBilateralPar::build(): blurred unref" );


  //padding=0;
  // Final cropping to remove the padding pixels
  VipsImage* cropped;
  if( vips_crop(sliced, &cropped, padding, padding,
      srcimg->Xsize, srcimg->Ysize, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_crop() failed."<<std::endl;
    PF_UNREF( sliced, "BlurBilateralPar::build(): sliced unref" );
    PF_REF( srcimg, "BlurBilateralPar::build(): vips_crop() failed" );
    return srcimg;
  }
  PF_UNREF( sliced, "BlurBilateralPar::build(): sliced unref" );


  int ts2 = iss * 2;
  int nt2 = (cropped->Xsize/ts2) * 1 + 1;
  //VipsAccess acc = VIPS_ACCESS_SEQUENTIAL;
  //VipsAccess acc = VIPS_ACCESS_RANDOM;
  //int threaded = 1, persistent = 0;
  VipsImage* cached;
  if( phf_tilecache(cropped, &cached,
      "tile_width", ts2,
      "tile_height", ts2,
      "max_tiles", nt2,
      "access", acc, "threaded", threaded,
      "persistent", persistent, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_tilecache() failed."<<std::endl;
    return cropped;
  }
  PF_UNREF( cropped, "BlurBilateralPar::build(): cropped unref" );

  std::cout<<"BlurBilateralPar::build(): ts="<<ts<<"  ts2="<<ts2<<std::endl;

  return cached;
}



PF::BlurBilateralAlgoPar::BlurBilateralAlgoPar():
          PF::OpParBase(),
          //iterations("iterations",this,1),
          sigma_s("sigma_s",this,50),
          sigma_r("sigma_r",this,20)
{
  set_type( "blur_bilateral_algo" );
}



VipsImage* PF::BlurBilateralAlgoPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;

  colorspace_t csin = PF::PF_COLORSPACE_UNKNOWN;
  if( srcimg )
    csin = PF::convert_colorspace( srcimg->Type );
  std::cout<<"BlurBilateralAlgoPar::build(): csin = "<<csin<<std::endl;

  ss = sigma_s.get();
  for( int l = 1; l <= level; l++ ) {
    ss /= 2;
  }
  sr = sigma_r.get();

  out = OpParBase::build( in, first, imap, omap, level );

  return out;
}


PF::ProcessorBase* PF::new_blur_bilateral()
{
  return( new PF::Processor<PF::BlurBilateralPar,PF::BlurBilateralProc>() );
}
