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
#include "smooth_anisotropic.hh"



PF::GmicSmoothAnisotropicPar::GmicSmoothAnisotropicPar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_amplitude("amplitude",this,60),
  prop_sharpness("sharpness",this,0.7),
  prop_anisotropy("anisotropy",this,0.3),
  prop_gradient_smoothness("gradient_smoothness",this,0.6),
  prop_tensor_smoothness("tensor_smoothness",this,1.1),
  prop_spatial_precision("spatial_precision",this,0.8),
  prop_angular_precision("angular_precision",this,30),
  prop_value_precision("value_precision",this,2),
  prop_interpolation("interpolation", this, 0, "NEAREST_NEIGHBOR", "Nearest neighbor"),
prop_fast_approximation("fast_approximation",this,1),
  prop_padding("padding",this,24)
{	
  gmic = PF::new_gmic();
  prop_interpolation.add_enum_value(1, "LINEAR", "Linear"),
  prop_interpolation.add_enum_value(2, "RUNGE_KUTTA", "Runge-Kutta"),
  set_type( "gmic_smooth_anisotropic" );
}



int PF::GmicSmoothAnisotropicPar::get_padding( int level )
{
  return( prop_padding.get() );
}


VipsImage* PF::GmicSmoothAnisotropicPar::build(std::vector<VipsImage*>& in, int first, 
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
	for( unsigned int l = 1; l <= level; l++ )
		scalefac *= 2;

  char interp_val[10];
  snprintf( interp_val, 9, "%d", prop_interpolation.get_enum_value().first );

  std::string command = "-smooth  ";
  command = command + prop_amplitude.get_str();
  command = command + std::string(",") + prop_sharpness.get_str();
  command = command + std::string(",") + prop_anisotropy.get_str();
  command = command + std::string(",") + prop_gradient_smoothness.get_str();
  command = command + std::string(",") + prop_tensor_smoothness.get_str();
  command = command + std::string(",") + prop_spatial_precision.get_str();
  command = command + std::string(",") + prop_angular_precision.get_str();
  command = command + std::string(",") + prop_value_precision.get_str();
  //command = command + std::string(",") + prop_interpolation.get_str();
  command = command + std::string(",") + interp_val;
  command = command + std::string(",") + prop_fast_approximation.get_str();
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


PF::ProcessorBase* PF::new_gmic_smooth_anisotropic()
{
  return( new PF::Processor<PF::GmicSmoothAnisotropicPar,PF::GmicSmoothAnisotropicProc>() );
}
