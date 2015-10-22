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

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif /*HAVE_CONFIG_H*/
//#include <vips/intl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <lcms2.h>

#include <iostream>


#include <vips/dispatch.h>

#include "../base/processor.hh"
#include "../operations/perspective.hh"

#include "../dt/iop/clipping.cc"

#define PF_MAX_INPUT_IMAGES 10

static GObject* object_in;

/**/
#define VIPS_TYPE_PERSPECTIVE (vips_perspective_get_type())
#define VIPS_PERSPECTIVE( obj ) \
	(G_TYPE_CHECK_INSTANCE_CAST( (obj), \
		VIPS_TYPE_PERSPECTIVE, VipsPerspective ))
#define VIPS_PERSPECTIVE_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_CAST( (klass), \
		VIPS_TYPE_PERSPECTIVE, VipsPerspectiveClass))
#define VIPS_IS_PERSPECTIVE( obj ) \
	(G_TYPE_CHECK_INSTANCE_TYPE( (obj), VIPS_TYPE_PERSPECTIVE ))
#define VIPS_IS_PERSPECTIVE_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_TYPE( (klass), VIPS_TYPE_PERSPECTIVE ))
#define VIPS_PERSPECTIVE_GET_CLASS( obj ) \
	(G_TYPE_INSTANCE_GET_CLASS( (obj), \
		VIPS_TYPE_PERSPECTIVE, VipsPerspectiveClass ))
/**/
typedef struct _VipsPerspective {
  VipsOperation parent_instance;
  
  /* The vector of input images.
   */
  VipsImage* in;

  /* The vector of input images.
   */
  VipsImage* out;

  /* Pointer to the object which does the actual image processing
   */
  PF::ProcessorBase* processor;

  VipsInterpolate* interpolate;

  /* The preferred output style for this layer
   */
  VipsDemandStyle demand_hint;

  dt_iop_clipping_data_t data;

} VipsPerspective;

/*
typedef struct _VipsPerspectiveClass {
	VipsOperationClass parent_class;
} VipsPerspectiveClass;
*/
typedef VipsOperationClass VipsPerspectiveClass;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

G_DEFINE_TYPE( VipsPerspective, vips_perspective, VIPS_TYPE_OPERATION );

#ifdef __cplusplus
}
#endif /*__cplusplus*/



#define MIN_MAX( MIN, MAX, VAL) { if(VAL<MIN) MIN=VAL; if(VAL>MAX) MAX=VAL;}


template<class T>
static int
vips_perspective_gen_template( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion *ir = (VipsRegion *) seq;
  VipsPerspective *perspective = (VipsPerspective *) b;
  const int window_size =
    vips_interpolate_get_window_size( perspective->interpolate );
  const int window_offset =
    vips_interpolate_get_window_offset( perspective->interpolate );
  
  PF::PerspectivePar* par = dynamic_cast<PF::PerspectivePar*>( perspective->processor->get_par() );

  /* Do the actual processing
   */

  /* Output area we are building.
   */
  const VipsRect *r = &oreg->valid;
  VipsRect s, r_in;
  int i;
  int x, xx, y, k;
  dt_iop_roi_t roi_in, roi_out;
  
  /* Area of input we need.
   */
#ifndef NDEBUG
  std::cout<<"vips_perspective_gen(): mapping output region top="<<oreg->valid.top
     <<" left="<<oreg->valid.left
     <<" width="<<oreg->valid.width
     <<" height="<<oreg->valid.height<<std::endl;
#endif

  roi_out.x = oreg->valid.left;
  roi_out.y = oreg->valid.top;
  roi_out.width = oreg->valid.width;
  roi_out.height = oreg->valid.height;
  modify_roi_in( &(perspective->data), &roi_out, &roi_in, ir->im->Xsize, ir->im->Ysize );
  //std::cout<<"vips_perspective_gen_template(): roi_in="
  //    <<roi_in.width<<","<<roi_in.height<<"+"<<roi_in.x<<"+"<<roi_in.y<<std::endl;
  //std::cout<<"vips_perspective_gen_template(): roi_out="
  //    <<roi_out.width<<","<<roi_out.height<<"+"<<roi_out.x<<"+"<<roi_out.y<<std::endl;

  s.left = roi_in.x - window_offset - 1;
  s.top = roi_in.y - window_offset - 1;
  s.width = roi_in.width + window_size + 2 - 1;
  s.height = roi_in.height + window_size + 2 - 1;

  VipsRect rimg = { 0, 0, ir->im->Xsize, ir->im->Ysize };
  vips_rect_intersectrect( &rimg, &s, &s );

  r_in.left = s.left + window_offset;
  r_in.top = s.top + window_offset;
  r_in.width = s.width - window_size + 1;
  r_in.height = s.height - window_size + 1;

  /**/
#ifndef NDEBUG
  std::cout<<"vips_perspective_gen(): "<<std::endl;
  std::cout<<"  input region:  top="<<s.top
	   <<" left="<<s.left
	   <<" width="<<s.width
	   <<" height="<<s.height<<std::endl
	   <<"  output region: top="<<oreg->valid.top
	   <<" left="<<oreg->valid.left
	   <<" width="<<oreg->valid.width
	   <<" height="<<oreg->valid.height<<std::endl;
#endif
  /**/
  /* Prepare the input images
   */
  if(ir) {
#ifndef NDEBUG
    std::cout<<"  preparing region ir:  top="<<s.top
        <<" left="<<s.left
        <<" width="<<s.width
        <<" height="<<s.height<<std::endl;
#endif
    /**/
    if( vips_region_prepare( ir, &s ) )
      return( -1 );
  }

  /* Do the actual processing
   */
#ifndef NDEBUG
  std::cout<<"perspective->in->Bands="<<perspective->in->Bands<<std::endl;
#endif
  int line_size = r->width * perspective->in->Bands;
  struct dt_iop_clipping_data_t *d = &(perspective->data);
  //std::cout<<"d->enlarge_x="<<d->enlarge_x<<"  d->enlarge_y="<<d->enlarge_y<<std::endl;
  //std::cout<<"d->offset_x="<<d->offset_x<<"  d->offset_y="<<d->offset_y<<std::endl;
  float o[2];
  VipsInterpolateMethod interp_method =
      vips_interpolate_get_method ( perspective->interpolate );
  for( y = 0; y < r->height; y++ ) {
    T *q = (T *)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
    for( x = 0; x < line_size; x+=perspective->in->Bands ) {
      float outx = r->left + d->offset_x - d->enlarge_x + x/perspective->in->Bands + 0.5f;
      float outy = r->top + d->offset_y - d->enlarge_y + y + 0.5f;
      o[0] = outx/ir->im->Xsize; o[1] = outy/ir->im->Ysize;
      keystone_backtransform(o, d->k_space, d->a, d->b, d->d, d->e, d->g, d->h, d->kxa, d->kya);
      int srcx = o[0]*ir->im->Xsize;
      int srcy = o[1]*ir->im->Ysize;
      //std::cout<<"out="<<outx<<","<<outy<<"    src="<<srcx<<","<<srcy<<std::endl;
      if( vips_rect_includespoint(&r_in, srcx, srcy) ) {
        interp_method( perspective->interpolate, &(q[x]), ir, o[0]*ir->im->Xsize, o[1]*ir->im->Ysize );
        //T *p = (T *)VIPS_REGION_ADDR( ir, srcx, srcy );
        //for( xx = 0; xx < perspective->in->Bands; xx++ ) {
        //  q[x+xx] = p[xx];
        //}
        //if( (r->top+x/perspective->in->Bands) == 1000 && (r->left+y) == 1000 )
        //{
          //std::cout<<"r->height="<<r->height<<" line_size="<<line_size<<std::endl;
          //std::cout<<"x="<<x/perspective->in->Bands<<" y="<<y<<std::endl;
          //std::cout<<"out="<<outx<<","<<outy<<"    src="<<srcx<<","<<srcy<<std::endl;
          //std::cout<<"  p="<<p[0]<<" "<<p[1]<<" "<<p[2]<<std::endl;
        //}
      } else {
        for( xx = 0; xx < perspective->in->Bands; xx++ ) {
          q[x+xx] = PF::FormatInfo<T>::MIN;
        }
      }
    }
  }
  /**/
#ifndef NDEBUG
  std::cout<<"vips_perspective_gen(): "<<std::endl
	   <<"  bands = "<<oreg->im->Bands<<std::endl
	   <<"  fmt = "<<oreg->im->BandFmt<<std::endl
	   <<"  colorspace = "<<oreg->im->Type<<std::endl;
#endif
  /**/

  return( 0 );
}


static int
vips_perspective_gen( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  //std::cout<<"vips_perspective_gen() called."<<std::endl;

  VipsRegion *ir = (VipsRegion *) seq;
  if( !ir ) return 1;

  int result = 0;

  switch( ir->im->BandFmt ) {
  case VIPS_FORMAT_UCHAR:
    result = vips_perspective_gen_template<unsigned char>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_USHORT:
    result = vips_perspective_gen_template<unsigned short int>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_FLOAT:
    result = vips_perspective_gen_template<float>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_DOUBLE:
    result = vips_perspective_gen_template<double>( oreg, seq, a, b, stop );
    break;
  }

  return result;
}


static int
vips_perspective_build( VipsObject *object )
{
  VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
  VipsOperation *operation = VIPS_OPERATION( object );
  VipsPerspective *perspective = (VipsPerspective *) object;
  int i;

  int window_size = vips_interpolate_get_window_size( perspective->interpolate );
  int window_offset =
    vips_interpolate_get_window_offset( perspective->interpolate );
  VipsDemandStyle hint;

  //g_print("vips_perspective_build() called. in=%p\n", perspective->in);
  //g_print("vips_perspective_build() window_offset=%d window_size=%d\n",
  //    window_offset, window_size);

  if( VIPS_OBJECT_CLASS( vips_perspective_parent_class )->build( object ) )
    return( -1 );

  //perspective->rand = 0;/*random();*/

  // Count total number of input images
  if( vips_image_pio_input( perspective->in ) ||
      vips_check_coding_known( klass->nickname, perspective->in ) )
    return( -1 );

  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( perspective, "out", vips_image_new(), NULL );

  /* Add new pixels around the input so we can interpolate at the edges.
   */
  VipsImage* enlarged = perspective->in;
  /*
  if( vips_embed( perspective->in, &enlarged,
    window_offset, window_offset,
    perspective->in->Xsize + window_size - 1,
    perspective->in->Ysize + window_size - 1,
    "extend", VIPS_EXTEND_COPY,
    NULL ) )
    return( -1 );
    */

  g_object_ref( enlarged );

  /* Normally SMALLTILE ...
   */
  hint = VIPS_DEMAND_STYLE_SMALLTILE;

  /* Set demand hints. 
  */
  //std::cout<<"vips_perspective_build(): enlarged="<<enlarged<<std::endl;
  VipsImage* invec[2] = {enlarged, NULL};
  if( vips_image_pipelinev( perspective->out,
				 hint, enlarged, NULL ) )
    return( -1 );

  PF::PerspectivePar* par = dynamic_cast<PF::PerspectivePar*>( perspective->processor->get_par() );
  if( !par ) return( -1 );
  if( par->get_keystones().size() != 4 )
    return( -1 );

#ifndef NDEBUG
  std::cout<<"vips_perspective_build(): output format = "<<par->get_format()<<std::endl;
#endif

  dt_iop_roi_t roi_in, roi_out;
  roi_in.x = roi_in.y = 0;
  roi_in.width = enlarged->Xsize; //perspective->in->Xsize;
  roi_in.height = enlarged->Ysize; //perspective->in->Ysize;
  //std::cout<<"vips_perspective_build(): roi_in="<<roi_in.width<<","<<roi_in.height
  //    <<"+"<<roi_in.x<<"+"<<roi_in.y<<std::endl;

  dt_iop_clipping_params_t pars;
  pars.angle = 0;
  pars.cx = pars.cy = 0.0f; pars.cw = pars.ch = 1.0f;
  pars.k_h = pars.k_v = 0.0f;
  pars.kxa = par->get_keystones()[0].first;
  pars.kya = par->get_keystones()[0].second;
  pars.kxb = par->get_keystones()[1].first;
  pars.kyb = par->get_keystones()[1].second;
  pars.kxc = par->get_keystones()[2].first;
  pars.kyc = par->get_keystones()[2].second;
  pars.kxd = par->get_keystones()[3].first;
  pars.kyd = par->get_keystones()[3].second;
  pars.k_type = 3;
  pars.k_apply = 1;
  //dt_iop_clipping_data_t data;
  commit_params( &pars, &(perspective->data) );
  modify_roi_out( &(perspective->data), &roi_out, &roi_in );
  perspective->data.offset_x = roi_out.x;
  perspective->data.offset_y = roi_out.y;

  //std::cout<<"vips_perspective_build(): roi_out="<<roi_out.width<<","<<roi_out.height
  //    <<"+"<<roi_out.x<<"+"<<roi_out.y<<std::endl;

  vips_image_init_fields( perspective->out,
        //par->get_xsize(), par->get_ysize(),
      roi_out.width, roi_out.height,
        par->get_nbands(), par->get_format(),
        par->get_coding(),
        par->get_interpretation(),
        1.0, 1.0);
  if( vips_image_generate( perspective->out,
      vips_start_one, vips_perspective_gen, vips_stop_one,
      perspective->in, perspective ) )
    return( -1 );

  VIPS_UNREF( enlarged );

  //g_print("vips_perspective_build() finished!!!\n");
  return( 0 );
}


static void
vips_perspective_class_init( VipsPerspectiveClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( klass );
  VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( klass );

  gobject_class->set_property = vips_object_set_property;
  gobject_class->get_property = vips_object_get_property;

  vobject_class->nickname = "perspective";
  vobject_class->description = _( "Perspective correction" );
  vobject_class->build = vips_perspective_build;

  operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED/*+VIPS_OPERATION_NOCACHE*/;

  int argid = 0;

  VIPS_ARG_IMAGE( klass, "in", argid,
      _( "Input" ),
      _( "Input image" ),
      VIPS_ARGUMENT_REQUIRED_INPUT,
      G_STRUCT_OFFSET( VipsPerspective, in ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "out", argid, 
		  _( "Output" ), 
		  _( "Output image" ),
		  VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		  G_STRUCT_OFFSET( VipsPerspective, out ) );
  argid += 1;

  VIPS_ARG_POINTER( klass, "processor", argid, 
		    _( "Processor" ),
		    _( "Image processing object" ),
		    VIPS_ARGUMENT_REQUIRED_INPUT,
		    G_STRUCT_OFFSET( VipsPerspective, processor ) );
  argid += 1;

  VIPS_ARG_INTERPOLATE( klass, "interpolate", argid,
    _( "Interpolate" ),
    _( "Interpolate pixels with this" ),
    VIPS_ARGUMENT_REQUIRED_INPUT,
    G_STRUCT_OFFSET( VipsPerspective, interpolate ) );
}

static void
vips_perspective_init( VipsPerspective *perspective )
{
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
vips_perspective( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, VipsInterpolate* interpolate, ... )
{
  va_list ap;
  int result;

  va_start( ap, interpolate );
  result = vips_call_split( "perspective", ap, in, out, proc, interpolate );
  va_end( ap );

  return( result );
}
