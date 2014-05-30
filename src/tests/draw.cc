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

  VipsRect update;

  int width = 400, height = 400;
 
  if (vips_init (argv[0]))
    vips::verror ();

  vips_layer_get_type();

  /*
  PF::Processor<PF::UniformPar,PF::Uniform>* uniform = new PF::Processor<PF::UniformPar,PF::Uniform>();
  if( uniform->get_par() ) {
    uniform->get_par()->get_R().set( 0 );
    uniform->get_par()->get_G().set( 0 );
    uniform->get_par()->get_B().set( 0 );
  }

  uniform->get_par()->set_image_hints( width, height, PF::PF_COLORSPACE_RGB );
  uniform->get_par()->set_nbands( 3 );
  uniform->get_par()->set_format( VIPS_FORMAT_USHORT );
  uniform->get_par()->set_blend_mode( PF::PF_BLEND_NORMAL );
  uniform->get_par()->set_opacity( 1 );
  std::vector<VipsImage*> in; //in.push_back( NULL );
  VipsImage* image = uniform->get_par()->build(in, 0, NULL, NULL, level );
  */

  /*
  if( vips_rawsave( image, argv[1], NULL ) )
    vips_error_exit( NULL );
  */

  unsigned short int buf[width*3];
  /*
  int fd = open( argv[1], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU );
  if( fd < 0 ) return 1;
  memset( buf, 0, sizeof(unsigned short int)*width*3 );
  for( int y = 0; y < height; y++ ) {
    write( fd, buf, sizeof(unsigned short int)*width*3 );
  }
  close( fd );
  */

  PF::RawBuffer rawbuf( "/tmp/test.raw" );

  rawbuf.set_xsize( width );
  rawbuf.set_ysize( height );
  rawbuf.set_nbands( 3 );
  rawbuf.set_format( VIPS_FORMAT_USHORT );

  std::vector<float> bgd_color;
  bgd_color.push_back( 0.5 );
  bgd_color.push_back( 0.5 );
  bgd_color.push_back( 0.5 );
  rawbuf.init( bgd_color );

  vips_rawload( "/tmp/test.raw", &image, width, height, (int)(sizeof(unsigned short int)*3), NULL );

  vips_copy( image, &out, 
	     "format", VIPS_FORMAT_USHORT,
	     "bands", 3,
	     "coding", VIPS_CODING_NONE,
	     "interpretation", VIPS_INTERPRETATION_RGB,
	     NULL );
  g_object_unref( image );




  /*
  int fd = open( "/tmp/test.raw", O_WRONLY );
  if( fd < 0 ) return 1;

  int y0 = 100, x0 = 100;
  memset( buf, 0xFF, sizeof(unsigned short int)*100*3 );
  for( int y = y0; y < y0+100; y++ ) {
    off_t offset = (off_t(width)*y+x0)*sizeof(unsigned short int)*3;
    lseek( fd, offset, SEEK_SET );
    write( fd, buf, sizeof(unsigned short int)*100*3 );
  }
  //close( fd );
  */
  

  PF::Pen pen;
  pen.set_size(5);
  pen.set_channel( 0, 1 );
  pen.set_channel( 1, 0 );
  pen.set_channel( 2, 0 );
  pen.set_opacity( 0.25 );

  PF::Segment segment;

  pen.set_size(50);
  rawbuf.start_stroke();
  rawbuf.draw_point( pen, 100, 100, update, true );
  rawbuf.end_stroke();
  rawbuf.start_stroke();
  rawbuf.draw_point( pen, 200, 200, update, true );
  rawbuf.end_stroke();
  rawbuf.start_stroke();
  rawbuf.draw_point( pen, 300, 300, update, true );
  rawbuf.end_stroke();


  level = 1;
  PF::PyramidLevel* l = rawbuf.get_pyramid().get_level( level );
  if( l && l->image) {
    vips_image_write_to_file( l->image, "/tmp/test.tif" );
  }
  g_object_unref( out );

  return( 0 );
}
