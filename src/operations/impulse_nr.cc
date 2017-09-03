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
#include <string.h>
#include <cstring>

#include "gaussblur.hh"
#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "impulse_nr.hh"


PF::ImpulseNRPar::ImpulseNRPar():
  OpParBase(), 
  threshold("threshold",this,0.5),
  in_profile( NULL )
{

  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert2input = new_icc_transform();
  gauss_blur = new_gaussblur();
  impulse_nr_algo = new PF::Processor<PF::ImpulseNR_RTAlgo_Par,PF::ImpulseNR_RTAlgo_Proc>();

  set_type("impulse_nr" );

  set_default_name( _("impulse NR") );
}


VipsImage* PF::ImpulseNRPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  //std::cout<<"ImpulseNRPar::build(): in[0]="<<in[0]<<std::endl;
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;
  VipsImage* srcimg = in[0];

  in_profile = PF::get_icc_profile( in[first] );

  std::vector<VipsImage*> in2;

  // Extend the image by two pixels to account for the pixel averaging window
  // of the impulse noise reduction algorithm
  VipsImage* extended;
  VipsExtend extend = VIPS_EXTEND_COPY;
  if( vips_embed(srcimg, &extended, 2, 2,
      srcimg->Xsize+4, srcimg->Ysize+4,
      "extend", extend, NULL) ) {
    std::cout<<"ImpulseNRPar::build(): vips_embed() failed."<<std::endl;
    PF_REF( in[0], "ImpulseNRPar::build(): vips_embed() failed." );
    return in[0];
  }

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


  VipsImage* lch;
  int nbands = 1;
  if( vips_extract_band( labimg, &lch, 0, "n", nbands, NULL ) ) {
    std::cout<<"ImpulseNRPar::build(): vips_extract_band() failed"<<std::endl;
    PF_UNREF( labimg, "ImpulseNRPar::build(): vips_extract_band() failed" );
    PF_REF( in[0], "ImpulseNRPar::build(): vips_extract_band() failed" );
    return in[0];
  }

  vips_image_init_fields( lch,
                          labimg->Xsize, labimg->Ysize,
                          nbands, labimg->BandFmt,
                          srcimg->Coding,
                          labimg->Type,
                          1.0, 1.0);
  PF_UNREF( labimg, "ImageReaderPar::build(): labimg unref after extract_band" );


  GaussBlurPar* gausspar = dynamic_cast<GaussBlurPar*>( gauss_blur->get_par() );
  if( !gausspar ) {
    std::cout<<"ImpulseNRPar::build(): could not get pointer to gaussian blur operation"<<std::endl;
    PF_UNREF( lch, "ImpulseNRPar::build(): failed to get pointer to gaussian blur operation" );
    PF_REF( in[0], "ImpulseNRPar::build(): failed to get pointer to gaussian blur operation" );
    return in[0];
  }

  float thr = threshold.get()/20.f;
  float radius = MAX( 2.0,(thr-1.0f) );
  gausspar->set_radius( radius );
  gausspar->set_image_hints( lch );
  gausspar->set_format( get_format() );
  in2.clear(); in2.push_back( lch );
  VipsImage* smoothed = gausspar->build( in2, 0, NULL, NULL, level );
  if( !smoothed ) {
    std::cout<<"ImpulseNRPar::build(): gauss_blur failed"<<std::endl;
    PF_REF( lch, "ImpulseNRPar::build(): gauss_blur failed" );
    PF_REF( in[0], "ImpulseNRPar::build(): gauss_blur failed" );
    return in[0];
  }
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  smoothed->Xsize="<<smoothed->Xsize<<std::endl;

  ImpulseNR_RTAlgo_Par* impnrpar = dynamic_cast<ImpulseNR_RTAlgo_Par*>( impulse_nr_algo->get_par() );
  if( !impnrpar ) {
    std::cout<<"ImpulseNRPar::build(): could not get pointer to impulse NR operation"<<std::endl;
    PF_UNREF( lch, "ImpulseNRPar::build(): failed to get pointer to impulse NR operation" );
    PF_UNREF( smoothed, "ImpulseNRPar::build(): failed to get pointer to impulse NR operation" );
    PF_REF( in[0], "ImpulseNRPar::build(): failed to get pointer to impulse NR operation" );
    return in[0];
  }
  impnrpar->set_threshold( threshold.get()/20.f );
  in2.clear();
  in2.push_back(extended);
  in2.push_back(lch);
  in2.push_back(smoothed);
  impulse_nr_algo->get_par()->set_image_hints( extended );
  impulse_nr_algo->get_par()->set_format( get_format() );
  VipsImage* impnrimg = impulse_nr_algo->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( lch, "ImpulseNRPar::build(): lch unref" );
  PF_UNREF( smoothed, "ImpulseNRPar::build(): smoothed unref" );

  // Final cropping to remove the padding pixels
  VipsImage* cropped;
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  impnrimg->Xsize="<<impnrimg->Xsize<<std::endl;
  if( vips_crop(impnrimg, &cropped, 2, 2,
      srcimg->Xsize, srcimg->Ysize, NULL) ) {
    std::cout<<"GaussBlurPar::build(): vips_crop() failed."<<std::endl;
    PF_UNREF( impnrimg, "GaussBlurPar::build(): impnrimg unref" );
    PF_REF( in[0], "ImpulseNRPar::build(): vips_crop() failed" );
    return in[0];
  }
  PF_UNREF( impnrimg, "GaussBlurPar::build(): impnrimg unref" );
  //std::cout<<"srcimg->Xsize="<<srcimg->Xsize<<"  cropped->Xsize="<<cropped->Xsize<<std::endl;


  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2input->get_par() );
  //std::cout<<"ImageArea::update(): icc_par="<<icc_par<<std::endl;
  if( icc_par ) {
    //std::cout<<"ImageArea::update(): setting display profile: "<<current_display_profile<<std::endl;
    icc_par->set_out_profile( in_profile );
  }
  convert2input->get_par()->set_image_hints( cropped );
  convert2input->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( cropped );
  //std::cout<<"ImpulseNRPar::build(): calling convert2input->get_par()->build()"<<std::endl;
  VipsImage* out = convert2input->get_par()->build(in2, 0, NULL, NULL, level );
  PF_UNREF( cropped, "ImpulseNRPar::update() cropped unref" );


  set_image_hints( out );

  //VipsImage* out = cropped;
  //std::cout<<"ImpulseNRPar::build(): out="<<out<<std::endl;

  return out;
}


PF::ProcessorBase* PF::new_impulse_nr()
{
  return new PF::Processor<PF::ImpulseNRPar,PF::ImpulseNRProc>();
}
