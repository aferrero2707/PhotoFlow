/* vips8 introspection demo
 *
 * compile with:
 *
 * 	gcc -g -Wall introspect.c `pkg-config vips --cflags --libs`
 *
 * try:
 *
 * 	./a.out embed
 *
 * for a complicated example.
 *
 * 1/2/14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include <vips/vips.h>

static void *
list_class( GType type )
{
	int depth = vips_type_depth( type );
	int i;

	for( i = 0; i < depth * 2; i++ )
		printf( " " );

	/* This printf()s a one-line note about the class:
	 * 	CanonicalName (nickname), summary[extra properties]
	 * for example:
	 * 	VipsAdd (add), add two images
	 * The GType must refer to a sub-class of VipsObject (obviously). 
	 */
	vips_object_print_summary_class( 
		VIPS_OBJECT_CLASS( g_type_class_ref( type ) ) );

	return( NULL );
}

static void
show_argument( GParamSpec *pspec, VipsArgumentClass *argument_class )
{
	GType otype = G_PARAM_SPEC_VALUE_TYPE( pspec );
        VipsObjectClass *oclass;

	/* See
	 * https://developer.gnome.org/gobject/2.38/gobject-GParamSpec.html
	 */
	printf( "\t%s\n", g_param_spec_get_name( pspec ) );
	printf( "\t\t%s\n", g_param_spec_get_nick( pspec ) );
	printf( "\t\t%s\n", g_param_spec_get_blurb( pspec ) );

	/* You can walk GParamSpec to work out what to supply. See
	 * 
	 * https://github.com/jcupitt/nip2/blob/master/src/vipsobject.c
	 *
	 * for a complete example. That code links nip2's heap objects up to
	 * libvips's GValues.
	 *
	 * For exaple, if the pspec is an enum, you can get the GType of the
	 * enum, then walk that to get all the possible settings, a
	 * description of each one, and a default value.
	 */

	if( g_type_is_a( otype, VIPS_TYPE_IMAGE ) ) 
		printf( "\t\tVipsImage\n" );
	else if( g_type_is_a( otype, VIPS_TYPE_OBJECT ) &&
		(oclass = g_type_class_ref( otype )) ) {
		printf( "\t\tVipsObject subclass - %s\n", oclass->description );
		/* You could construct from a string with
		 * vips_object_new_from_string().
		 */
	}
	else if( G_IS_PARAM_SPEC_BOOLEAN( pspec ) ) {
		GParamSpecBoolean *pspec_bool = G_PARAM_SPEC_BOOLEAN( pspec ); 

		/* Make a toggle widget, perhaps.
		 */
		printf( "\t\tbool - default = %d\n", 
			pspec_bool->default_value );
	}
	else if( G_IS_PARAM_SPEC_INT( pspec ) ) {
		GParamSpecInt *pspec_int = G_PARAM_SPEC_INT( pspec ); 

		/* Things like image coordnates. 
		 */
		printf( "\t\tint - min = %d, max = %d, default = %d\n", 
			pspec_int->minimum,
			pspec_int->maximum,
			pspec_int->default_value );
	}
	else if( G_IS_PARAM_SPEC_UINT64( pspec ) ) {
		GParamSpecUInt64 *pspec_uint64 = G_PARAM_SPEC_UINT64( pspec ); 

		/* Things like file offsets or object sizes.
		 */
		printf( "\t\tuint64 - min = %" G_GUINT64_FORMAT 
			", max = %" G_GUINT64_FORMAT 
			", default = %" G_GUINT64_FORMAT "\n", 
			pspec_uint64->minimum,
			pspec_uint64->maximum,
			pspec_uint64->default_value );
	}
	else if( G_IS_PARAM_SPEC_DOUBLE( pspec ) ) {
		GParamSpecDouble *pspec_double = G_PARAM_SPEC_DOUBLE( pspec ); 

		/* Things like radius. 
		 */
		printf( "\t\tdouble - min = %g, max = %g, default = %g\n", 
			pspec_double->minimum,
			pspec_double->maximum,
			pspec_double->default_value );
	}
	else if( G_IS_PARAM_SPEC_ENUM( pspec ) ) {
		GParamSpecEnum *pspec_enum = G_PARAM_SPEC_ENUM( pspec ); 
		int i;

		/* Map to an option menu?
		 */
		printf( "\t\tenum - values:\n" ); 
		for( i = 0; i < pspec_enum->enum_class->n_values - 1; i++ ) 
			printf( "\t\t\t%d - %s (%s)\n", 
				i, 
				pspec_enum->enum_class->values[i].value_nick,
				pspec_enum->enum_class->values[i].value_name );
		printf( "\t\tdefault = %d\n", pspec_enum->default_value ); 
	}
	else if( G_IS_PARAM_SPEC_BOXED( pspec ) ) {
		if( g_type_is_a( otype, VIPS_TYPE_ARRAY_INT ) ) {
			printf( "\t\tarray of int\n" );
		}
		else if( g_type_is_a( otype, VIPS_TYPE_ARRAY_DOUBLE ) ) {
			printf( "\t\tarray of double\n" );
		}
		else if( g_type_is_a( otype, VIPS_TYPE_ARRAY_IMAGE ) ) {
			printf( "\t\tarray of images\n" );
		}
		else
			printf( "unsupported boxed type\n" ); 
	}
	else
		printf( "\t\tunsupported type\n" ); 
}

/* argument_class has all the static stuff about this argument: priority,
 * type, flags, all that.
 *
 * argument_instance has the info about this particular slot on this
 * particular operation: has it been supplied yet, what object is it attached
 * to, close handles, and so on.
 */
static void *
show_required_optional( VipsObject *operation, GParamSpec *pspec,
	VipsArgumentClass *argument_class, 
	VipsArgumentInstance *argument_instance, void *a, void *b )
{
	gboolean required = *((gboolean *) a);

	/* See
	 * http://www.vips.ecs.soton.ac.uk/supported/7.38/doc/html/libvips/libvips-VipsArgument.html#VipsArgumentFlags
	 */

	/* Skip deprecated args.
	 */
	if( argument_class->flags & VIPS_ARGUMENT_DEPRECATED ) 
		return( NULL ); 

	/* Only input args ... we're only interested in ones we must supply.
	 *
	 * If we supplied all required input args and then built the
	 * operation with vips_object_build(), we could have a second pass 
	 * which walked the object again and extracted all the output args.
	 *
	 * Use vips_cache_operation_buildp() to build an operation with the
	 * vips operation cache.
	 */
	if( !(argument_class->flags & VIPS_ARGUMENT_INPUT) ) 
		return( NULL ); 

	/* Skip things which are not needed at construct time.
	 */
	if( !(argument_class->flags & VIPS_ARGUMENT_CONSTRUCT) ) 
		return( NULL ); 

	if( (argument_class->flags & VIPS_ARGUMENT_REQUIRED) == required )  
		show_argument( pspec, argument_class );

	return( NULL ); 
}

static int
usage( const char *operation_name )
{
	VipsOperation *operation;
	gboolean required;

	/* Make an operation, ready for arguments to be supplied.
	 *
	 * In this example, we just loop over the args and show them, but we
	 * could also supply values with (eg.)
	 *
	 * 	VipsImage *left = ...;
	 * 	g_object_set( operation, "left", left, NULL ); 
	 *
	 * Or you can use GValue, which is a generic way of expressing
	 * values useful for language bindings:
	 *
	 * 	GValue gvalue = { 0 };
	 * 
	 * 	if( !convert_my_values_to_gvalue( my_value, &gvalue ) ) {
	 *		g_value_unset( &gvalue );
	 *		return( error code );
	 * 	}
	 * 	const char *arg_name = ...;
	 * 	g_object_set_property( G_OBJECT( operation ), 
	 * 		arg_name, &gvalue );
	 * 	g_value_unset( &gvalue );
	 *
	 * If we really wanted to just show arguments, we could use the
	 * parallel API vips_argument_class_map() which loops over the class
	 * rather than a class instance.
	 *
	 * We are walking subclasses of VipsOperation here (ie. all the vips
	 * operations),  but all this API works for any subclass of
	 * VipsObject. You can also introspect VipsImage, for example.
	 */
	if( !(operation = vips_operation_new( operation_name ) ) )
		return( -1 );

	/* See:
	 * http://www.vips.ecs.soton.ac.uk/supported/7.38/doc/html/libvips/libvips-VipsArgument.html#vips-argument-map
	 */
	printf( "%s:\n", operation_name ); 
	printf( "required input arguments:\n" );
	required = TRUE;
	vips_argument_map( VIPS_OBJECT( operation ), 
		show_required_optional, &required, NULL ); 
	printf( "optional input arguments:\n" );
	required = FALSE;
	vips_argument_map( VIPS_OBJECT( operation ), 
		show_required_optional, &required, NULL ); 

	g_object_unref( operation ); 

	return( 0 ); 
}

int
main( int argc, char **argv )
{
	GOptionContext *context;
	GOptionGroup *main_group;

	GError *error = NULL;

	if( vips_init( argv[0] ) )
	        vips_error_exit( NULL );

	context = g_option_context_new( "[OPERATION] - introspection demo" );

	main_group = g_option_group_new( NULL, NULL, NULL, NULL, NULL );
	g_option_context_set_main_group( context, main_group );
	g_option_context_add_group( context, vips_get_option_group() );

	if( !g_option_context_parse( context, &argc, &argv, &error ) ) {
		if( error ) {
			fprintf( stderr, "%s\n", error->message );
			g_error_free( error );
		}

		vips_error_exit( NULL );
	}

	if( argc == 1 ) {
		printf( "all vips operations:\n" ); 
		vips_type_map_all( g_type_from_name( "VipsOperation" ), 
			(VipsTypeMapFn) list_class, NULL );
	}
	else {
		if( usage( argv[1] ) )
			vips_error_exit( NULL );
	}

	g_option_context_free( context );

	vips_shutdown();

	return( 0 );
}