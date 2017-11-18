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
//#include "../base/new_operation.hh"
#include "icc_transform.hh"
#include "convert_colorspace.hh"

#include "desaturate_luminance.hh"

#include "desaturate.hh"

PF::DesaturatePar::DesaturatePar(): 
  OpParBase(),
  method( "method", this, PF::DESAT_LAB, "DESAT_LAB",  _("luminance (LCh)") )
{
  method.add_enum_value( PF::DESAT_LIGHTNESS, "DESAT_LIGHTNESS", _("lightness (HSL)") );
  method.add_enum_value( PF::DESAT_AVERAGE, "DESAT_AVERAGE", _("average") );
  //method.add_enum_value( PF::DESAT_LUMINOSITY, "DESAT_LUMINOSITY", _("Luminosity") );

  proc_luminosity = PF::new_desaturate_luminosity();
  proc_luminance = PF::new_desaturate_luminance();
  proc_lightness = PF::new_desaturate_lightness();
  proc_average = PF::new_desaturate_average();
  proc_average2 = PF::new_desaturate_average();
  //convert2lab = PF::new_operation( "convert2lab", NULL );
  convert_cs = PF::new_icc_transform();
  convert2lab = PF::new_convert_colorspace();
  PF::ConvertColorspacePar* csconvpar = dynamic_cast<PF::ConvertColorspacePar*>(convert2lab->get_par());
  if(csconvpar) {
    csconvpar->set_out_profile_mode( PF::PROF_MODE_DEFAULT );
    csconvpar->set_out_profile_type( PF::PROF_TYPE_LAB );
  }
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
    if( proc_luminance->get_par() ) {
      proc_luminance->get_par()->set_image_hints( in[0] );
      proc_luminance->get_par()->set_format( get_format() );
      out = proc_luminance->get_par()->build( in, first, imap, omap, level );
    }
    break;
  }

  return out;
}
