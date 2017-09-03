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
#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "desaturate.hh"

PF::DesaturatePar::DesaturatePar(): 
  OpParBase(),
  method( "method", this, PF::DESAT_LAB, "DESAT_LAB",  _("Luminance") )
{
  method.add_enum_value( PF::DESAT_LIGHTNESS, "DESAT_LIGHTNESS", _("Lightness") );
  method.add_enum_value( PF::DESAT_AVERAGE, "DESAT_AVERAGE", _("Average") );
  //method.add_enum_value( PF::DESAT_LUMINOSITY, "DESAT_LUMINOSITY", _("Luminosity") );

  proc_luminosity = PF::new_desaturate_luminosity();
  proc_lightness = PF::new_desaturate_lightness();
  proc_average = PF::new_desaturate_average();
  proc_average2 = PF::new_desaturate_average();
  convert2lab = PF::new_operation( "convert2lab", NULL );
  convert_cs = PF::new_icc_transform();
  set_type( "desaturate" );

  set_default_name( _("desaturate") );
}



VipsImage* PF::DesaturatePar::build(std::vector<VipsImage*>& in, int first, 
                                    VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* out = NULL;
  switch( method.get_enum_value().first ) {
  case PF::DESAT_LUMINOSITY:
    if( proc_luminosity->get_par() ) {
      proc_luminosity->get_par()->set_image_hints( in[0] );
      proc_luminosity->get_par()->set_format( get_format() );
      out = proc_luminosity->get_par()->build( in, first, imap, omap, level );
    }
    break;
  case PF::DESAT_LIGHTNESS:
    if( proc_lightness->get_par() ) {
      proc_lightness->get_par()->set_image_hints( in[0] );
      proc_lightness->get_par()->set_format( get_format() );
      out = proc_lightness->get_par()->build( in, first, imap, omap, level );
    }
    break;
  case PF::DESAT_AVERAGE:
    if( proc_average->get_par() ) {
      proc_average->get_par()->set_image_hints( in[0] );
      proc_average->get_par()->set_format( get_format() );
      out = proc_average->get_par()->build( in, first, imap, omap, level );
    }
    break;
  case PF::DESAT_LAB:
    {
      void *profile_data;
      size_t profile_length;
      VipsImage* srcimg = in[0];
      if( !srcimg ) return NULL;
      PF::ICCProfile* iccprof_in = PF::get_icc_profile( srcimg );

      // No Lab conversion possible if the input image has no ICC profile
      if( !iccprof_in ) {
        std::cout<<"DesaturatePar::build(): no profile data"<<std::endl;
        return NULL;
      }
  
      std::vector<VipsImage*> in2; 

      convert2lab->get_par()->set_image_hints( srcimg );
      convert2lab->get_par()->set_format( get_format() );
      in2.clear(); in2.push_back( srcimg );
      VipsImage* labimg = convert2lab->get_par()->build( in2, 0, NULL, NULL, level );
      if( !labimg ) {
        std::cout<<"DesaturatePar::build(): null lab image"<<std::endl;
        return NULL;
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
        icc_par->set_out_profile( iccprof_in );
      }

      VipsImage* rgbimg = convert_cs->get_par()->build( in2, 0, NULL, NULL, level );
      PF_UNREF( greyimg, "ClonePar::L2rgb(): greyimg unref" );

      out = rgbimg;

      if( false || get_render_mode() == PF_RENDER_PREVIEW ) {
        // Not valid anymore, since we now process the preview in floating point precision

        // We have to circumvent the fact that in 16-bits integer precision,
        // LCMS does not produce a perfectly neutral RGB image
        // in the Lab -> RGB conversion when a=b=0
        in2.clear(); in2.push_back( rgbimg );
        proc_average2->get_par()->set_image_hints( rgbimg );
        proc_average2->get_par()->set_format( get_format() );
        out = proc_average2->get_par()->build( in2, 0, NULL, NULL, level );
        PF_UNREF( rgbimg, "ClonePar::L2rgb(): rgbimg unref" );
      }
    }
    break;
  }

  return out;
}


PF::ProcessorBase* PF::new_desaturate_luminosity()
{
  return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateLuminosity>() );
}


PF::ProcessorBase* PF::new_desaturate_lightness()
{
  return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateLightness>() );
}


PF::ProcessorBase* PF::new_desaturate_average()
{
  return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateAverage>() );
}


PF::ProcessorBase* PF::new_desaturate()
{
  return( new PF::Processor<PF::DesaturatePar,PF::DesaturateProc>() );
}
