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



/* Turn on ADDR() range checks.
#define DEBUG 1
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/
#include <vips/intl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <lcms2.h>

#include <iostream>


#include <vips/dispatch.h>

#include "../base/processor.hh"
#include "../base/layer.hh"

#define PF_MAX_INPUT_IMAGES 10

static GObject* object_in;

/**/
#define VIPS_TYPE_LAYER (vips_layer_get_type())
#define VIPS_LAYER( obj ) \
	(G_TYPE_CHECK_INSTANCE_CAST( (obj), \
		VIPS_TYPE_LAYER, VipsLayer ))
#define VIPS_LAYER_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_CAST( (klass), \
		VIPS_TYPE_LAYER, VipsLayerClass))
#define VIPS_IS_LAYER( obj ) \
	(G_TYPE_CHECK_INSTANCE_TYPE( (obj), VIPS_TYPE_LAYER ))
#define VIPS_IS_LAYER_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_TYPE( (klass), VIPS_TYPE_LAYER ))
#define VIPS_LAYER_GET_CLASS( obj ) \
	(G_TYPE_INSTANCE_GET_CLASS( (obj), \
		VIPS_TYPE_LAYER, VipsLayerClass ))
/**/
typedef struct _VipsLayer {
  VipsOperation parent_instance;
  
  /* The vector of input images.
   */
  VipsImage* in[PF_MAX_INPUT_IMAGES];
  int ninput;

  /* The vector of input images.
   */
  VipsImage* out;

  /* The index at which input images start
   * in[0] always corresponds to the lower layer of the blending step,
   * however it might not directly represent an input image for processing.
   * This is for example the case for composed operations: the last step
   * receives as input the result of the previous operation, but performs
   * the blending with the initial input image.
   */
  int in_first;

  /* Pointer to the object which does the actual image processing
   */
  PF::ProcessorBase* processor;

  /* The (optional) intensity map
   */
  VipsImage *imap;
  
  /* The (optional) opacity map
   */
  VipsImage *omap;
  
  /* The preferred output style for this layer
   */
  VipsDemandStyle demand_hint;

  int width;
  int height;
  int nbands;

  //int rand;
  
  /* All the imput images, including the intensity 
     and opacity masks
   */
  VipsImage **in_all;

} VipsLayer;

/*
typedef struct _VipsLayerClass {
	VipsOperationClass parent_class;
} VipsLayerClass;
*/
typedef VipsOperationClass VipsLayerClass;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

G_DEFINE_TYPE( VipsLayer, vips_layer, VIPS_TYPE_OPERATION );

#ifdef __cplusplus
}
#endif /*__cplusplus*/



/* Run the PhotoFlow image editing code
 */
static int
vips_layer_gen( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion **ir = (VipsRegion **) seq;
  VipsLayer *layer = (VipsLayer *) b;
  

  /* Do the actual processing
   */

  /* Output area we are building.
   */
  const VipsRect *r = &oreg->valid;
  VipsRect s;
  int i;
  int x, y;
  int ninput = layer->ninput;
  
  /* Area of input we need.
   */
  layer->processor->get_par()->transform_inv(r,&s);

  /*
#ifndef NDEBUG
  std::cout<<"vips_layer_gen(): "<<std::endl;
  if( layer->processor->get_par()->get_config_ui() )
    std::cout<<"  name: "<<layer->processor->get_par()->get_config_ui()->get_layer()->get_name()<<std::endl;
  std::cout<<"  input region:  top="<<s.top
	   <<" left="<<s.left
	   <<" width="<<s.width
	   <<" height="<<s.height<<std::endl
	   <<"  output region: top="<<oreg->valid.top
	   <<" left="<<oreg->valid.left
	   <<" width="<<oreg->valid.width
	   <<" height="<<oreg->valid.height<<std::endl;
#endif
  */
  /* Prepare the input images
   */
  if(ir) {
    for( i = 0; ir[i]; i++ ) {
      /*
#ifndef NDEBUG
      std::cout<<"  preparing region ir["<<i<<"]:  top="<<s.top
	       <<" left="<<s.left
	       <<" width="<<s.width
	       <<" height="<<s.height<<std::endl;
#endif
      */
      if( vips_region_prepare( ir[i], &s ) )
	return( -1 );
    }
  }

  /* Do the actual processing
   */
  /*
  int line_size = r->width * layer->in_all[0]->Bands; 
  for( y = 0; y < r->height; y++ ) {
    unsigned char *p = (unsigned char *)
      VIPS_REGION_ADDR( ir[0], r->left, r->top + y ); 
    unsigned char *q = (unsigned char *)
      VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 

    for( x = 0; x < line_size; x++ ) {
      q[x] = 255 - p[x];
      //std::cout<<"x="<<x<<"  p["<<x<<"]="<<(uint32_t)p[x]<<"  pout["<<x<<"]="<<(uint32_t)q[x]<<std::endl;
    }

  }
  */
  /*
  std::cout<<"vips_layer_gen(): "<<std::endl
	   <<"  bands = "<<oreg->im->Bands<<std::endl
	   <<"  fmt = "<<oreg->im->BandFmt<<std::endl
	   <<"  colorspace = "<<oreg->im->Type<<std::endl
	   <<"  imap = "<<layer->imap<<"  omap = "<<layer->omap<<std::endl;
  */

  // Get pointers to imap and omap regions
  VipsRegion* rimap = NULL;
  VipsRegion* romap = NULL;
  if(ir) {
    for( i = 0; ir[i]; i++ ) {
      //std::cout<<"  array["<<i<<"]="<<array[i]<<"  imap="<<layer->imap<<std::endl;
      if(ir[i]->im == layer->imap) { rimap = ir[i]; }
      if(ir[i]->im == layer->omap) { romap = ir[i]; }
      //std::cout<<"  rimap="<<rimap<<std::endl;
    }
  }

  //pf_process(pflayer->processor,r,&s,pflayer);
#ifndef NDEBUG
  //std::cout<<"Calling processor function..."<<std::endl;
#endif
  layer->processor->process(ir, ninput, layer->in_first, rimap, romap, oreg);
#ifndef NDEBUG
  //std::cout<<"...done"<<std::endl;
#endif
  return( 0 );
}


static int
vips_layer_build( VipsObject *object )
{
  VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
  VipsOperation *operation = VIPS_OPERATION( object );
  VipsLayer *layer = (VipsLayer *) object;
  int i;

  if( VIPS_OBJECT_CLASS( vips_layer_parent_class )->build( object ) )
    return( -1 );

  //layer->rand = 0;/*random();*/

  // Count total number of input images
  int nimg = layer->ninput;
  if(layer->imap) {
    nimg++;
  }
  if(layer->omap) {
    nimg++;
  }

  layer->in_all = (VipsImage**)im_malloc( layer->out, sizeof(VipsImage*)*(nimg+1) );
  if( !layer->in_all ) return( -1 );

  for( i = 0; i < layer->ninput; i++ ) {
    layer->in_all[i] = layer->in[i];
  }
  if( layer->imap ) { layer->in_all[i] = layer->imap; i+= 1; }
  if( layer->omap ) { layer->in_all[i] = layer->omap; i+= 1; }
  layer->in_all[nimg] = NULL;

  //if( layer->processor->identity() == 1 ) 
  //  return( vips_image_write( layer->in, conversion->out ) );

  for( i = 0; i < nimg; i++ ) {
    if( vips_image_pio_input( layer->in_all[i] ) || 
	vips_check_coding_known( klass->nickname, layer->in_all[i] ) )  
      return( -1 );
  }

  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( layer, "out", vips_image_new(), NULL ); 

  /* Set demand hints. 
  */
  if( vips_image_pipeline_array( layer->out, 
				 layer->demand_hint, 
				 layer->in_all ) )
    return( -1 );

  PF::OpParBase* par = layer->processor->get_par();

#ifndef NDEBUG
  std::cout<<"vips_layer_build(): layer->ninput="<<layer->ninput<<std::endl;
#endif

#ifndef NDEBUG
  std::cout<<"vips_layer_build(): output format = "<<par->get_format()<<std::endl;
#endif
  vips_image_init_fields( layer->out,
			  par->get_xsize(), par->get_ysize(), 
			  par->get_nbands(), par->get_format(),
			  par->get_coding(),
			  par->get_interpretation(),
			  1.0, 1.0);
  if(nimg > 0) {
    if( vips_image_generate( layer->out,
			     vips_start_many, vips_layer_gen, vips_stop_many, 
			     layer->in_all, layer ) )
      return( -1 );
  } else {
    if( vips_image_generate( layer->out, 
			     NULL, vips_layer_gen, NULL, NULL, layer ) )
      return( -1 );
  }

  return( 0 );
}


static void
vips_layer_class_init( VipsLayerClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( klass );
  VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( klass );

  gobject_class->set_property = vips_object_set_property;
  gobject_class->get_property = vips_object_get_property;

  vobject_class->nickname = "layer";
  vobject_class->description = _( "Photoflow layer" );
  vobject_class->build = vips_layer_build;

  operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED/*+VIPS_OPERATION_NOCACHE*/;

  int argid = 0;

  VIPS_ARG_INT( klass, "ninput", argid, 
		_( "NInput" ), 
		_( "Number of input images" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsLayer, ninput ),
		0, PF_MAX_INPUT_IMAGES, 0);
  argid += 1;

  VIPS_ARG_IMAGE( klass, "out", argid, 
		  _( "Output" ), 
		  _( "Output image" ),
		  VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		  G_STRUCT_OFFSET( VipsLayer, out ) );
  argid += 1;

  VIPS_ARG_POINTER( klass, "processor", argid, 
		    _( "Processor" ),
		    _( "Image processing object" ),
		    VIPS_ARGUMENT_REQUIRED_INPUT,
		    G_STRUCT_OFFSET( VipsLayer, processor ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "intensity_map", argid, 
		  _( "IntensityMap" ), 
		  _( "Intensity map" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT,
		  G_STRUCT_OFFSET( VipsLayer, imap ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "opacity_map", argid, 
		  _( "OpacityMap" ), 
		  _( "Opacity map" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT,
		  G_STRUCT_OFFSET( VipsLayer, omap ) );
  argid += 1;

  VIPS_ARG_ENUM( klass, "demand_hint", argid, 
		 _( "DemandHint" ), 
		 _( "Preferred demand style" ),
		 VIPS_ARGUMENT_REQUIRED_INPUT,
		 G_STRUCT_OFFSET( VipsLayer, demand_hint ),
		 VIPS_TYPE_DEMAND_STYLE, VIPS_DEMAND_STYLE_THINSTRIP );
  argid += 1;

  VIPS_ARG_INT( klass, "width", argid, 
		  _( "Width" ), 
		  _( "Image width" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsLayer, width ),
		0, 10000000, 0);
  argid += 1;

  VIPS_ARG_INT( klass, "height", argid, 
		  _( "Height" ), 
		  _( "Image height" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsLayer, height ),
		0, 10000000, 0);
  argid += 1;

  VIPS_ARG_INT( klass, "nbands", argid, 
		  _( "NBands" ), 
		  _( "Number of channels" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsLayer, nbands ),
		1, 1000, 3);
  argid += 1;

  char tstr[100];
  char tstr2[100];
  char tstr3[100];
  for( int imgid = 0; imgid < PF_MAX_INPUT_IMAGES; imgid++ ) {
    snprintf(tstr,99,"in%d",imgid);
    snprintf(tstr2,99,"Input%d",imgid);
    snprintf(tstr3,99,"Input image %d",imgid);
    VIPS_ARG_IMAGE( klass, tstr, argid, 
		    _( tstr2 ), 
		    _( tstr3 ),
		    VIPS_ARGUMENT_OPTIONAL_INPUT,
		    G_STRUCT_OFFSET( VipsLayer, in )+sizeof(VipsImage*)*imgid );
    argid += 1;
  }
}

static void
vips_layer_init( VipsLayer *layer )
{
  layer->in[0] = NULL;
}

/**
 * vips_layer:
 * @in: input image
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_layer( int n, VipsImage **out, PF::ProcessorBase* proc, 
	    VipsImage* imap, VipsImage* omap, VipsDemandStyle demand_hint,
	    int width, int height, int nbands, ...)
{
  va_list ap;
  int result;

  va_start( ap, nbands );
  result = vips_call_split( "layer", ap, n, out, proc, imap, omap, demand_hint, width, height, nbands );
  va_end( ap );

  return( result );
}

/*
    VipsImage* in = image; 
    VipsImage* out; 
    VipsImage* imap; 
    VipsArea *area;
    VipsImage **array; 
    const int N = 0;

    if (vips_call("layer", NULL, &imap, 0, gradient, NULL, NULL, NULL))
      verror ();
    //g_object_unref( imap );

    area = vips_area_new_array_object( 1 );
    array = (VipsImage **) area->data;
    array[0] = in;
    g_object_ref( array[0] );
    if (vips_call("layer", area, &out, 0, bc, imap, NULL, NULL))
      verror ();
    vips_area_unref( area );
    g_object_unref( out );
    in = out;

    for(int i = 0; i < N; i++) {
      area = vips_area_new_array_object( 1 );
      array = (VipsImage **) area->data;
      array[0] = in;
      g_object_ref( array[0] );
      if (vips_call("layer", area, &out, 0, invert, NULL, NULL, NULL))
        verror ();
      vips_area_unref( area );
*/
