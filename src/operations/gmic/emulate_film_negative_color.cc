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
#include "emulate_film_negative_color.hh"



PF::GmicEmulateFilmNegativeColorPar::GmicEmulateFilmNegativeColorPar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_preset("preset", this, 0, "None", "None"),
  prop_opacity("opacity",this,1),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,1),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0)
{	
  gmic = PF::new_gmic();
  prop_preset.add_enum_value( 1, "Agfa_Ultra_Color_100", "Agfa Ultra Color 100" );
  prop_preset.add_enum_value( 2, "Agfa_Vista_200", "Agfa Vista 200" );
  prop_preset.add_enum_value( 3, "Fuji_Superia_200", "Fuji Superia 200" );
  prop_preset.add_enum_value( 4, "Fuji_Superia_HG_1600", "Fuji Superia HG 1600" );
  prop_preset.add_enum_value( 5, "Fuji_Superia_Reala_100", "Fuji Superia Reala 100" );
  prop_preset.add_enum_value( 6, "Fuji_Superia_X_Tra_800", "Fuji Superia X-Tra 800" );
  prop_preset.add_enum_value( 7, "Kodak_Elite_100_XPRO", "Kodak Elite 100 XPRO" );
  prop_preset.add_enum_value( 8, "Kodak_Elite_Color_200", "Kodak Elite Color 200" );
  prop_preset.add_enum_value( 9, "Kodak_Elite_Color_400", "Kodak Elite Color 400" );
  prop_preset.add_enum_value( 10, "Kodak_Portra_160_NC", "Kodak Portra 160 NC" );
  prop_preset.add_enum_value( 11, "Kodak_Portra_160_VC", "Kodak Portra 160 VC" );
  prop_preset.add_enum_value( 12, "Lomography_Redscale_100", "Lomography Redscale 100" );
  set_type( "gmic_emulate_film_negative_color" );
}


int PF::GmicEmulateFilmNegativeColorPar::get_padding( int level )
{
  return 0;
}


VipsImage* PF::GmicEmulateFilmNegativeColorPar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_emulate_film_negative_color  ";
  command = command + prop_preset.get_enum_value_str();
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_contrast.get_str();
  command = command + std::string(",") + prop_brightness.get_str();
  command = command + std::string(",") + prop_hue.get_str();
  command = command + std::string(",") + prop_saturation.get_str();
  command = command + std::string(",") + prop_post_normalize.get_str();
  gpar->set_command( command.c_str() );
  gpar->set_iterations( iterations.get() );
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


PF::ProcessorBase* PF::new_gmic_emulate_film_negative_color()
{
  return( new PF::Processor<PF::GmicEmulateFilmNegativeColorPar,PF::GmicEmulateFilmNegativeColorProc>() );
}
