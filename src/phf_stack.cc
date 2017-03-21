/* stack an array of images together
 *
 */

/*

    This file is part of VIPS.
    
    VIPS is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA

 */

/*

    These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

/**/
#define VIPS_DEBUG
/* */

#include <vips/intl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vips/vips.h>
#include <vips/debug.h>

#include <gexiv2/gexiv2-metadata.h>

typedef struct _VipsArraystack {
  VipsOperation parent_instance;

	/* Params.
	 */
	VipsArrayImage *in;
	VipsImage* out;

} VipsArraystack;

typedef VipsOperationClass VipsArraystackClass;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

G_DEFINE_TYPE( VipsArraystack, vips_arraystack, VIPS_TYPE_OPERATION );

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#define PROCESS_PEL(TYPE) { \
    for( c = 0; c < bands; c++ ) { \
      avg = 0; N = 0; delta = 10000000000000; \
      for( i = 0; i < n; i++ ) { \
        TYPE* px = (TYPE*)VIPS_REGION_ADDR( ir[i], r->left + x, r->top + y ); \
        avg += px[c]; N += 1; \
      } \
      avg /= N; \
      for( i = 0; i < n; i++ ) { \
        TYPE* px = (TYPE*)VIPS_REGION_ADDR( ir[i], r->left + x, r->top + y ); \
        D = fabs( avg - (double)(px[c]) ); \
        if( D >= delta ) continue; \
        median = (double)(px[c]); \
        delta = D; \
      } \
      TYPE* pout = (TYPE*)VIPS_REGION_ADDR( or, r->left + x, r->top + y ); \
      pout[c] = (TYPE)(median); \
      if(c==3) pout[c] = 1; \
    } \
}

static int
vips_arraystack_gen( VipsRegion *or, void *seq,
	void *a, void *b, gboolean *stop )
{
	VipsRegion **ir = (VipsRegion **) seq;
	VipsArraystack *stack = (VipsArraystack *) b;
	VipsRect *r = &or->valid;
	int n = ((VipsArea *) stack->in)->n;
  const int bands = vips_image_get_bands( ir[0]->im );
  int sz = r->width * bands;
  //printf("vips_arraystack_gen(): bands=%d\n", bands);

	int i, x, y, c;
  double avg = 0, delta = 10000000000000, D, median; int N = 0;

  for( i = 0; i < n; i++) {
    if( vips_region_prepare( ir[i], r ) )
      return( -1 );
  }

	/* Output requires more than one input. Paste all touching inputs into
	 * the output.
	 */
	for( y = 0; y < r->height; y++ ) {
	  for( x = 0; x < r->width; x++ ) {

	    switch( vips_image_get_format( ir[0]->im ) ) {
	    case VIPS_FORMAT_CHAR:    PROCESS_PEL( signed char ); break;
	    case VIPS_FORMAT_SHORT:   PROCESS_PEL( signed short ); break;
	    case VIPS_FORMAT_INT:     PROCESS_PEL( signed int ); break;
	    case VIPS_FORMAT_FLOAT:   PROCESS_PEL( float ); break;
	    case VIPS_FORMAT_DOUBLE:  PROCESS_PEL( double ); break;

	    default:
	      g_assert_not_reached();
	    }
	  }
	}

	return( 0 );
}

static int
vips_arraystack_build( VipsObject *object )
{
	VipsObjectClass *class = VIPS_OBJECT_GET_CLASS( object );
	VipsOperation *conversion = VIPS_OPERATION( object );
	VipsArraystack *stack = (VipsArraystack *) object;

	VipsImage **in;
	int n;

	VipsImage **format;
	VipsImage **band;
	VipsImage **size;

	int i;

  printf("stack->out: %p\n",(void*)stack->out);

  if( VIPS_OBJECT_CLASS( vips_arraystack_parent_class )->build( object ) )
		return( -1 );

	in = vips_array_image_get( stack->in, &n );
	/* Array length zero means error.
	 */
	if( n == 0 )
		return( -1 ); 

	/* Move all input images to a common format and number of bands.
	 *//*
	format = (VipsImage **) vips_object_local_array( object, n );
	if( vips__formatalike_vec( in, format, n ) )
		return( -1 );
	in = format;
	*/

	/* We have to include the number of bands in @background in our
	 * calculation.
	 *//*
	band = (VipsImage **) vips_object_local_array( object, n );
	if( vips__bandalike_vec( class->nickname, 
		in, band, n, stack->background->n ) )
		return( -1 );
	in = band;
	*/


  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( stack, "out", vips_image_new(), NULL );


	if( vips_image_pipeline_array( stack->out,
		VIPS_DEMAND_STYLE_THINSTRIP, in ) )
		return( -1 );

	printf("stack->out: %p   n=%d\n",(void*)stack->out, n);

	//stack->out->Xsize = in[0]->Xsize;
	//stack->out->Ysize = in[0]->Ysize;

	if( vips_image_generate( stack->out,
		vips_start_many, vips_arraystack_gen, vips_stop_many,
		in, stack ) )
		return( -1 );

	return( 0 );
}

static void
vips_arraystack_class_init( VipsArraystackClass *class )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( class );
	VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( class );
	VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( class );

  VIPS_DEBUG_MSG( "vips_arraystack_class_init\n" );
  printf( "vips_arraystack_class_init\n" );

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	vobject_class->nickname = "arraystack";
	vobject_class->description = _( "stack an array of images" );
	vobject_class->build = vips_arraystack_build;

	operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED;

	VIPS_ARG_BOXED( class, "in", -1, 
		_( "Input" ), 
		_( "Array of input images" ),
		VIPS_ARGUMENT_REQUIRED_INPUT,
		G_STRUCT_OFFSET( VipsArraystack, in ),
		VIPS_TYPE_ARRAY_IMAGE );

  VIPS_ARG_IMAGE( class, "out", 1,
    _( "Output" ),
    _( "Output image" ),
    VIPS_ARGUMENT_REQUIRED_OUTPUT,
    G_STRUCT_OFFSET( VipsArraystack, out ) );
}

static void
vips_arraystack_init( VipsArraystack *stack )
{
}

static int
vips_arraystackv( VipsImage **in, VipsImage **out, int n, va_list ap )
{
	VipsArrayImage *array; 
	int result;

	array = vips_array_image_new( in, n ); 
	result = vips_call_split( "arraystack", ap, array, out );
	vips_area_unref( VIPS_AREA( array ) );

	return( result );
}

/**
 * vips_arraystack:
 * @in: (array length=n) (transfer none): array of input images
 * @out: output image
 * @n: number of input images
 * @...: %NULL-terminated list of optional named arguments
 *
 * Optional arguments:
 *
 * Returns: 0 on success, -1 on error
 */
int
vips_arraystack( VipsImage **in, VipsImage **out, int n, ... )
{
	va_list ap;
	int result;

	va_start( ap, n );
	result = vips_arraystackv( in, out, n, ap );
	va_end( ap );

	return( result );
}



int main(int argc, char** argv)
{
  int ai, N = 0;

  VipsImage* in[1000];
  VipsImage* out;

  if (vips_init (argv[0]))
    //vips::verror ();
    return 1;

  vips_arraystack_get_type();

  //im_concurrency_set( 1 );

  for( ai = 2; ai < argc; ai += 1 ) {
    in[N] = vips_image_new_from_file( argv[ai], NULL );
    if(!in[N]) break;
    N += 1;
  }

  vips_arraystack(in, &out, N, NULL);

  printf("in[1]: %p W=%d H=%d\n",(void*)in[0], in[0]->Xsize, in[0]->Ysize);
  printf("out: %p W=%d H=%d\n",(void*)out, out->Xsize, out->Ysize);
  printf("argv[1]: %s\n",argv[1]);

  vips_tiffsave( out, argv[1], NULL );

  void* gexiv2_buf;
  size_t gexiv2_buf_length;
  if( vips_image_get_blob( out, "gexiv2-data",
                           &gexiv2_buf, &gexiv2_buf_length ) )
    gexiv2_buf = NULL;
  if( gexiv2_buf && (gexiv2_buf_length==sizeof(GExiv2Metadata)) ) {
    gexiv2_metadata_save_file( (GExiv2Metadata*)gexiv2_buf, argv[1], NULL );
  }

  vips_shutdown();


  return 0;
}
