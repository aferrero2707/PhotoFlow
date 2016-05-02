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
#include "smooth_nlmeans.hh"



PF::GmicSmoothNonLocalMeansPar::GmicSmoothNonLocalMeansPar():
OpParBase(),
//iterations("iterations",this,1),
  prop_radius("radius",this,4),
  prop_size("size",this,4),
  prop_tonal_bandwidth("tonal_bandwidth",this,10),
  prop_padding("padding",this,0)
{	
  gmic = PF::new_gmic();
  set_type( "gmic_smooth_nlmeans" );
}



float PF::GmicSmoothNonLocalMeansPar::get_radius( unsigned int level )
{
  float r = prop_radius.get();
  for( unsigned int l = 1; l<= level; l++ ) r /= 2;
  return r;
}


float PF::GmicSmoothNonLocalMeansPar::get_size( unsigned int level )
{
  float s = prop_size.get();
  for( unsigned int l = 1; l<= level; l++ ) s /= 2;
  return s;
}


int PF::GmicSmoothNonLocalMeansPar::get_padding( unsigned int level )
{
  return MAX( get_radius(level)*4, get_size(level)*2 );
  //return prop_padding.get();
}


VipsImage* PF::GmicSmoothNonLocalMeansPar::build(std::vector<VipsImage*>& in, int first,
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

  std::string command = "-nlmeans  ";
  command = command + std::string(",") + to_string( get_radius(level) );
  command = command + std::string(",") + to_string( get_size(level) );
  //command = command + std::string(",") + prop_tonal_bandwidth.get_str();
  gpar->set_command( command.c_str() );
  //gpar->set_iterations( iterations.get() );
  gpar->set_iterations( 1 );
  gpar->set_x_scale( 1.0f );
  gpar->set_y_scale( 1.0f );

  gpar->set_image_hints( srcimg );
  gpar->set_format( get_format() );

  gpar->set_padding( get_padding(level) );

  out = gpar->build( in, first, imap, omap, level );
  if( !out ) {
    std::cout<<"gmic.build() failed!!!!!!!"<<std::endl;
  }

	return out;
}


PF::ProcessorBase* PF::new_gmic_smooth_nlmeans()
{
  return( new PF::Processor<PF::GmicSmoothNonLocalMeansPar,PF::GmicSmoothNonLocalMeansProc>() );
}
