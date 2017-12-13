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

#include "../base/processor.hh"
#include "operations.hh"
#include "icc_transform.hh"
#include "convert_colorspace.hh"

#include "desaturate_luminance.hh"

PF::DesaturateLuminancePar::DesaturateLuminancePar(): OpParBase()
{
  proc_average = PF::new_desaturate_average();
  //convert2lab = PF::new_operation( "convert2lab", NULL );
  convert_cs = PF::new_icc_transform();
  convert2lab = PF::new_convert_colorspace();
  PF::ConvertColorspacePar* csconvpar = dynamic_cast<PF::ConvertColorspacePar*>(convert2lab->get_par());
  if(csconvpar) {
    csconvpar->set_out_profile_mode( PF::PROF_MODE_DEFAULT );
    csconvpar->set_out_profile_type( PF::PROF_TYPE_LAB );
  }
  set_type( "desaturate_luminance" );

  set_default_name( _("desaturate (luminance)") );
}



VipsImage* PF::DesaturateLuminancePar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;
  void *profile_data;
  size_t profile_length;
  VipsImage* srcimg = in[0];
  if( !srcimg ) return NULL;
  profile = PF::get_icc_profile( srcimg );

  // No Lab conversion possible if the input image has no ICC profile
  if( !profile ) {
    std::cout<<"DesaturateLuminancePar::build(): no profile data"<<std::endl;
    PF_REF(srcimg, "DesaturateLuminancePar::build(): srcimg ref when no profile data");
    return srcimg;
  }

  set_image_hints( srcimg );
  out = PF::OpParBase::build( in, first, imap, omap, level );
  return out;

/*
  std::vector<VipsImage*> in2;

  std::cout<<"DesaturateLuminancePar::build(): DESAT_LAB"<<std::endl;
  convert2lab->get_par()->set_image_hints( srcimg );
  convert2lab->get_par()->set_format( get_format() );
  convert2lab->get_par()->lab_image( get_xsize(), get_ysize() );
  in2.clear(); in2.push_back( srcimg );
  VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
  if( !labimg ) {
    std::cout<<"DesaturateLuminancePar::build(): null lab image"<<std::endl;
    PF_REF(srcimg, "DesaturateLuminancePar::build(): srcimg ref when null lab image");
    return srcimg;
  }

  in2.clear(); in2.push_back( labimg );
  proc_average->get_par()->set_image_hints( labimg );
  proc_average->get_par()->set_format( get_format() );
  VipsImage* greyimg = proc_average->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( labimg, "ClonePar::L2rgb(): labimg unref" );
  //VipsImage* greyimg = labimg;

  in2.clear(); in2.push_back( greyimg );
  convert_cs->get_par()->set_image_hints( greyimg );
  convert_cs->get_par()->set_format( get_format() );

  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert_cs->get_par() );
  if( icc_par ) {
    icc_par->set_out_profile( profile );
  }

  VipsImage* rgbimg = convert_cs->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( greyimg, "ClonePar::L2rgb(): greyimg unref" );

  out = rgbimg;

  return out;
*/
}
