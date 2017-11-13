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
#include "emulate_film_instant_pro.hh"



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
