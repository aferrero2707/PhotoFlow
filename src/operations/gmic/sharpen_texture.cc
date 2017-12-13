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
#include "sharpen_texture.hh"



PF::GmicSharpenTexturePar::GmicSharpenTexturePar():
OpParBase(),
  prop_radius("radius",this,4),
  prop_strength("strength",this,1),
  padding(0)
{	
  gmic = PF::new_gmic();
  set_type( "gmic_sharpen_texture" );
}


bool PF::GmicSharpenTexturePar::needs_caching() { return true; }



int PF::GmicSharpenTexturePar::get_gmic_padding( int level )
{
  float scalefac = 1;
  for( unsigned int l = 1; l <= level; l++ )
    scalefac *= 2;

  return( prop_radius.get()*2.0/scalefac );
}


VipsImage* PF::GmicSharpenTexturePar::build(std::vector<VipsImage*>& in, int first,
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

	padding = prop_radius.get()*2.0/scalefac;
	float radius = prop_radius.get()/scalefac;
	int iradius = (int)radius;

	if(iradius < 1) {
	  PF_REF(srcimg,"GmicSharpenTexturePar::build(): radius<1");
	  return srcimg;
	}

	std::cout<<"GmicSharpenTexturePar::build() called, padding="<<padding<<", radius="<<radius<<std::endl;

  std::string command = "-gimp_sharpen_texture ";
  command = command + prop_strength.get_str();
  command = command + std::string(",") + to_string(radius); //prop_radius.get_str();
  command = command + std::string(",0 -cut 0,255");
  std::cout<<"command: "<<command<<std::endl;
  gpar->set_command( command.c_str() );
  gpar->set_iterations( 1 );
  gpar->set_gmic_padding( get_gmic_padding( level ) );
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


PF::ProcessorBase* PF::new_gmic_sharpen_texture()
{
  return( new PF::Processor<PF::GmicSharpenTexturePar,PF::GmicSharpenTextureProc>() );
}
