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


#include "gmic.hh"
#include "tone_mapping.hh"



PF::GmicToneMappingPar::GmicToneMappingPar(): 
OpParBase(),
  prop_threshold("threshold",this,0.5),
  prop_gamma("gamma",this,0.7),
  prop_smoothness("smoothness",this,0.1),
  prop_iterations("iterations",this,30),
prop_channels("channels", this, 3, "Luminance", "Luminance"),
  prop_padding("padding",this,20)
{	
  gmic = PF::new_gmic();
  prop_channels.add_enum_value( 0, "All", "All" );
  prop_channels.add_enum_value( 1, "RGBA", "RGBA" );
  prop_channels.add_enum_value( 2, "RGB", "RGB" );
  prop_channels.add_enum_value( 4, "Blue/red_chrominances", "Blue/red chrominances" );
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


int PF::GmicToneMappingPar::get_padding( int level )
{
  int scale = 1;
  for( int i = 1; i < level; i++ ) scale *= 2;
  return( prop_padding.get()/scale );
}


VipsImage* PF::GmicToneMappingPar::build(std::vector<VipsImage*>& in, int first, 
                                        VipsImage* imap, VipsImage* omap, 
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;
  
  if( !(gmic->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
	for( int l = 1; l <= level; l++ )
		scalefac *= 2;

  std::string command = "-verbose + -gimp_map_tones  ";
  command = command + prop_threshold.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_smoothness.get_str();
  command = command + std::string(",") + prop_iterations.get_str();
  command = command + std::string(",") + prop_channels.get_enum_value_str();
  gpar->set_command( command.c_str() );
  gpar->set_iterations( 1 );
  gpar->set_padding( get_padding( level ) );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
}


PF::ProcessorBase* PF::new_gmic_tone_mapping()
{
  return( new PF::Processor<PF::GmicToneMappingPar,PF::GmicToneMappingProc>() );
}
