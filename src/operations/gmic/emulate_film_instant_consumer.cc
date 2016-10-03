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
#include "emulate_film_instant_consumer.hh"



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


int PF::GmicEmulateFilmInstantConsumerPar::get_padding( int level )
{
  return 0;
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


PF::ProcessorBase* PF::new_gmic_emulate_film_instant_consumer()
{
  return( new PF::Processor<PF::GmicEmulateFilmInstantConsumerPar,PF::GmicEmulateFilmInstantConsumerProc>() );
}
