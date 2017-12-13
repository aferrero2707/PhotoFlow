/* Compile with
 
   gcc profile.c `pkg-config vips --cflags --libs`
 
*/
 
#include <vips/vips.h>
#include <lcms2.h>
 
int
main( int argc, char **argv )
{
  VipsImage *image;
  VipsImage *x;
  void *data;
  size_t data_length;
  cmsHPROFILE profile;
  char str[1024];
 
  if( !(image = vips_image_new_from_file( argv[1] )) )
    vips_error_exit( NULL );
 
  if( vips_image_get_blob( image, VIPS_META_ICC_NAME, 
			   &data, &data_length ) )
    vips_error_exit( NULL );
 
  printf( "profile: %zd bytes at %p\n", data_length, data ); 
 
  if( !(profile = cmsOpenProfileFromMem( data, data_length )) )
    vips_error_exit( "bad profile" );
 
  cmsGetProfileInfoASCII( profile, cmsInfoDescription, 
			  "en", "US", str, 1024 );
  printf( "profile: %s\n", str ); 
 
  cmsCloseProfile( profile ); 
 
  if( vips_cast( image, &x, VIPS_FORMAT_INT, NULL ) )
    vips_error_exit( NULL );
  g_object_unref( image );
  image = x;
 
  if( vips_image_get_blob( image, VIPS_META_ICC_NAME, 
			   &data, &data_length ) )
    vips_error_exit( NULL );
 
  printf( "profile: %zd bytes at %p\n", data_length, data ); 
 
  if( !(profile = cmsOpenProfileFromMem( data, data_length )) )
    vips_error_exit( "bad profile" );
 
  cmsGetProfileInfoASCII( profile, cmsInfoDescription, 
			  "en", "US", str, 1024 );
  printf( "profile: %s\n", str ); 
 
  cmsCloseProfile( profile ); 
 
  if( vips_image_write_to_file( image, argv[2] ) )
    vips_error_exit( NULL );
 
  g_object_unref( image ); 
 
  return( 0 );
}
