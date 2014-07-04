#include <string.h>
#include <vips/vips.h>
#include <gtkmm/main.h>

#include "../base/image.hh"

#include "../operations/image_reader.hh"
#include "../operations/invert.hh"
//#include "../operations/brightness_contrast.hh"
//#include "../operations/gradient.hh"

//#include "../gui/operations/brightness_contrast_config.hh"

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

	VIPS_ARG_INT( klass, "image_max", 4, 
		"Image maximum", 
		"Maximum value in image: pivot about this",
		VIPS_ARGUMENT_OPTIONAL_INPUT,
		G_STRUCT_OFFSET( Negative, image_max ),
		0, 255, 255 );

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

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

  extern GType vips_layer_get_type( void ); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

int 
main( int argc, char **argv )
{
  Gtk::Main kit(argc, argv);

  VipsImage *in;
  VipsImage *out;
  VipsImage *temp;


  if( vips_init( argv[0] ) )
    vips_error_exit( "unable to init" ); 

  im_concurrency_set( 1 );

  /* Add negative to the class hierarchy. You'll now be able to use it
   * exactly like any built-in vips operation and it'll appear in
   * Python, nip2, C, C++ etc etc. 
   */
  negative_get_type();
  vips_layer_get_type();

  if( argc != 4 )
    vips_error_exit( "usage: %s vips/pf infile outfile", argv[0] ); 

  bool check_vips = true;
  if(!strcmp(argv[1],"pf")) check_vips = false;

  if( check_vips ) {
    if( !(in = vips_image_new_from_file( argv[2] )) )
      vips_error_exit( "unable to open" ); 

    std::cout<<"in refcount after new_from_file:  "<<G_OBJECT(in)->ref_count<<std::endl;

    temp = in;

    for(int i = 0; i < 4; i++) {
    
      std::cout<<std::endl;
      std::cout<<"refcount before negative("<<i<<"): in("<<temp<<")="<<G_OBJECT(temp)->ref_count<<std::endl;
      if( negative( temp, &out, "image_max", 128, NULL ) ) {
	g_object_unref( temp );
	vips_error_exit( "unable to invert" ); 
      }
      std::cout<<"refcount after negative("<<i<<"):  in("<<temp<<")="<<G_OBJECT(temp)->ref_count
	       <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;

      /* We have a ref to out, out holds a ref to the negative operation,
       * and the operation holds a ref to in. We can safely unref in and
       * it'll be unreffed when we unref out.
       */
      g_object_unref( temp );
      std::cout<<std::endl;
      std::cout<<"refcount after unref(in):  in("<<temp<<")="<<G_OBJECT(temp)->ref_count
	       <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;
      std::cout<<"-----------------------------------"<<std::endl;
      temp = out;

    }

    printf("Writing output image...\n");

    std::cout<<"refcount before write_to_file:  in("<<in<<")="<<G_OBJECT(in)->ref_count
	       <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;
    if( vips_image_write_to_file( out, argv[3] ) ) { 
      g_object_unref( out );
      vips_error_exit( "unable to write" ); 
    }

    printf("...done\n");

    std::cout<<"refcount after write_to_file:  in("<<in<<")="<<G_OBJECT(in)->ref_count
	       <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;
    g_object_unref( out );
    std::cout<<"refcount after unref(out):  in("<<in<<")="<<G_OBJECT(in)->ref_count
	       <<"  out("<<out<<")="<<G_OBJECT(out)->ref_count<<std::endl;
  } else {

    std::vector<VipsImage*> in;

    PF::Processor<PF::ImageReaderPar,PF::ImageReader>* imgread = 
      new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
    imgread->get_par()->set_file_name( argv[2] );

    PF::Image* pf_image = new PF::Image();
    pf_image->add_view( VIPS_FORMAT_UCHAR, 0 );
    PF::LayerManager& layer_manager = pf_image->get_layer_manager();

    PF::Layer* limg = layer_manager.new_layer();
    limg->set_processor( imgread );
    limg->set_name( "input image" );
    layer_manager.get_layers().push_back( limg );

    for(int i = 0; i < 4; i++) {    
      PF::Layer* linv1 = layer_manager.new_layer();
      linv1->set_processor( new PF::Processor<PF::InvertPar,PF::Invert>() );
      linv1->set_name( "invert" );
      layer_manager.get_layers().push_back( linv1 );
    }

    pf_image->do_update();

    PF::View* view = pf_image->get_view( 0 );

    VipsImage* image = view->get_output();

    printf("Writing output image...\n");

    if( vips_image_write_to_file( view->get_output(), argv[3] ) ) { 
      //g_object_unref( out );
      vips_error_exit( "unable to write" ); 
    }

    printf("...done\n");

    delete pf_image;

  }

  return( 0 ); 
}
