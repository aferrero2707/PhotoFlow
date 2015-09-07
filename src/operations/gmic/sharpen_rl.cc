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
#include "sharpen_rl.hh"



PF::GmicSharpenRLPar::GmicSharpenRLPar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_sigma("sigma",this,1),
  prop_iterations("rl_iterations",this,1),
  prop_blur("blur", this, 1, "Gaussian", "Gaussian"),
  padding(0)
{	
  gmic = PF::new_gmic();
  prop_blur.add_enum_value( 0, "Exponential", "Exponential" );
  set_type( "gmic_sharpen_rl" );
}


bool PF::GmicSharpenRLPar::needs_caching() { return true; }



int PF::GmicSharpenRLPar::get_padding( int level )
{
  return(padding);
}


VipsImage* PF::GmicSharpenRLPar::build(std::vector<VipsImage*>& in, int first, 
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

	padding = prop_sigma.get()*2.0*prop_iterations.get()/scalefac;

  std::string command = "-deblur_richardsonlucy ";
  command = command + prop_sigma.get_str();
  command = command + std::string(",") + prop_iterations.get_str();
  command = command + std::string(",") + prop_blur.get_enum_value_str();
  command = command + std::string(" -cut 0,255");
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


PF::ProcessorBase* PF::new_gmic_sharpen_rl()
{
  return( new PF::Processor<PF::GmicSharpenRLPar,PF::GmicSharpenRLProc>() );
}
