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

//#include <vips/cimg_funcs.h>

#include "../convertformat.hh"
#include "gmic.hh"

int vips_gmic(int n, VipsImage** out, const char* command, int padding, float x_scale, float y_scale,...);


PF::GMicPar::GMicPar(): 
  OpParBase(),
  iterations("iterations",this,1),
  command("command",this,""),
  padding("padding",this,0),
  x_scale("x_scale",this,1), 
  y_scale("y_scale",this,1)
{
  convert_format = PF::new_convert_format();	
  convert_format2 = PF::new_convert_format();	
  set_type( "gmic" );
}



VipsImage* PF::GMicPar::build(std::vector<VipsImage*>& in, int first, 
				   VipsImage* imap, VipsImage* omap, 
				   unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];

  if( !srcimg ) return NULL;

  if( command.get().empty() ) {
    PF_REF( srcimg, "GMicPar::build(): empty command string" );
    return srcimg;
  }

  /*
	if( (get_render_mode() == PF_RENDER_PREVIEW && level>0) ) {
		PF_REF( out, "PF::GMicPar::build(): out ref" );
		return out;
	}
  */

  /**/
  std::vector<VipsImage*> in2;
  in2.push_back( srcimg );
  convert_format->get_par()->set_image_hints( srcimg );
  convert_format->get_par()->set_format( VIPS_FORMAT_FLOAT );
  VipsImage* convimg = convert_format->get_par()->build( in2, 0, NULL, NULL, level );
  if( !convimg ) return NULL;
  /**/
  //convimg = srcimg;
  //PF_REF( srcimg, "GMicPar::build(): srcimg ref" );

  //return convimg;

  /**/
  VipsImage* iter_in = convimg;
  VipsImage* iter_out = NULL;
  for( int i = 0; i < iterations.get(); i++ ) {
    if( vips_gmic( 1, &iter_out, command.get().c_str(),
                   padding.get(), x_scale.get(),
                   y_scale.get(), 
                   "in0", iter_in, NULL ) ) {
      std::cout<<"vips_gmic() failed!!!!!!!"<<std::endl;
      PF_UNREF( iter_in, "GMicPar::build(): vips_gmic() failed, iter_in unref" );
      return NULL;
    }
    PF_UNREF( iter_in, "GMicPar::build(): iter_in unref" );
    iter_in = iter_out;
  }
  /**/
  //return iter_out;

  //out = convimg;
  in2.clear();
  in2.push_back( iter_out );
  convert_format2->get_par()->set_image_hints( iter_out );
  convert_format2->get_par()->set_format( get_format() );
  VipsImage* out = convert_format2->get_par()->build( in2, 0, NULL, NULL, level );
  PF_UNREF( iter_out, "GMicPar::build(): iter_out unref" );

	return out;
}


PF::ProcessorBase* PF::new_gmic()
{
  return( new PF::Processor<PF::GMicPar,PF::GMicProc>() );
}
