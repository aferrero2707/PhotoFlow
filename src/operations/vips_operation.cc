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
#include <vips/vips.h>

#include "vips_operation.hh"

/* argument_class has all the static stuff about this argument: priority,
 * type, flags, all that.
 *
 * argument_instance has the info about this particular slot on this
 * particular operation: has it been supplied yet, what object is it attached
 * to, close handles, and so on.
 */
static void* show_required_optional( VipsObject *operation, GParamSpec *pspec,
			VipsArgumentClass *argument_class, 
			VipsArgumentInstance *argument_instance, void *a, void *b )
{
  gboolean required = *((gboolean *) a);
  PF::VipsOperationPar* par = (PF::VipsOperationPar *)b;
 
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
    par->add_argument( pspec, argument_class );
 
  return( NULL ); 
}
 
static int usage( const char *operation_name, PF::VipsOperationPar* par )
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
		     show_required_optional, &required, par ); 
  printf( "optional input arguments:\n" );
  required = FALSE;
  vips_argument_map( VIPS_OBJECT( operation ), 
		     show_required_optional, &required, par ); 
 
  g_object_unref( operation ); 
 
  return( 0 ); 
}
 



PF::VipsOperationPar::VipsOperationPar(): 
  PF::BlenderPar()
{
  //set_demand_hint( VIPS_DEMAND_STYLE_THINSTRIP );
  set_demand_hint( VIPS_DEMAND_STYLE_ANY );
  //blender = new PF::Processor<PF::BlenderProc,PF::BlenderPar>();
  //set_opacity(0.5);
  //PF::PropertyBase* p = blender.get_par()->get_property( "opacity" );
  //if(p) map_property( p );
}


void PF::VipsOperationPar::set_op(std::string name)
{
  op_name = name;
  usage( op_name.c_str(), this );
  
  set_type( std::string("vips-")+name );
}


void PF::VipsOperationPar::add_argument( GParamSpec *pspec, VipsArgumentClass *argument_class )
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
 
  PropertyBase* prop = NULL;

  if( g_type_is_a( otype, VIPS_TYPE_IMAGE ) ) {
    // Images are handled separately, because the value can only be set at build time
    // The image pointer are taken from the input vector passed to the "build()" method
    printf( "\t\tVipsImage\n" );
    //prop = new PF::Property< pair<std::string,int> >(g_param_spec_get_name( pspec ),
    //						     this,NULL);
  } 
  else if( g_type_is_a( otype, VIPS_TYPE_OBJECT ) &&
	   (oclass = (VipsObjectClass*)g_type_class_ref( otype )) ) {
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
    prop = new PF::Property<bool>(g_param_spec_get_name( pspec ),
				  this, pspec_bool->default_value);
  }
  else if( G_IS_PARAM_SPEC_INT( pspec ) ) {
    GParamSpecInt *pspec_int = G_PARAM_SPEC_INT( pspec ); 
 
    /* Things like image coordnates. 
     */
    printf( "\t\tint - min = %d, max = %d, default = %d\n", 
	    pspec_int->minimum,
	    pspec_int->maximum,
	    pspec_int->default_value );
    prop = new PF::Property<int>(g_param_spec_get_name( pspec ),
				 this, pspec_int->default_value);
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
    prop = new PF::Property<uint64_t>(g_param_spec_get_name( pspec ),
				      this, pspec_uint64->default_value);
  }
  else if( G_IS_PARAM_SPEC_DOUBLE( pspec ) ) {
    GParamSpecDouble *pspec_double = G_PARAM_SPEC_DOUBLE( pspec ); 
 
    /* Things like radius. 
     */
    printf( "\t\tdouble - min = %g, max = %g, default = %g\n", 
	    pspec_double->minimum,
	    pspec_double->maximum,
	    pspec_double->default_value );
    prop = new PF::Property<double>(g_param_spec_get_name( pspec ),
    				    this, pspec_double->default_value);
  }
  else if( G_IS_PARAM_SPEC_ENUM( pspec ) ) {
    GParamSpecEnum *pspec_enum = G_PARAM_SPEC_ENUM( pspec ); 
    int i = pspec_enum->default_value;
 
    prop = new PF::PropertyBase(g_param_spec_get_name( pspec ), this,
				i, pspec_enum->enum_class->values[i].value_name,
				pspec_enum->enum_class->values[i].value_nick );
    /* Map to an option menu?
     */
    printf( "\t\tenum - values:\n" ); 
    for( i = 0; i < (int)pspec_enum->enum_class->n_values - 1; i++ ) {
      printf( "\t\t\t%d - %s (%s)\n", 
	      i, 
	      pspec_enum->enum_class->values[i].value_nick,
	      pspec_enum->enum_class->values[i].value_name );
      prop->add_enum_value( i, pspec_enum->enum_class->values[i].value_name,
			    pspec_enum->enum_class->values[i].value_nick );
    }
    printf( "\t\tdefault = %d\n", pspec_enum->default_value ); 
  }
  else if( G_IS_PARAM_SPEC_BOXED( pspec ) ) {
    if( g_type_is_a( otype, VIPS_TYPE_ARRAY_INT ) ) {
      printf( "\t\tarray of int\n" );
      //prop = new PF::Property< std::vector<int> >( g_param_spec_get_name( pspec ),
      //					   this,std::vector<int>() );
    }
    else if( g_type_is_a( otype, VIPS_TYPE_ARRAY_DOUBLE ) ) {
      printf( "\t\tarray of double\n" );
      //prop =new PF::Property< std::vector<double> >( g_param_spec_get_name( pspec ),
      //					     this,std::vector<double>() );
    }
    else if( g_type_is_a( otype, VIPS_TYPE_ARRAY_IMAGE ) ) {
      printf( "\t\tarray of images\n" );
      //prop = new PF::Property< std::vector< pair<std::string,int> > >( g_param_spec_get_name( pspec ),
      //							       this,std::vector< pair<std::string,int> >() );
    }
    else
      printf( "unsupported boxed type\n" ); 
  }
  else
    printf( "\t\tunsupported type\n" ); 
}
 


VipsImage* PF::VipsOperationPar::build(std::vector<VipsImage*>& in, int first, 
				       VipsImage* imap, VipsImage* omap, 
				       unsigned int& level)
{
  int in_img_id = first;
  VipsOperation* operation;
  if( !(operation = vips_operation_new( op_name.c_str() ) ) )
    return( NULL );

  VIPS_ARGUMENT_FOR_ALL( operation, 
			 pspec, argument_class, argument_instance ) {
    
    g_assert( argument_instance );
    
    if( (argument_class->flags & VIPS_ARGUMENT_INPUT) ) {
      GType otype = G_PARAM_SPEC_VALUE_TYPE( pspec );
      const gchar* arg_name = g_param_spec_get_name( pspec );

      if( g_type_is_a( otype, VIPS_TYPE_IMAGE ) ) {

	if( in_img_id < (int)in.size() ) {
	  VipsImage* img = in[in_img_id];
	  if( img )
	    g_object_set( operation, arg_name, img, NULL );
	  in_img_id += 1;
	}

      } else {

	PF::PropertyBase* prop = get_property( arg_name );
	if( prop ) prop->set_gobject( operation );

      }
    }
  } 
  VIPS_ARGUMENT_FOR_ALL_END;
      
  /* Build from cache.
   */
  if( vips_cache_operation_buildp( &operation ) ) {
    std::cout<<"Error: "<<vips_error_buffer()<<std::endl;
    return( NULL );
  }
  


  VipsImage* out = NULL;

  VIPS_ARGUMENT_FOR_ALL( operation, 
			 pspec, argument_class, argument_instance ) {
    
    g_assert( argument_instance );
    
    if( (argument_class->flags & VIPS_ARGUMENT_OUTPUT) ) {
      GType otype = G_PARAM_SPEC_VALUE_TYPE( pspec );
      const gchar* arg_name = g_param_spec_get_name( pspec );

      if( g_type_is_a( otype, VIPS_TYPE_IMAGE ) &&
	  out == NULL ) {

	g_object_get( operation, arg_name, &out, NULL );
	if( out != NULL )
	  g_object_unref( out ); 

      }
    }
  } 
  VIPS_ARGUMENT_FOR_ALL_END;

  std::cout<<op_name<<" -> output image: "<<(void*)out<<std::endl;

  std::vector<VipsImage*> in2;
  in2.push_back( in[first] );
  in2.push_back( out );
  
  VipsImage* out2 = PF::BlenderPar::build( in2, 0, NULL, omap, level );
  VIPS_UNREF( in[first] );
  VIPS_UNREF( out );

  return out2;
}
