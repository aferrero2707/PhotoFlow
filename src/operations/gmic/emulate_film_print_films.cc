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
#include "emulate_film_print_films.hh"



PF::GmicEmulateFilmPrintFilmsPar::GmicEmulateFilmPrintFilmsPar(): 
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
  prop_preset.add_enum_value( 1, "Fuji_3510_(Constlclip)", "Fuji 3510 (Constlclip)" );
  prop_preset.add_enum_value( 2, "Fuji_3510_(Constlmap)", "Fuji 3510 (Constlmap)" );
  prop_preset.add_enum_value( 3, "Fuji_3510_(Cuspclip)", "Fuji 3510 (Cuspclip)" );
  prop_preset.add_enum_value( 4, "Fuji_3513_(Constlclip)", "Fuji 3513 (Constlclip)" );
  prop_preset.add_enum_value( 5, "Fuji_3513_(Constlmap)", "Fuji 3513 (Constlmap)" );
  prop_preset.add_enum_value( 6, "Fuji_3513_(Cuspclip)", "Fuji 3513 (Cuspclip)" );
  prop_preset.add_enum_value( 7, "Kodak_2383_(Constlclip)", "Kodak 2383 (Constlclip)" );
  prop_preset.add_enum_value( 8, "Kodak_2383_(Constlmap)", "Kodak 2383 (Constlmap)" );
  prop_preset.add_enum_value( 9, "Kodak_2383_(Cuspclip)", "Kodak 2383 (Cuspclip)" );
  prop_preset.add_enum_value( 10, "Kodak_2393_(Constlclip)", "Kodak 2393 (Constlclip)" );
  prop_preset.add_enum_value( 11, "Kodak_2393_(Constlmap)", "Kodak 2393 (Constlmap)" );
  prop_preset.add_enum_value( 12, "Kodak_2393_(Cuspclip)", "Kodak 2393 (Cuspclip)" );
  set_type( "gmic_emulate_film_print_films" );
}


int PF::GmicEmulateFilmPrintFilmsPar::get_padding( int level )
{
  return 0;
}


VipsImage* PF::GmicEmulateFilmPrintFilmsPar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_emulate_film_print  ";
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


PF::ProcessorBase* PF::new_gmic_emulate_film_print_films()
{
  return( new PF::Processor<PF::GmicEmulateFilmPrintFilmsPar,PF::GmicEmulateFilmPrintFilmsProc>() );
}
