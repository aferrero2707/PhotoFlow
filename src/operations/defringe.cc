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

#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "gaussblur.hh"
#include "defringe.hh"


PF::DefringePar::DefringePar():
OpParBase(),
op_mode("op_mode",this,PF::MODE_GLOBAL_AVERAGE,"MODE_GLOBAL_AVERAGE","global average (fast)"),
radius("radius",this,4.0),
threshold("threshold",this,20.0),
in_profile( NULL )
{
  op_mode.add_enum_value(PF::MODE_LOCAL_AVERAGE,"MODE_LOCAL_AVERAGE","local average (slow)");
  op_mode.add_enum_value(PF::MODE_STATIC,"MODE_STATIC","static threshold (fast)");

  gauss = new_gaussblur();
  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert2input = new_icc_transform();

  set_type("defringe" );

  set_default_name( _("defringe") );
}


bool PF::DefringePar::needs_caching()
{
  return false;
}

VipsImage* PF::DefringePar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;

  std::vector<VipsImage*> in2;

  void *data;
  size_t data_length;
  if( in_profile ) cmsCloseProfile( in_profile );
  in_profile = NULL;
  if( !vips_image_get_blob( in[0], VIPS_META_ICC_NAME, &data, &data_length ) ) {
    in_profile = cmsOpenProfileFromMem( data, data_length );
  }

  convert2lab->get_par()->set_image_hints( in[0] );
  convert2lab->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( in[0] );
  VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
  if( !labimg ) {
    std::cout<<"DefringePar::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "DefringePar::build(): null Lab image" );
    return in[0];
  }

  VipsImage* out = NULL;
  VipsImage* blurred = NULL;

  GaussBlurPar* gausspar = dynamic_cast<GaussBlurPar*>( gauss->get_par() );
  if( gausspar ) {
    gausspar->set_radius( radius.get() );
    gausspar->set_image_hints( labimg );
    gausspar->set_format( get_format() );
    in2.clear(); in2.push_back( labimg );
    blurred = gausspar->build( in2, 0, NULL, NULL, level );
    PF_UNREF( labimg, "ImageReaderPar::build(): extended unref after convert2lab" );
  }

  if( !blurred ) {
    std::cout<<"DefringePar::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "DefringePar::build(): null Lab image" );
    return in[0];
  }

  in2.clear();
  in2.push_back(labimg);
  in2.push_back(blurred);
  VipsImage* defr = OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( blurred, "ImageArea::update() blurred unref" );

  PF::ICCTransformPar* icc_par = dynamic_cast<PF::ICCTransformPar*>( convert2input->get_par() );
  if( icc_par ) {
    icc_par->set_out_profile( in_profile );
  }
  convert2input->get_par()->set_image_hints( defr );
  convert2input->get_par()->set_format( get_format() );
  in2.clear(); in2.push_back( defr );
  std::cout<<"DefringePar::build(): calling convert2input->get_par()->build()"<<std::endl;
  out = convert2input->get_par()->build(in2, 0, NULL, NULL, level );
  PF_UNREF( defr, "ImageArea::update() cropped unref" );


  set_image_hints( in[0] );

  return out;
}


PF::ProcessorBase* PF::new_defringe()
{
  return new PF::Processor<PF::DefringePar,PF::DefringeProc>();
}
