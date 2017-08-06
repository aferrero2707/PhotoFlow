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

#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "nlmeans.hh"


PF::NonLocalMeansPar::NonLocalMeansPar():
  OpParBase(),
  radius("radius",this,2),
  strength("strength",this,10),
  luma_frac("luma_frac", this, 0.5),
  chroma_frac("chroma_frac", this, 1),
  in_profile( NULL )
{
  set_demand_hint( VIPS_DEMAND_STYLE_SMALLTILE );
  set_type( "nlmeans" );

  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert2input = new_icc_transform();
  nlmeans_algo = new PF::Processor<PF::NonLocalMeans_DTAlgo_Par,PF::NonLocalMeans_DTAlgo_Proc>();

  set_default_name( _("NL means") );
}



VipsImage* PF::NonLocalMeansPar::build(std::vector<VipsImage*>& in, int first,
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

#ifndef NDEBUG
  std::cout<<"NonLocalMeansPar::build() called"<<std::endl;
#endif

  float scale = 1;
	for( unsigned int l = 1; l <= level; l++ ) scale /= 2;

  // adjust to zoom size:
  P = ceilf(radius.get() * fmin(scale, 2.0f)); // pixel filter size
  K = ceilf(7.f * fmin(scale, 2.0f));         // nbhood
  sharpness = 3000.0f / (1.0f + strength.get());
#ifndef NDEBUG
  std::cout<<"NonLocalMeansPar::build(): P="<<P<<" K="<<K<<" sharpness="<<sharpness<<std::endl;
#endif

  void *data;
  size_t data_length;

  if( in_profile ) cmsCloseProfile( in_profile );

  in_profile = NULL;
  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME,
                           &data, &data_length ) ) {
    in_profile = cmsOpenProfileFromMem( data, data_length );
  }

  std::vector<VipsImage*> in2;

  // Extend the image by two pixels to account for the pixel averaging window
  // of the impulse noise reduction algorithm
  VipsImage* extended;
  /**/
  VipsExtend extend = VIPS_EXTEND_COPY;
  if( vips_embed(srcimg, &extended, get_padding(), get_padding(),
      srcimg->Xsize+2*get_padding(), srcimg->Ysize+2*get_padding(),
      "extend", extend, NULL) ) {
    std::cout<<"ImpulseNRPar::build(): vips_embed() failed."<<std::endl;
    PF_REF( in[0], "ImpulseNRPar::build(): vips_embed() failed." );
    return in[0];
  }
  /**/
  //extended = srcimg; PF_REF(srcimg, "");

  convert2lab->get_par()->set_image_hints( extended );
  convert2lab->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( extended );
  VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
  if( !labimg ) {
    std::cout<<"ImpulseNRPar::build(): null Lab image"<<std::endl;
    PF_UNREF( extended, "ImageReaderPar::build(): extended unref after null lab image" );
    PF_REF( in[0], "ImpulseNRPar::build(): null Lab image" );
    return in[0];
  }
  PF_UNREF( extended, "ImageReaderPar::build(): extended unref after convert2lab" );
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  extended->Xsize="<<extended->Xsize<<std::endl;


  NonLocalMeans_DTAlgo_Par* impnrpar = dynamic_cast<NonLocalMeans_DTAlgo_Par*>( nlmeans_algo->get_par() );
  if( !impnrpar ) {
    std::cout<<"ImpulseNRPar::build(): could not get pointer to impulse NR operation"<<std::endl;
    PF_UNREF( extended, "ImpulseNRPar::build(): failed to get pointer to impulse NR operation" );
    PF_REF( in[0], "ImpulseNRPar::build(): failed to get pointer to impulse NR operation" );
    return in[0];
  }
  impnrpar->set_P( P );
  impnrpar->set_K( K );
  impnrpar->set_sharpness( sharpness );
  impnrpar->set_luma_frac( luma_frac.get() );
  impnrpar->set_chroma_frac( chroma_frac.get() );
  in2.clear();
  in2.push_back(labimg);
  nlmeans_algo->get_par()->set_image_hints( labimg );
  nlmeans_algo->get_par()->set_format( get_format() );
  VipsImage* impnrimg = nlmeans_algo->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( labimg, "ImpulseNRPar::build(): labimg unref" );


  // Final cropping to remove the padding pixels
  VipsImage* cropped;
#ifndef NDEBUG
  std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  impnrimg->Xsize="<<impnrimg->Xsize<<std::endl;
  std::cout<<"srcimg->Ysize="<<srcimg->Ysize<<"  impnrimg->Ysize="<<impnrimg->Ysize<<std::endl;
#endif
  if( vips_crop(impnrimg, &cropped, get_padding(), get_padding(),
      impnrimg->Xsize-2*get_padding(), impnrimg->Ysize-2*get_padding(), NULL) ) {
    std::cout<<"GaussBlurPar::build(): vips_crop() failed."<<std::endl;
    PF_UNREF( impnrimg, "GaussBlurPar::build(): out unref" );
    PF_REF( in[0], "ImpulseNRPar::build(): vips_crop() failed" );
    return in[0];
  }
  PF_UNREF( impnrimg, "GaussBlurPar::build(): out unref" );
#ifndef NDEBUG
  std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  cropped->Xsize="<<cropped->Xsize<<std::endl;
  std::cout<<"srcimg->Ysize="<<srcimg->Ysize<<"  cropped->Ysize="<<cropped->Ysize<<std::endl;
#endif

  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2input->get_par() );
  //std::cout<<"ImageArea::update(): icc_par="<<icc_par<<std::endl;
  if( icc_par ) {
    //std::cout<<"ImageArea::update(): setting display profile: "<<current_display_profile<<std::endl;
    icc_par->set_out_profile( in_profile );
  }
  convert2input->get_par()->set_image_hints( cropped );
  convert2input->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( cropped );
#ifndef NDEBUG
  std::cout<<"NonLocalmeansPar::build(): calling convert2input->get_par()->build()"<<std::endl;
#endif
  VipsImage* out = convert2input->get_par()->build(in2, 0, NULL, NULL, level );
  PF_UNREF( cropped, "ImageArea::update() cropped unref" );


  set_image_hints( out );

  return out;
}


PF::ProcessorBase* PF::new_nlmeans()
{
  return( new PF::Processor<PF::NonLocalMeansPar,PF::NonLocalMeansProc>() );
}
