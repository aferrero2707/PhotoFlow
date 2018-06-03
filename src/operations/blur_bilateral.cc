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
PF::OpParBase()
{
  balgo = new PF::Processor<PF::BlurBilateralAlgoPar,PF::BlurBilateralAlgoProc>();
  map_properties( balgo->get_par()->get_properties() );
  set_type( "blur_bilateral" );
  set_default_name( _("blilateral blur") );
}


//void set_iterations( int i ) { iterations.set( i ); }
void PF::BlurBilateralPar::set_sigma_s( float s )
{
  BlurBilateralAlgoPar* bop = dynamic_cast<BlurBilateralAlgoPar*>( balgo->get_par() );
  if(bop) bop->set_sigma_s( s );
}
void PF::BlurBilateralPar::set_sigma_r( float s )
{
  BlurBilateralAlgoPar* bop = dynamic_cast<BlurBilateralAlgoPar*>( balgo->get_par() );
  if(bop) bop->set_sigma_r( s );
}



VipsImage* PF::BlurBilateralPar::build(std::vector<VipsImage*>& in, int first,
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
  std::cout<<"BlurBilateralPar::build(): csin = "<<csin<<std::endl;

  int padding = get_padding(0);
  // Extend the original image by padding pixels
  VipsImage* extended;
  VipsExtend extend = VIPS_EXTEND_COPY;
  if( vips_embed(srcimg, &extended, padding, padding,
      srcimg->Xsize+padding*2, srcimg->Ysize+padding*2,
      "extend", extend, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_embed() failed."<<std::endl;
    PF_REF( in[0], "BlurBilateralPar::build(): vips_embed() failed." );
    return in[0];
  }

  BlurBilateralAlgoPar* bop = dynamic_cast<BlurBilateralAlgoPar*>( balgo->get_par() );
  balgo->get_par()->set_image_hints( extended );
  balgo->get_par()->set_format( get_format() );
  std::vector<VipsImage*> in2;
  in2.clear();
  in2.push_back(extended);

  VipsImage* blurred = balgo->get_par()->build( in2, 0, imap, omap, level );
  PF_UNREF( extended, "BlurBilateralPar::build(): extended unref" );


  // Final cropping to remove the padding pixels
  VipsImage* cropped;
  if( vips_crop(blurred, &cropped, padding, padding,
      srcimg->Xsize, srcimg->Ysize, NULL) ) {
    std::cout<<"BlurBilateralPar::build(): vips_crop() failed."<<std::endl;
    PF_UNREF( blurred, "BlurBilateralPar::build(): blurred unref" );
    PF_REF( in[0], "BlurBilateralPar::build(): vips_crop() failed" );
    return in[0];
  }
  PF_UNREF( blurred, "BlurBilateralPar::build(): blurred unref" );

  return cropped;
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
