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
 
  // Create VipsImage from given file
#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
  image = vips_image_new_from_file( argv[1] );
#else
  image = vips_image_new_from_file( argv[1], NULL );
#endif
  if( !image ) {
    printf("Failed to load \"%s\"\n",argv[1]);
    return 1;
  }

  printf("image:            %p\n",image);
  printf("# of bands:       %d\n",image->Bands);
  printf("band format:      %d\n",image->BandFmt);
  printf("type:             %d\n",image->Type);
  printf("image dimensions: %d x %d\n",image->Xsize,image->Ysize);

  printf("saving test buffer (image=%p)...\n",image);
  size_t array_sz;
  void* mem_array = vips_image_write_to_memory( image, &array_sz );
  printf("test buffer saved (mem_array=%p, array_sz=%d).\n",mem_array,(int)array_sz);
  if( mem_array ) free(mem_array);

  //if( !mem_array )

  vips_tiffsave( image, "test.tif", "compression", VIPS_FOREIGN_TIFF_COMPRESSION_DEFLATE,
      "predictor", VIPS_FOREIGN_TIFF_PREDICTOR_NONE, NULL );

  g_object_unref( image );

  return( 0 );
}
