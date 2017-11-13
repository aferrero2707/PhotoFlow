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
#include "emulate_film.hh"



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



PF::GmicEmulateFilmInstantConsumerPar::GmicEmulateFilmInstantConsumerPar():
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
  prop_preset.add_enum_value( 1, "Polaroid_PX_100UV+_Cold___", "Polaroid PX-100UV+ Cold --" );
  prop_preset.add_enum_value( 2, "Polaroid_PX_100UV+_Cold__", "Polaroid PX-100UV+ Cold -" );
  prop_preset.add_enum_value( 3, "Polaroid_PX_100UV+_Cold", "Polaroid PX-100UV+ Cold" );
  prop_preset.add_enum_value( 4, "Polaroid_PX_100UV+_Cold_+", "Polaroid PX-100UV+ Cold +" );
  prop_preset.add_enum_value( 5, "Polaroid_PX_100UV+_Cold_++", "Polaroid PX-100UV+ Cold ++" );
  prop_preset.add_enum_value( 6, "Polaroid_PX_100UV+_Cold_+++", "Polaroid PX-100UV+ Cold +++" );
  prop_preset.add_enum_value( 7, "Polaroid_PX_100UV+_Warm___", "Polaroid PX-100UV+ Warm --" );
  prop_preset.add_enum_value( 8, "Polaroid_PX_100UV+_Warm__", "Polaroid PX-100UV+ Warm -" );
  prop_preset.add_enum_value( 9, "Polaroid_PX_100UV+_Warm", "Polaroid PX-100UV+ Warm" );
  prop_preset.add_enum_value( 10, "Polaroid_PX_100UV+_Warm_+", "Polaroid PX-100UV+ Warm +" );
  prop_preset.add_enum_value( 11, "Polaroid_PX_100UV+_Warm_++", "Polaroid PX-100UV+ Warm ++" );
  prop_preset.add_enum_value( 12, "Polaroid_PX_100UV+_Warm_+++", "Polaroid PX-100UV+ Warm +++" );
  prop_preset.add_enum_value( 13, "Polaroid_PX_680___", "Polaroid PX-680 --" );
  prop_preset.add_enum_value( 14, "Polaroid_PX_680__", "Polaroid PX-680 -" );
  prop_preset.add_enum_value( 15, "Polaroid_PX_680", "Polaroid PX-680" );
  prop_preset.add_enum_value( 16, "Polaroid_PX_680_+", "Polaroid PX-680 +" );
  prop_preset.add_enum_value( 17, "Polaroid_PX_680_++", "Polaroid PX-680 ++" );
  prop_preset.add_enum_value( 18, "Polaroid_PX_680_Cold___", "Polaroid PX-680 Cold --" );
  prop_preset.add_enum_value( 19, "Polaroid_PX_680_Cold__", "Polaroid PX-680 Cold -" );
  prop_preset.add_enum_value( 20, "Polaroid_PX_680_Cold", "Polaroid PX-680 Cold" );
  prop_preset.add_enum_value( 21, "Polaroid_PX_680_Cold_+", "Polaroid PX-680 Cold +" );
  prop_preset.add_enum_value( 22, "Polaroid_PX_680_Cold_++", "Polaroid PX-680 Cold ++" );
  prop_preset.add_enum_value( 23, "Polaroid_PX_680_Cold_++a", "Polaroid PX-680 Cold ++a" );
  prop_preset.add_enum_value( 24, "Polaroid_PX_680_Warm___", "Polaroid PX-680 Warm --" );
  prop_preset.add_enum_value( 25, "Polaroid_PX_680_Warm__", "Polaroid PX-680 Warm -" );
  prop_preset.add_enum_value( 26, "Polaroid_PX_680_Warm", "Polaroid PX-680 Warm" );
  prop_preset.add_enum_value( 27, "Polaroid_PX_680_Warm_+", "Polaroid PX-680 Warm +" );
  prop_preset.add_enum_value( 28, "Polaroid_PX_680_Warm_++", "Polaroid PX-680 Warm ++" );
  prop_preset.add_enum_value( 29, "Polaroid_PX_70___", "Polaroid PX-70 --" );
  prop_preset.add_enum_value( 30, "Polaroid_PX_70__", "Polaroid PX-70 -" );
  prop_preset.add_enum_value( 31, "Polaroid_PX_70", "Polaroid PX-70" );
  prop_preset.add_enum_value( 32, "Polaroid_PX_70_+", "Polaroid PX-70 +" );
  prop_preset.add_enum_value( 33, "Polaroid_PX_70_++", "Polaroid PX-70 ++" );
  prop_preset.add_enum_value( 34, "Polaroid_PX_70_+++", "Polaroid PX-70 +++" );
  prop_preset.add_enum_value( 35, "Polaroid_PX_70_Cold___", "Polaroid PX-70 Cold --" );
  prop_preset.add_enum_value( 36, "Polaroid_PX_70_Cold__", "Polaroid PX-70 Cold -" );
  prop_preset.add_enum_value( 37, "Polaroid_PX_70_Cold", "Polaroid PX-70 Cold" );
  prop_preset.add_enum_value( 38, "Polaroid_PX_70_Cold_+", "Polaroid PX-70 Cold +" );
  prop_preset.add_enum_value( 39, "Polaroid_PX_70_Cold_++", "Polaroid PX-70 Cold ++" );
  prop_preset.add_enum_value( 40, "Polaroid_PX_70_Warm___", "Polaroid PX-70 Warm --" );
  prop_preset.add_enum_value( 41, "Polaroid_PX_70_Warm__", "Polaroid PX-70 Warm -" );
  prop_preset.add_enum_value( 42, "Polaroid_PX_70_Warm", "Polaroid PX-70 Warm" );
  prop_preset.add_enum_value( 43, "Polaroid_PX_70_Warm_+", "Polaroid PX-70 Warm +" );
  prop_preset.add_enum_value( 44, "Polaroid_PX_70_Warm_++", "Polaroid PX-70 Warm ++" );
  prop_preset.add_enum_value( 45, "Polaroid_Time_Zero_(Expired)____", "Polaroid Time Zero (Expired) ---" );
  prop_preset.add_enum_value( 46, "Polaroid_Time_Zero_(Expired)___", "Polaroid Time Zero (Expired) --" );
  prop_preset.add_enum_value( 47, "Polaroid_Time_Zero_(Expired)__", "Polaroid Time Zero (Expired) -" );
  prop_preset.add_enum_value( 48, "Polaroid_Time_Zero_(Expired)", "Polaroid Time Zero (Expired)" );
  prop_preset.add_enum_value( 49, "Polaroid_Time_Zero_(Expired)_+", "Polaroid Time Zero (Expired) +" );
  prop_preset.add_enum_value( 50, "Polaroid_Time_Zero_(Expired)_++", "Polaroid Time Zero (Expired) ++" );
  prop_preset.add_enum_value( 51, "Polaroid_Time_Zero_(Expired)_Cold____", "Polaroid Time Zero (Expired) Cold ---" );
  prop_preset.add_enum_value( 52, "Polaroid_Time_Zero_(Expired)_Cold___", "Polaroid Time Zero (Expired) Cold --" );
  prop_preset.add_enum_value( 53, "Polaroid_Time_Zero_(Expired)_Cold__", "Polaroid Time Zero (Expired) Cold -" );
  prop_preset.add_enum_value( 54, "Polaroid_Time_Zero_(Expired)_Cold", "Polaroid Time Zero (Expired) Cold" );
  set_type( "gmic_emulate_film_instant_consumer" );
}


VipsImage* PF::GmicEmulateFilmInstantConsumerPar::build(std::vector<VipsImage*>& in, int first,
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

  std::string command = "-gimp_emulate_film_instant_consumer  ";
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


PF::ProcessorBase* PF::new_gmic_emulate_film_instant_consumer()
{
  return( new PF::Processor<PF::GmicEmulateFilmInstantConsumerPar,PF::GmicEmulateFilmInstantConsumerProc>() );
}



PF::GmicEmulateFilmInstantProPar::GmicEmulateFilmInstantProPar():
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
  prop_preset.add_enum_value( 1, "Fuji_FP_100c___", "Fuji FP-100c --" );
  prop_preset.add_enum_value( 2, "Fuji_FP_100c__", "Fuji FP-100c -" );
  prop_preset.add_enum_value( 3, "Fuji_FP_100c", "Fuji FP-100c" );
  prop_preset.add_enum_value( 4, "Fuji_FP_100c_+", "Fuji FP-100c +" );
  prop_preset.add_enum_value( 5, "Fuji_FP_100c_++", "Fuji FP-100c ++" );
  prop_preset.add_enum_value( 6, "Fuji_FP_100c_++a", "Fuji FP-100c ++a" );
  prop_preset.add_enum_value( 7, "Fuji_FP_100c_+++", "Fuji FP-100c +++" );
  prop_preset.add_enum_value( 8, "Fuji_FP_100c_Cool___", "Fuji FP-100c Cool --" );
  prop_preset.add_enum_value( 9, "Fuji_FP_100c_Cool__", "Fuji FP-100c Cool -" );
  prop_preset.add_enum_value( 10, "Fuji_FP_100c_Cool", "Fuji FP-100c Cool" );
  prop_preset.add_enum_value( 11, "Fuji_FP_100c_Cool_+", "Fuji FP-100c Cool +" );
  prop_preset.add_enum_value( 12, "Fuji_FP_100c_Cool_++", "Fuji FP-100c Cool ++" );
  prop_preset.add_enum_value( 13, "Fuji_FP_100c_Negative___", "Fuji FP-100c Negative --" );
  prop_preset.add_enum_value( 14, "Fuji_FP_100c_Negative__", "Fuji FP-100c Negative -" );
  prop_preset.add_enum_value( 15, "Fuji_FP_100c_Negative", "Fuji FP-100c Negative" );
  prop_preset.add_enum_value( 16, "Fuji_FP_100c_Negative_+", "Fuji FP-100c Negative +" );
  prop_preset.add_enum_value( 17, "Fuji_FP_100c_Negative_++", "Fuji FP-100c Negative ++" );
  prop_preset.add_enum_value( 18, "Fuji_FP_100c_Negative_++a", "Fuji FP-100c Negative ++a" );
  prop_preset.add_enum_value( 19, "Fuji_FP_100c_Negative_+++", "Fuji FP-100c Negative +++" );
  prop_preset.add_enum_value( 20, "Fuji_FP_3000b___", "Fuji FP-3000b --" );
  prop_preset.add_enum_value( 21, "Fuji_FP_3000b__", "Fuji FP-3000b -" );
  prop_preset.add_enum_value( 22, "Fuji_FP_3000b", "Fuji FP-3000b" );
  prop_preset.add_enum_value( 23, "Fuji_FP_3000b_+", "Fuji FP-3000b +" );
  prop_preset.add_enum_value( 24, "Fuji_FP_3000b_++", "Fuji FP-3000b ++" );
  prop_preset.add_enum_value( 25, "Fuji_FP_3000b_+++", "Fuji FP-3000b +++" );
  prop_preset.add_enum_value( 26, "Fuji_FP_3000b_HC", "Fuji FP-3000b HC" );
  prop_preset.add_enum_value( 27, "Fuji_FP_3000b_Negative___", "Fuji FP-3000b Negative --" );
  prop_preset.add_enum_value( 28, "Fuji_FP_3000b_Negative__", "Fuji FP-3000b Negative -" );
  prop_preset.add_enum_value( 29, "Fuji_FP_3000b_Negative", "Fuji FP-3000b Negative" );
  prop_preset.add_enum_value( 30, "Fuji_FP_3000b_Negative_+", "Fuji FP-3000b Negative +" );
  prop_preset.add_enum_value( 31, "Fuji_FP_3000b_Negative_++", "Fuji FP-3000b Negative ++" );
  prop_preset.add_enum_value( 32, "Fuji_FP_3000b_Negative_+++", "Fuji FP-3000b Negative +++" );
  prop_preset.add_enum_value( 33, "Fuji_FP_3000b_Negative_Early", "Fuji FP-3000b Negative Early" );
  prop_preset.add_enum_value( 34, "Polaroid_665___", "Polaroid 665 --" );
  prop_preset.add_enum_value( 35, "Polaroid_665__", "Polaroid 665 -" );
  prop_preset.add_enum_value( 36, "Polaroid_665", "Polaroid 665" );
  prop_preset.add_enum_value( 37, "Polaroid_665_+", "Polaroid 665 +" );
  prop_preset.add_enum_value( 38, "Polaroid_665_++", "Polaroid 665 ++" );
  prop_preset.add_enum_value( 39, "Polaroid_665_Negative__", "Polaroid 665 Negative -" );
  prop_preset.add_enum_value( 40, "Polaroid_665_Negative", "Polaroid 665 Negative" );
  prop_preset.add_enum_value( 41, "Polaroid_665_Negative_+", "Polaroid 665 Negative +" );
  prop_preset.add_enum_value( 42, "Polaroid_665_Negative_HC", "Polaroid 665 Negative HC" );
  prop_preset.add_enum_value( 43, "Polaroid_669___", "Polaroid 669 --" );
  prop_preset.add_enum_value( 44, "Polaroid_669__", "Polaroid 669 -" );
  prop_preset.add_enum_value( 45, "Polaroid_669", "Polaroid 669" );
  prop_preset.add_enum_value( 46, "Polaroid_669_+", "Polaroid 669 +" );
  prop_preset.add_enum_value( 47, "Polaroid_669_++", "Polaroid 669 ++" );
  prop_preset.add_enum_value( 48, "Polaroid_669_+++", "Polaroid 669 +++" );
  prop_preset.add_enum_value( 49, "Polaroid_669_Cold___", "Polaroid 669 Cold --" );
  prop_preset.add_enum_value( 50, "Polaroid_669_Cold__", "Polaroid 669 Cold -" );
  prop_preset.add_enum_value( 51, "Polaroid_669_Cold", "Polaroid 669 Cold" );
  prop_preset.add_enum_value( 52, "Polaroid_669_Cold_+", "Polaroid 669 Cold +" );
  prop_preset.add_enum_value( 53, "Polaroid_690___", "Polaroid 690 --" );
  prop_preset.add_enum_value( 54, "Polaroid_690__", "Polaroid 690 -" );
  prop_preset.add_enum_value( 55, "Polaroid_690", "Polaroid 690" );
  prop_preset.add_enum_value( 56, "Polaroid_690_+", "Polaroid 690 +" );
  prop_preset.add_enum_value( 57, "Polaroid_690_++", "Polaroid 690 ++" );
  prop_preset.add_enum_value( 58, "Polaroid_690_Cold___", "Polaroid 690 Cold --" );
  prop_preset.add_enum_value( 59, "Polaroid_690_Cold__", "Polaroid 690 Cold -" );
  prop_preset.add_enum_value( 60, "Polaroid_690_Cold", "Polaroid 690 Cold" );
  prop_preset.add_enum_value( 61, "Polaroid_690_Cold_+", "Polaroid 690 Cold +" );
  prop_preset.add_enum_value( 62, "Polaroid_690_Cold_++", "Polaroid 690 Cold ++" );
  prop_preset.add_enum_value( 63, "Polaroid_690_Warm___", "Polaroid 690 Warm --" );
  prop_preset.add_enum_value( 64, "Polaroid_690_Warm__", "Polaroid 690 Warm -" );
  prop_preset.add_enum_value( 65, "Polaroid_690_Warm", "Polaroid 690 Warm" );
  prop_preset.add_enum_value( 66, "Polaroid_690_Warm_+", "Polaroid 690 Warm +" );
  prop_preset.add_enum_value( 67, "Polaroid_690_Warm_++", "Polaroid 690 Warm ++" );
  set_type( "gmic_emulate_film_instant_pro" );
}


VipsImage* PF::GmicEmulateFilmInstantProPar::build(std::vector<VipsImage*>& in, int first,
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

  std::string command = "-gimp_emulate_film_instant_pro  ";
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


PF::ProcessorBase* PF::new_gmic_emulate_film_instant_pro()
{
  return( new PF::Processor<PF::GmicEmulateFilmInstantProPar,PF::GmicEmulateFilmInstantProProc>() );
}



PF::GmicEmulateFilmNegativeColorPar::GmicEmulateFilmNegativeColorPar():
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


PF::ProcessorBase* PF::new_gmic_emulate_film_negative_color()
{
  return( new PF::Processor<PF::GmicEmulateFilmNegativeColorPar,PF::GmicEmulateFilmNegativeColorProc>() );
}



PF::GmicEmulateFilmNegativeNewPar::GmicEmulateFilmNegativeNewPar():
OpParBase(),
  iterations("iterations",this,1),
  prop_preset("preset", this, 0, "None", "None"),
  prop_effect("effect", this, 1, "Standard", "Standard"),
  prop_opacity("opacity",this,100),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,0),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0)
{
  gmic = PF::new_gmic();
  prop_preset.add_enum_value( 1, "Fuji_160C", "Fuji 160C" );
  prop_preset.add_enum_value( 2, "Fuji_400H", "Fuji 400H" );
  prop_preset.add_enum_value( 3, "Fuji_800Z", "Fuji 800Z" );
  prop_preset.add_enum_value( 4, "Fuji_Ilford_HP5", "Fuji Ilford HP5" );
  prop_preset.add_enum_value( 5, "Kodak_Portra_160", "Kodak Portra 160" );
  prop_preset.add_enum_value( 6, "Kodak_Portra_400", "Kodak Portra 400" );
  prop_preset.add_enum_value( 7, "Kodak_Portra_800", "Kodak Portra 800" );
  prop_preset.add_enum_value( 8, "Kodak_TMAX_3200", "Kodak TMAX 3200" );
  prop_preset.add_enum_value( 9, "Kodak_TRI_X_400", "Kodak TRI-X 400" );
  prop_effect.add_enum_value( 0, "Low", "Low" );
  prop_effect.add_enum_value( 2, "High", "High" );
  prop_effect.add_enum_value( 3, "Higher", "Higher" );
  set_type( "gmic_emulate_film_negative_new" );
}


VipsImage* PF::GmicEmulateFilmNegativeNewPar::build(std::vector<VipsImage*>& in, int first,
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

  std::string command = "-gimp_emulate_film_negative_new  ";
  command = command + prop_preset.get_enum_value_str();
  command = command + std::string(",") + prop_effect.get_enum_value_str();
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


PF::ProcessorBase* PF::new_gmic_emulate_film_negative_new()
{
  return( new PF::Processor<PF::GmicEmulateFilmNegativeNewPar,PF::GmicEmulateFilmNegativeNewProc>() );
}



PF::GmicEmulateFilmNegativeOldPar::GmicEmulateFilmNegativeOldPar():
OpParBase(),
  iterations("iterations",this,1),
  prop_preset("preset", this, 0, "None", "None"),
  prop_effect("effect", this, 1, "Standard", "Standard"),
  prop_opacity("opacity",this,100),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,0),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0)
{
  gmic = PF::new_gmic();
  prop_preset.add_enum_value( 1, "Fuji_Ilford_Delta_3200", "Fuji Ilford Delta 3200" );
  prop_preset.add_enum_value( 2, "Fuji_Neopan_1600", "Fuji Neopan 1600" );
  prop_preset.add_enum_value( 3, "Fuji_Superia_100", "Fuji Superia 100" );
  prop_preset.add_enum_value( 4, "Fuji_Superia_400", "Fuji Superia 400" );
  prop_preset.add_enum_value( 5, "Fuji_Superia_800", "Fuji Superia 800" );
  prop_preset.add_enum_value( 6, "Fuji_Superia_1600", "Fuji Superia 1600" );
  prop_preset.add_enum_value( 7, "Kodak_Portra_160_NC", "Kodak Portra 160 NC" );
  prop_preset.add_enum_value( 8, "Kodak_Portra_160_VC", "Kodak Portra 160 VC" );
  prop_preset.add_enum_value( 9, "Kodak_Portra_400_NC", "Kodak Portra 400 NC" );
  prop_preset.add_enum_value( 10, "Kodak_Portra_400_UC", "Kodak Portra 400 UC" );
  prop_preset.add_enum_value( 11, "Kodak_Portra_400_VC", "Kodak Portra 400 VC" );
  prop_effect.add_enum_value( 0, "Low", "Low" );
  prop_effect.add_enum_value( 2, "High", "High" );
  prop_effect.add_enum_value( 3, "Higher", "Higher" );
  set_type( "gmic_emulate_film_negative_old" );
}


VipsImage* PF::GmicEmulateFilmNegativeOldPar::build(std::vector<VipsImage*>& in, int first,
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

  std::string command = "-gimp_emulate_film_negative_old  ";
  command = command + prop_preset.get_enum_value_str();
  command = command + std::string(",") + prop_effect.get_enum_value_str();
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


PF::ProcessorBase* PF::new_gmic_emulate_film_negative_old()
{
  return( new PF::Processor<PF::GmicEmulateFilmNegativeOldPar,PF::GmicEmulateFilmNegativeOldProc>() );
}



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


/*



PF::GmicEmulateFilmUserDefinedPar::GmicEmulateFilmUserDefinedPar():
OpParBase(),
prop_filename("filename", this),
prop_haldlutfilename("haldlutfilename", this),
  prop_opacity("opacity",this,100),
  prop_gamma("gamma",this,0),
  prop_contrast("contrast",this,0),
  prop_brightness("brightness",this,0),
  prop_hue("hue",this,0),
  prop_saturation("saturation",this,0),
  prop_post_normalize("post_normalize",this,0),
  temp_lut_created( false )
{
  gmic_proc = PF::new_gmic();
  gmic_instance = NULL;
  set_type( "gmic_emulate_film_user_defined" );
  set_default_name( _("apply LUT") );
}


PF::GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar()
{
  if( temp_lut_created ) {
    std::cout<<"GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar(): deleting "<<prop_haldlutfilename.get()<<std::endl;
    unlink( prop_haldlutfilename.get().c_str() );
  }

  std::cout<<"GmicEmulateFilmUserDefinedPar::~GmicEmulateFilmUserDefinedPar()"<<std::endl;
  if( gmic_instance ) delete gmic_instance;
}


gmic* PF::GmicEmulateFilmUserDefinedPar::new_gmic()
{
  //if( custom_gmic_commands ) delete [] custom_gmic_commands;
  if( gmic_instance ) delete gmic_instance;

  std::cout<<"Loading G'MIC custom commands..."<<std::endl;
  char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
  snprintf( fname, 499, "%s\\gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
  struct stat buffer;
  int stat_result = stat( fname, &buffer );
  if( stat_result != 0 ) {
    fname[0] = 0;
  }
#elif defined(__APPLE__) && defined (__MACH__)
  //snprintf( fname, 499, "%s/../share/photoflow/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
  snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_data_dir().c_str() );
  std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
  struct stat buffer;
  int stat_result = stat( fname, &buffer );
  if( stat_result != 0 ) {
    fname[0] = 0;
  }
#else
  if( getenv("HOME") ) {
    //snprintf( fname, 499, "%s/.photoflow/gmic_update.gmic", getenv("HOME") );
    snprintf( fname, 499, "%s/share/photoflow/gmic_def.gmic", INSTALL_PREFIX );
    std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
    struct stat buffer;
    int stat_result = stat( fname, &buffer );
    if( stat_result != 0 ) {
      //snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
      //stat_result = stat( fname, &buffer );
      //if( stat_result != 0 ) {
      fname[0] = 0;
      //}
    }
  }
#endif
  if( strlen( fname ) > 0 ) {
    std::ifstream t;
    int length;
    t.open(fname);      // open input file
    t.seekg(0, std::ios::end);    // go to the end
    length = t.tellg();           // report location (this is the length)
    t.seekg(0, std::ios::beg);    // go back to the beginning
    custom_gmic_commands = new char[length];    // allocate memory for a buffer of appropriate dimension
    t.read(custom_gmic_commands, length);       // read the whole file into the buffer
    t.close();                    // close file handle
    std::cout<<"G'MIC custom commands loaded"<<std::endl;
  }

  // Make a gmic for this thread.
  gmic_instance = new gmic( 0, custom_gmic_commands, false, 0, 0 );
  return gmic_instance;
}


void PF::GmicEmulateFilmUserDefinedPar::pre_build( rendermode_t mode )
{
  std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build() called"<<std::endl;

  if( prop_filename.get().empty() ) return;

  std::string ext;
#if defined(WIN32)
  PF::getFileExtension( "\\", prop_filename.get(), ext );
#else
  PF::getFileExtension( "/", prop_filename.get(), ext );
#endif

  if( ext != "cube" ) {
    return;
  }

  if( prop_filename.get() != cur_lut_filename || prop_haldlutfilename.get().empty() ) {

    std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build(): converting CUBE LUT \""<<prop_filename.get()
        <<"\" to HaldCLUT format"<<std::endl;

    if( !new_gmic() )
      return;

    std::cout<<"GmicEmulateFilmUserDefinedPar::pre_build(): gmic_instance="<<gmic_instance<<std::endl;

    if( prop_haldlutfilename.get().empty() ) {
      char fname[500];
      sprintf( fname,"%shaldclut-XXXXXX.tif", PF::PhotoFlow::Instance().get_cache_dir().c_str() );
      int temp_fd = pf_mkstemp( fname, 4 );
      if( temp_fd >= 0 ) close( temp_fd );
      prop_haldlutfilename.update( std::string(fname) );
    }

    std::string command = "-verbose +  -input_cube \"";
    command = command + prop_filename.get();
    command = command + std::string("\" -r 64,64,64,3,3 -r 512,512,1,3,-1 ");
    command = command + std::string(" -o ") + prop_haldlutfilename.get();
    std::cout<<"g'mic command: "<<command<<std::endl;
    gmic_instance->run( command.c_str() );

    if( gmic_instance ) {
      delete gmic_instance;
      gmic_instance = NULL;
    }

    temp_lut_created = true;
    cur_lut_filename = prop_filename.get();
  }
}


VipsImage* PF::GmicEmulateFilmUserDefinedPar::build(std::vector<VipsImage*>& in, int first,
                                        VipsImage* imap, VipsImage* omap,
                                        unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  VipsImage* mask;
  VipsImage* out = srcimg;

  if( !out ) return NULL;

  if( prop_filename.get().empty() ) {
    PF_REF( out, "" );
    return out;
  }

  if( !(gmic_proc->get_par()) ) return NULL;
  PF::GMicPar* gpar = dynamic_cast<PF::GMicPar*>( gmic_proc->get_par() );
  if( !gpar ) return NULL;

  float scalefac = 1;
  for( int l = 1; l <= level; l++ )
    scalefac *= 2;

  std::string ext;
#if defined(WIN32)
  PF::getFileExtension( "\\", prop_filename.get(), ext );
#else
  PF::getFileExtension( "/", prop_filename.get(), ext );
#endif

  std::string lutname = prop_filename.get();
  if( ext == "cube" ) {
    lutname = prop_haldlutfilename.get();
  }

  std::cout<<"GmicEmulateFilmUserDefinedPar::build(): prop_haldlutfilename="<<prop_haldlutfilename.get()<<std::endl;
  std::string command = "-verbose - -gimp_emulate_film_userdefined  2,";
  command = command + lutname;
  command = command + std::string(",") + prop_opacity.get_str();
  command = command + std::string(",") + prop_brightness.get_str();
  command = command + std::string(",") + prop_contrast.get_str();
  command = command + std::string(",") + prop_gamma.get_str();
  command = command + std::string(",") + prop_hue.get_str();
  command = command + std::string(",") + prop_saturation.get_str();
  command = command + std::string(",") + prop_post_normalize.get_str();
  gpar->set_command( command.c_str() );
  gpar->set_iterations( 1 );
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


PF::ProcessorBase* PF::new_gmic_emulate_film_user_defined()
{
  return( new PF::Processor<PF::GmicEmulateFilmUserDefinedPar,PF::GmicEmulateFilmUserDefinedProc>() );
}
*/
