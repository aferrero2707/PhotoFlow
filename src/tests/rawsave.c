/* Compile with
 
   gcc rawsave.c `pkg-config vips --cflags --libs`
 
*/
 
#include <vips/vips.h>
 
int
main( int argc, char **argv )
{
  VipsImage *image;
  VipsImage *image2;
  VipsImage *out;
 
  if( !(image = vips_image_new_from_file( argv[1] )) )
    vips_error_exit( NULL );
 
  if( vips_rawsave( image, argv[2], NULL ) )
    vips_error_exit( NULL );

  vips_rawload( argv[2], &image2, image->Xsize, image->Ysize, image->Bands, NULL );
  vips_copy( image2, &out, 
	     "format", image->BandFmt,
	     "coding", image->Coding,
	     "interpretation", image->Type,
	     NULL );
  g_object_unref( image );
  g_object_unref( image2 );
  vips_image_write_to_file( out, argv[3] );
  g_object_unref( out );

  return( 0 );
}
