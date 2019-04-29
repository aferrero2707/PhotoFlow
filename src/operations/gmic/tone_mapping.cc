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


#include "../../base/processor_imp.hh"
#include "gmic.hh"
#include "tone_mapping.hh"



PF::GmicToneMappingPar::GmicToneMappingPar(): 
  GmicUntiledOperationPar(),
  prop_threshold("threshold",this,0.5),
  prop_gamma("gamma",this,0.7),
  prop_smoothness("smoothness",this,0.1),
  prop_iterations("iterations",this,30),
  prop_channels("channels", this, 3, "Luminance", "Luminance")
{	
  prop_channels.add_enum_value( 0, "All", "All" );
  prop_channels.add_enum_value( 1, "RGBA", "RGBA" );
  prop_channels.add_enum_value( 2, "RGB", "RGB" );
  prop_channels.add_enum_value( 4, "Blue_red_chrominances", "Blue/red chrominances" );
  prop_channels.add_enum_value( 5, "Blue_chrominance", "Blue chrominance" );
  prop_channels.add_enum_value( 6, "Red_chrominance", "Red chrominance" );
  prop_channels.add_enum_value( 7, "Lightness", "Lightness" );
  prop_channels.add_enum_value( 8, "ab_components", "ab-components" );
  prop_channels.add_enum_value( 9, "a_component", "a-component" );
  prop_channels.add_enum_value( 10, "b_component", "b-component" );
  prop_channels.add_enum_value( 11, "Hue", "Hue" );
  prop_channels.add_enum_value( 12, "Saturation", "Saturation" );
  prop_channels.add_enum_value( 13, "Value", "Value" );
  prop_channels.add_enum_value( 14, "Key", "Key" );
  prop_channels.add_enum_value( 15, "Green_chrominance", "Green chrominance" );
  prop_channels.add_enum_value( 16, "ch_components", "ch-components" );
  prop_channels.add_enum_value( 17, "c_component", "c-component" );
  prop_channels.add_enum_value( 18, "h_component", "h-component" );
  set_type( "gmic_tone_mapping" );
}


VipsImage* PF::GmicToneMappingPar::build(std::vector<VipsImage*>& in, int first, 
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) return NULL;
  
  update_raster_images();
  PF::RasterImage* raster_image = get_raster_image(0);
  //if( !raster_image || (raster_image->get_file_name () != get_cache_file_name()) ) {
  if( !raster_image ) {
    std::string tempfile = save_image( srcimg, VIPS_FORMAT_FLOAT );

    std::string command = "-verbose + ";
    command = command + "-input " + tempfile + " -n 0,255 -gimp_map_tones ";
    command = command + prop_threshold.get_str();
    command = command + std::string(",") + prop_gamma.get_str();
    command = command + std::string(",") + prop_smoothness.get_str();
    command = command + std::string(",") + prop_iterations.get_str();
    command = command + std::string(",") + prop_channels.get_enum_value_str();
    command = command + " -n 0,1 -output " + get_cache_file_name(0) + ",float,lzw";
    
    run_gmic( srcimg, command );

    unlink( tempfile.c_str() );
  }
  std::vector<VipsImage*> outvec = get_output( level );
  VipsImage* out = (outvec.size()>0) ? outvec[0] : NULL;

  return out;
}


PF::ProcessorBase* PF::new_gmic_tone_mapping()
{
  return( new PF::Processor<PF::GmicToneMappingPar,PF::GmicToneMappingProc>() );
}
