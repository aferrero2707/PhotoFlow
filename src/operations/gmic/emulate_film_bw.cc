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
#include "emulate_film_bw.hh"



PF::GmicEmulateFilmBEPar::GmicEmulateFilmBEPar(): 
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
  prop_preset.add_enum_value( 1, "Agfa_APX_100", "Agfa APX 100" );
  prop_preset.add_enum_value( 2, "Agfa_APX_25", "Agfa APX 25" );
  prop_preset.add_enum_value( 3, "Fuji_Neopan_1600", "Fuji Neopan 1600" );
  prop_preset.add_enum_value( 4, "Fuji_Neopan_Acros_100", "Fuji Neopan Acros 100" );
  prop_preset.add_enum_value( 5, "Ilford_Delta_100", "Ilford Delta 100" );
  prop_preset.add_enum_value( 6, "Ilford_Delta_3200", "Ilford Delta 3200" );
  prop_preset.add_enum_value( 7, "Ilford_Delta_400", "Ilford Delta 400" );
  prop_preset.add_enum_value( 8, "Ilford_FP4_Plus_125", "Ilford FP4 Plus 125" );
  prop_preset.add_enum_value( 9, "Ilford_HP5_Plus_400", "Ilford HP5 Plus 400" );
  prop_preset.add_enum_value( 10, "Ilford_HPS_800", "Ilford HPS 800" );
  prop_preset.add_enum_value( 11, "Ilford_Pan_F_Plus_50", "Ilford Pan F Plus 50" );
  prop_preset.add_enum_value( 12, "Ilford_XP2", "Ilford XP2" );
  prop_preset.add_enum_value( 13, "Kodak_BW_400_CN", "Kodak BW 400 CN" );
  prop_preset.add_enum_value( 14, "Kodak_HIE_(HS_Infra)", "Kodak HIE (HS Infra)" );
  prop_preset.add_enum_value( 15, "Kodak_T_Max_100", "Kodak T-Max 100" );
  prop_preset.add_enum_value( 16, "Kodak_T_Max_3200", "Kodak T-Max 3200" );
  prop_preset.add_enum_value( 17, "Kodak_T_Max_400", "Kodak T-Max 400" );
  prop_preset.add_enum_value( 18, "Kodak_Tri_X_400", "Kodak Tri-X 400" );
  prop_preset.add_enum_value( 19, "Polaroid_664", "Polaroid 664" );
  prop_preset.add_enum_value( 20, "Polaroid_667", "Polaroid 667" );
  prop_preset.add_enum_value( 21, "Polaroid_672", "Polaroid 672" );
  prop_preset.add_enum_value( 22, "Rollei_IR_400", "Rollei IR 400" );
  prop_preset.add_enum_value( 23, "Rollei_Ortho_25", "Rollei Ortho 25" );
  prop_preset.add_enum_value( 24, "Rollei_Retro_100_Tonal", "Rollei Retro 100 Tonal" );
  prop_preset.add_enum_value( 25, "Rollei_Retro_80s", "Rollei Retro 80s" );
  set_type( "gmic_emulate_film_bw" );
}


VipsImage* PF::GmicEmulateFilmBEPar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_emulate_film_bw  ";
  command = command + prop_preset.get_enum_value_str();
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_brightness.get_str();
  command = command + std::string(",") + prop_contrast.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_hue.get_str();
  command = command + std::string(",") + prop_saturation.get_str();
  command = command + std::string(",") + prop_post_normalize.get_str();
  command = command + std::string(",0");
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


PF::ProcessorBase* PF::new_gmic_emulate_film_bw()
{
  return( new PF::Processor<PF::GmicEmulateFilmBEPar,PF::GmicEmulateFilmBEProc>() );
}
