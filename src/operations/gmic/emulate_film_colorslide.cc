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
#include "emulate_film_colorslide.hh"



PF::GmicEmulateFilmColorslidePar::GmicEmulateFilmColorslidePar(): 
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
  prop_preset.add_enum_value( 1, "Agfa_Precisa_100", "Agfa Precisa 100" );
  prop_preset.add_enum_value( 2, "Fuji_Astia_100F", "Fuji Astia 100F" );
  prop_preset.add_enum_value( 3, "Fuji_FP_100C", "Fuji FP 100C" );
  prop_preset.add_enum_value( 4, "Fuji_Provia_100F", "Fuji Provia 100F" );
  prop_preset.add_enum_value( 5, "Fuji_Provia_400F", "Fuji Provia 400F" );
  prop_preset.add_enum_value( 6, "Fuji_Provia_400X", "Fuji Provia 400X" );
  prop_preset.add_enum_value( 7, "Fuji_Sensia_100", "Fuji Sensia 100" );
  prop_preset.add_enum_value( 8, "Fuji_Superia_200_XPRO", "Fuji Superia 200 XPRO" );
  prop_preset.add_enum_value( 9, "Fuji_Velvia_50", "Fuji Velvia 50" );
  prop_preset.add_enum_value( 10, "Generic_Fuji_Astia_100", "Generic Fuji Astia 100" );
  prop_preset.add_enum_value( 11, "Generic_Fuji_Provia_100", "Generic Fuji Provia 100" );
  prop_preset.add_enum_value( 12, "Generic_Fuji_Velvia_100", "Generic Fuji Velvia 100" );
  prop_preset.add_enum_value( 13, "Generic_Kodachrome_64", "Generic Kodachrome 64" );
  prop_preset.add_enum_value( 14, "Generic_Kodak_Ektachrome_100_VS", "Generic Kodak Ektachrome 100 VS" );
  prop_preset.add_enum_value( 15, "Kodak_E_100_GX_Ektachrome_100", "Kodak E-100 GX Ektachrome 100" );
  prop_preset.add_enum_value( 16, "Kodak_Ektachrome_100_VS", "Kodak Ektachrome 100 VS" );
  prop_preset.add_enum_value( 17, "Kodak_Elite_Chrome_200", "Kodak Elite Chrome 200" );
  prop_preset.add_enum_value( 18, "Kodak_Elite_Chrome_400", "Kodak Elite Chrome 400" );
  prop_preset.add_enum_value( 19, "Kodak_Elite_ExtraColor_100", "Kodak Elite ExtraColor 100" );
  prop_preset.add_enum_value( 20, "Kodak_Kodachrome_200", "Kodak Kodachrome 200" );
  prop_preset.add_enum_value( 21, "Kodak_Kodachrome_25", "Kodak Kodachrome 25" );
  prop_preset.add_enum_value( 22, "Kodak_Kodachrome_64", "Kodak Kodachrome 64" );
  prop_preset.add_enum_value( 23, "Lomography_X_Pro_Slide_200", "Lomography X-Pro Slide 200" );
  prop_preset.add_enum_value( 24, "Polaroid_669", "Polaroid 669" );
  prop_preset.add_enum_value( 25, "Polaroid_690", "Polaroid 690" );
  prop_preset.add_enum_value( 26, "Polaroid_Polachrome", "Polaroid Polachrome" );
  set_type( "gmic_emulate_film_colorslide" );
}


int PF::GmicEmulateFilmColorslidePar::get_padding( int level )
{
  return 0;
}


VipsImage* PF::GmicEmulateFilmColorslidePar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_emulate_film_colorslide  ";
  //std::string command = "-v + -v + -v + -e $_path_rc -gimp_emulate_film_colorslide  ";
  //std::string command = "-v + -v + -v + -gimp_emulate_film_colorslide  ";
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


PF::ProcessorBase* PF::new_gmic_emulate_film_colorslide()
{
  return( new PF::Processor<PF::GmicEmulateFilmColorslidePar,PF::GmicEmulateFilmColorslideProc>() );
}
