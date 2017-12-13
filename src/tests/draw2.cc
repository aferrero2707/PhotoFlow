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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <vector>

#include <vips/vips.h>

#include "../base/rawbuffer.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/new_operation.hh"
#include "../operations/draw.hh"
#include "../base/photoflow.hh"


/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  extern GType vips_layer_get_type( void ); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

int main( int argc, char **argv )
{
  VipsImage *image;
  VipsImage *image2;
  VipsImage *out;
  unsigned int level = 0;

  int width = 4000, height = 4000;
 
  if (vips_init (argv[0]))
    vips::verror ();

  vips_layer_get_type();

  PF::PhotoFlow::Instance().set_new_op_func( PF::new_operation );
  PF::PhotoFlow::Instance().set_new_op_func_nogui( PF::new_operation );

  
  PF::Processor<PF::DrawPar,PF::DrawProc>* draw = new PF::Processor<PF::DrawPar,PF::DrawProc>();
  PF::DrawPar* par = (PF::DrawPar*)draw->get_par();
  if( !par ) return 1;

  par->get_pen_R().set( 1 );
  par->get_pen_G().set( 0 );
  par->get_pen_B().set( 0 );
  par->get_bgd_R().set( 0 );
  par->get_bgd_G().set( 0 );
  par->get_bgd_B().set( 0 );

  par->set_image_hints( width, height, PF::PF_COLORSPACE_RGB );
  par->set_nbands( 3 );
  par->set_format( VIPS_FORMAT_USHORT );
  par->set_blend_mode( PF::PF_BLEND_NORMAL );
  par->set_opacity( 1 );
  std::vector<VipsImage*> in; //in.push_back( NULL );
  image = draw->get_par()->build(in, 0, NULL, NULL, level );

  vips_image_write_to_file( image, "/tmp/test.tif" );

  VipsRect update;
  std::cout<<"Drawing strokes..."<<std::endl;
  std::cout.flush();
  par->start_stroke(100,0.5);
  for( int pi = 1000; pi <= 3000; pi+=10 )
    par->draw_point( pi, pi, update );
  for( int pi = 1000; pi <= 3000; pi+=10 )
    par->draw_point( pi, pi+50, update );
  par->end_stroke();

  par->start_stroke(100,0.5);
  for( int pi = 1000; pi <= 3000; pi+=10 )
    par->draw_point( pi, pi+100, update );
  par->end_stroke();

  std::cout<<"... done."<<std::endl;
  std::cout.flush();

  vips_image_write_to_file( image, "/tmp/test2.tif" );

  par->get_bgd_R().set( 0.5 );
  par->get_bgd_G().set( 0.5 );
  par->get_bgd_B().set( 0.5 );

  image = draw->get_par()->build(in, 0, NULL, NULL, level );

  vips_image_write_to_file( image, "/tmp/test3.tif" );

  /*
  VipsImage* tempimg;
  vips_rawload( par->get_rawbuf()->get_file_name().c_str(), &tempimg, width, height, sizeof(unsigned short int)*3, NULL );
  vips_copy( tempimg, &image, 
	     "format", VIPS_FORMAT_USHORT,
	     "bands", 3,
	     "coding", VIPS_CODING_NONE,
	     "interpretation", VIPS_INTERPRETATION_RGB,
	     NULL );

  vips_image_write_to_file( image, "/tmp/test4.tif" );
  */
  return( 0 );
}
