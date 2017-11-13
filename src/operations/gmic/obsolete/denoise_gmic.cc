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
#include "denoise.hh"



PF::GmicDenoisePar::GmicDenoisePar(): 
OpParBase(),
  iterations("iterations",this,1),
  prop_sigma_s("sigma_s",this,10),
  prop_sigma_r("sigma_r",this,10),
  prop_psize("psize",this,3),
  prop_rsize("rsize",this,5),
  prop_smoothness("smoothness",this,0),
  prop_is_fast("is_fast",this,1)
{	
  gmic = PF::new_gmic();
  set_type( "gmic_denoise" );
}



VipsImage* PF::GmicDenoisePar::build(std::vector<VipsImage*>& in, int first, 
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

  std::string command = "-denoise  ";
  command = command + prop_sigma_s.get_str();
  command = command + std::string(",") + prop_sigma_r.get_str();
  command = command + std::string(",") + prop_psize.get_str();
  command = command + std::string(",") + prop_rsize.get_str();
  command = command + std::string(",") + prop_smoothness.get_str();
  command = command + std::string(",") + prop_is_fast.get_str();
  std::cout<<"Command: "<<command<<std::endl;
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


PF::ProcessorBase* PF::new_gmic_denoise()
{
  return( new PF::Processor<PF::GmicDenoisePar,PF::GmicDenoiseProc>() );
}
