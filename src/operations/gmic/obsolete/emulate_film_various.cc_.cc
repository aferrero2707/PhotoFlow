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
#include "emulate_film_various.hh"



PF::GmicEmulateFilmVariousPar::GmicEmulateFilmVariousPar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_preset("preset", this, 0, "None", "None"),
  prop_opacity("opacity",this,100),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,0),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0)
{	
  gmic = PF::new_gmic();
  prop_preset.add_enum_value( 1, "60's", "60's" );
  prop_preset.add_enum_value( 2, "60's_(faded)", "60's (faded)" );
  prop_preset.add_enum_value( 3, "60's_(faded_alt)", "60's (faded alt)" );
  prop_preset.add_enum_value( 4, "Black_&_White", "Black & White" );
  prop_preset.add_enum_value( 5, "Color_(rich)", "Color (rich)" );
  prop_preset.add_enum_value( 6, "Faded", "Faded" );
  prop_preset.add_enum_value( 7, "Faded_(alt)", "Faded (alt)" );
  prop_preset.add_enum_value( 8, "Faded_(analog)", "Faded (analog)" );
  prop_preset.add_enum_value( 9, "Faded_(extreme)", "Faded (extreme)" );
  prop_preset.add_enum_value( 10, "Faded_(vivid)", "Faded (vivid)" );
  prop_preset.add_enum_value( 11, "Hong_Kong", "Hong Kong" );
  prop_preset.add_enum_value( 12, "Light_(blown)", "Light (blown)" );
  prop_preset.add_enum_value( 13, "Lomo", "Lomo" );
  prop_preset.add_enum_value( 14, "Natural_(vivid)", "Natural (vivid)" );
  prop_preset.add_enum_value( 15, "Nostalgic", "Nostalgic" );
  prop_preset.add_enum_value( 16, "Purple", "Purple" );
  prop_preset.add_enum_value( 17, "Retro", "Retro" );
  prop_preset.add_enum_value( 18, "Summer", "Summer" );
  prop_preset.add_enum_value( 19, "Summer_(alt)", "Summer (alt)" );
  prop_preset.add_enum_value( 20, "Sunny", "Sunny" );
  prop_preset.add_enum_value( 21, "Sunny_(alt)", "Sunny (alt)" );
  prop_preset.add_enum_value( 22, "Sunny_(warm)", "Sunny (warm)" );
  prop_preset.add_enum_value( 23, "Sunny_(rich)", "Sunny (rich)" );
  prop_preset.add_enum_value( 24, "Super_warm", "Super warm" );
  prop_preset.add_enum_value( 25, "Super_warm_(rich)", "Super warm (rich)" );
  prop_preset.add_enum_value( 26, "Sutro_FX", "Sutro FX" );
  prop_preset.add_enum_value( 27, "Vibrant", "Vibrant" );
  prop_preset.add_enum_value( 28, "Vibrant_(alien)", "Vibrant (alien)" );
  prop_preset.add_enum_value( 29, "Vintage", "Vintage" );
  prop_preset.add_enum_value( 30, "Vintage_(alt)", "Vintage (alt)" );
  prop_preset.add_enum_value( 31, "Vintage_(brighter)", "Vintage (brighter)" );
  prop_preset.add_enum_value( 32, "Warm", "Warm" );
  prop_preset.add_enum_value( 33, "Warm_(yellow)", "Warm (yellow)" );
  set_type( "gmic_emulate_film_various" );
}


VipsImage* PF::GmicEmulateFilmVariousPar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_emulate_film_various  ";
  command = command + prop_preset.get_enum_value_str();
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_brightness.get_str();
  command = command + std::string(",") + prop_contrast.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_hue.get_str();
  command = command + std::string(",") + prop_saturation.get_str();
  command = command + std::string(",") + prop_post_normalize.get_str();
  gpar->set_command( command.c_str() );
  gpar->set_iterations( iterations.get() );
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


PF::ProcessorBase* PF::new_gmic_emulate_film_various()
{
  return( new PF::Processor<PF::GmicEmulateFilmVariousPar,PF::GmicEmulateFilmVariousProc>() );
}
