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
#include "gradient_norm.hh"



PF::GmicGradientNormPar::GmicGradientNormPar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_smoothness("smoothness",this,0),
  prop_linearity("linearity",this,0.5),
  prop_min_threshold("min_threshold",this,0),
  prop_max_threshold("max_threshold",this,100)
{	
  gmic = PF::new_gmic();
  set_type( "gmic_gradient_norm" );
}


int PF::GmicGradientNormPar::get_padding( int level )
{
  return 0;
}


VipsImage* PF::GmicGradientNormPar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-gimp_gradient_norm  ";
  command = command + prop_smoothness.get_str();
  command = command + std::string(",") + prop_linearity.get_str();
  command = command + std::string(",") + prop_min_threshold.get_str();
  command = command + std::string(",") + prop_max_threshold.get_str();
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


PF::ProcessorBase* PF::new_gmic_gradient_norm()
{
  return( new PF::Processor<PF::GmicGradientNormPar,PF::GmicGradientNormProc>() );
}
