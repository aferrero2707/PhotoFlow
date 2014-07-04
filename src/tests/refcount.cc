#include <string.h>
#include <vips/vips.h>
#include <gtkmm/main.h>

#include <iostream>

static GObject* object_in;
static GObject* object_in2;

/* Create a tiny VipsOperator ... photographic negative of a uchar image. 
 */

typedef struct _Negative {
	VipsOperation parent_instance;

	VipsImage *in;
	VipsImage *out;

	int image_max;

} Negative;

typedef struct _NegativeClass {
	VipsOperationClass parent_class;

	/* No new class members needed for this op.
	 */

} NegativeClass;

/* This defines a function called negative_get_type() which adds the new
 * operation to the class hierarchy, and negative_parent_class, our
 * superclass.
 *
 * You must define negative_class_init() to init a new class struct and
 * negative_init() to init a new instance struct. 
 */

G_DEFINE_TYPE( Negative, negative, VIPS_TYPE_OPERATION );

static int
negative_generate( VipsRegion *oreg, 
	void *vseq, void *a, void *b, gboolean *stop )
{
	/* The area of the output region we have neen asked to make.
	 */
	VipsRect *r = &oreg->valid;

	/* The sequence value ... the thing returned by start_one.
	 */
	VipsRegion *ir = (VipsRegion *) vseq;

	Negative *negative = (Negative *) b;
	int line_size = r->width * negative->in->Bands; 

	int x, y;
	int max = negative->image_max;

	/* Request matching part of input region.
	 */
	if( vips_region_prepare( ir, r ) )
		return( -1 );

	for( y = 0; y < r->height; y++ ) {
		unsigned char *p = (unsigned char *)
			VIPS_REGION_ADDR( ir, r->left, r->top + y ); 
		unsigned char *q = (unsigned char *)
			VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

		for( x = 0; x < line_size; x++ ) 
			q[x] = max - p[x];
	}

	return( 0 );
}

static int
negative_build( VipsObject *object )
{
	VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
	Negative *negative = (Negative *) object;

	/* Build all the superclasses. This will set @in and
	 * @image_max.
	 */
	if( VIPS_OBJECT_CLASS( negative_parent_class )->build( object ) )
		return( -1 );

	/* We only work for uchar images.
	 */
	if( vips_check_uncoded( klass->nickname, negative->in ) ||
		vips_check_format( klass->nickname, 
			negative->in, VIPS_FORMAT_UCHAR ) )
		return( -1 );

	/* Get ready to write to @out. @out must be set via g_object_set() so
	 * that vips can see the assignment. It'll complain that @out hasn't
	 * been set otherwise.
	 */
	g_object_set( object, "out", vips_image_new(), NULL ); 

	/* Tell vips that @out depends on @in and prefers to work in
	 * scanlines.
	 */
	if( vips_image_pipelinev( negative->out, 
		VIPS_DEMAND_STYLE_THINSTRIP, negative->in, NULL ) )
		return( -1 );

	/* This attaches callbacks to @out to calculate pixels on request.
	 * start_one and stop_one are a pair of start and stop functions which
	 * create a single input region on extra argument 1, handy for 1-ary
	 * operations. 
	 */
	if( vips_image_generate( negative->out, 
		vips_start_one, 
		negative_generate, 
		vips_stop_one, 
		negative->in, negative ) )
		return( -1 );

	return( 0 );
}

static void
negative_class_init( NegativeClass *klass )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
	VipsObjectClass *object_class = (VipsObjectClass *) klass;

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	object_class->nickname = "negative";
	object_class->description = "photographic negative";
	object_class->build = negative_build;

	VIPS_ARG_IMAGE( klass, "in", 1, 
		"Input", 
		"Input image",
		VIPS_ARGUMENT_REQUIRED_INPUT,
		G_STRUCT_OFFSET( Negative, in ) );

	VIPS_ARG_IMAGE( klass, "out", 2, 
		"Output", 
		"Output image",
		VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		G_STRUCT_OFFSET( Negative, out ) );

}

static void
negative_init( Negative *negative )
{
	negative->image_max = 255;
}

/* A type-safe way to call nagative from C.
 *
 * You can also use
 *
 * 	vips_call( "negative", in, &out, "image_max", 128, NULL )
 *
 * but that won't have any compile-time checks, of course.
 */
static int 
negative( VipsImage *in, VipsImage **out, ... )
{
	va_list ap;
	int result;

	va_start( ap, out );
	result = vips_call_split( "negative", ap, in, out );
	va_end( ap );

	return( result );
}

int 
main( int argc, char **argv )
{
  Gtk::Main kit(argc, argv);

  VipsImage *in;
  VipsImage *in2;
  VipsImage *gamma;
  VipsImage *gamma2;
  VipsImage *out;
  VipsImage *out2;
  VipsImage *temp;


  if( vips_init( argv[0] ) )
    vips_error_exit( "unable to init" ); 

  im_concurrency_set( 1 );

  /* Add negative to the class hierarchy. You'll now be able to use it
   * exactly like any built-in vips operation and it'll appear in
   * Python, nip2, C, C++ etc etc. 
   */
  negative_get_type();

  if( argc != 3 )
    vips_error_exit( "usage: %s infile1 infile2", argv[0] ); 

  if( !(in = vips_image_new_from_file( argv[1] )) )
    vips_error_exit( "unable to open" ); 

  object_in = G_OBJECT(in);

  std::cout<<"refcount after new_from_file: in("<<in<<")="<<G_OBJECT(in)->ref_count<<std::endl;

  if( !(in2 = vips_image_new_from_file( argv[2] )) )
    vips_error_exit( "unable to open" ); 

  object_in2 = G_OBJECT(in2);

  std::cout<<"refcount after new_from_file2: in2("<<in<<")="<<G_OBJECT(in2)->ref_count<<std::endl;

  /*
  if( vips_gamma( in, &gamma, "exponent", (gdouble)1, NULL ) ) {
    g_object_unref( in );
    vips_error_exit( "unable to apply gamma" ); 
  }
  std::cout<<std::endl;
  std::cout<<"refcount after gamma: in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  gamma("<<gamma<<")="<<G_OBJECT(gamma)->ref_count<<std::endl;
  // We have a ref to gamma, gamma holds a ref to the gamma operation,
  // and the operation holds a ref to in. We can safely unref in and
  // it'll be unreffed when we unref gamma.
  g_object_unref( in );
  std::cout<<std::endl;
  std::cout<<"refcount after unref(in):  in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  gamma("<<gamma<<")="<<G_OBJECT(gamma)->ref_count<<std::endl;
*/
  gamma = in;

  if( negative( gamma, &out, NULL ) ) {
    g_object_unref( temp );
    vips_error_exit( "unable to invert" ); 
  }
  std::cout<<"refcount after negative:  gamma("<<gamma<<")="<<G_OBJECT(gamma)->ref_count
	   <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;

  /* We have a ref to out, out holds a ref to the negative operation,
   * and the operation holds a ref to gamma. We can safely unref gamma and
   * it'll be unreffed when we unref out.
   */
  g_object_unref( gamma );
  std::cout<<std::endl;
  std::cout<<"refcount after unref(gamma):  gamma("<<gamma<<")="<<G_OBJECT(gamma)->ref_count
	   <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;

  VipsImage* display_image = im_open( "display_image", "p" );

  if (vips_sink_screen (out, display_image, NULL,
			64, 64, (2000/64)*(2000/64), 
			//6400, 64, (2000/64), 
			0, NULL, NULL))
  std::cout<<std::endl;
  std::cout<<"refcount after sink_screen:  out("<<out<<")="<<G_OBJECT(out)->ref_count
	   <<"  display("<<display_image<<")="<<G_OBJECT(display_image)->ref_count<<std::endl;

  g_object_unref( out );
  g_object_unref( display_image );
  std::cout<<"refcount after unref(out,display):  out("<<out<<")="<<G_OBJECT(out)->ref_count
	   <<"  display("<<display_image<<")="<<G_OBJECT(display_image)->ref_count<<std::endl;

  /*
  std::cout<<std::endl;
  std::cout<<"refcount before bandjoin2:  in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  in2("<<in2<<")="<<G_OBJECT(in2)->ref_count<<std::endl;
  if( vips_bandjoin2( in, in2, &out, NULL ) ) {
    vips_error_exit( "unable to bandjoin2" ); 
  }
  std::cout<<"refcount after bandjoin2:  in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  in2("<<in2<<")="<<G_OBJECT(in2)->ref_count
	   <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;

  g_object_unref(in);
  g_object_unref(in2);
  std::cout<<"refcount after unref(in,in2):  in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  in2("<<in2<<")="<<G_OBJECT(in2)->ref_count
	   <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;

  g_object_unref( out );
  std::cout<<"refcount after unref(out):  in("<<in<<")="<<G_OBJECT(in)->ref_count
	   <<"  in2("<<in2<<")="<<G_OBJECT(in2)->ref_count
	   <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;
  */
  g_object_unref( in2 );
  std::cout<<"-----------------------------------"<<std::endl;
  return( 0 ); 
}
