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

#ifdef PF_HAS_LENSFUN
#include <lensfun.h>
#endif

#include <iostream>


#include <vips/dispatch.h>

#include "../base/processor.hh"
#include "../operations/lensfun.hh"

#define PF_MAX_INPUT_IMAGES 10

static GObject* object_in;

/**/
#define VIPS_TYPE_LENSFUN (vips_lensfun_get_type())
#define VIPS_LENSFUN( obj ) \
	(G_TYPE_CHECK_INSTANCE_CAST( (obj), \
		VIPS_TYPE_LENSFUN, VipsLensFun ))
#define VIPS_LENSFUN_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_CAST( (klass), \
		VIPS_TYPE_LENSFUN, VipsLensFunClass))
#define VIPS_IS_LENSFUN( obj ) \
	(G_TYPE_CHECK_INSTANCE_TYPE( (obj), VIPS_TYPE_LENSFUN ))
#define VIPS_IS_LENSFUN_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_TYPE( (klass), VIPS_TYPE_LENSFUN ))
#define VIPS_LENSFUN_GET_CLASS( obj ) \
	(G_TYPE_INSTANCE_GET_CLASS( (obj), \
		VIPS_TYPE_LENSFUN, VipsLensFunClass ))
/**/
typedef struct _VipsLensFun {
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

#ifdef PF_HAS_LENSFUN
  lfDatabase* ldb;
  lfModifier* modifier;
#endif
} VipsLensFun;

/*
typedef struct _VipsLensFunClass {
	VipsOperationClass parent_class;
} VipsLensFunClass;
*/
typedef VipsOperationClass VipsLensFunClass;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

G_DEFINE_TYPE( VipsLensFun, vips_lensfun, VIPS_TYPE_OPERATION );

#ifdef __cplusplus
}
#endif /*__cplusplus*/



#define MIN_MAX( MIN, MAX, VAL) { if(VAL<MIN) MIN=VAL; if(VAL>MAX) MAX=VAL;}

/* Run the PhotoFlow image editing code
 */
template<class T>
static int
vips_lensfun_gen_template( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion *ir = (VipsRegion *) seq;
  VipsLensFun *lensfun = (VipsLensFun *) b;
  

  /* Do the actual processing
   */

  /* Output area we are building.
   */
  const VipsRect *r = &oreg->valid;
  VipsRect s, r_in;
  int i;
  int x, xx, y, k;
  
  const int window_size =
    vips_interpolate_get_window_size( lensfun->interpolate );
  const int window_offset =
    vips_interpolate_get_window_offset( lensfun->interpolate );

  /* Area of input we need.
   */
#ifndef NDEBUG
  std::cout<<"vips_lensfun_gen(): mapping output region top="<<oreg->valid.top
     <<" left="<<oreg->valid.left
     <<" width="<<oreg->valid.width
     <<" height="<<oreg->valid.height<<std::endl;
#endif
  float* buf = new float[r->width*r->height*2*3];
#ifdef PF_HAS_LENSFUN
  bool ok = lensfun->modifier->ApplySubpixelGeometryDistortion( r->left, r->top, r->width, r->height, buf );
#endif
  int xmin=2000000000, xmax = -2000000000, ymin = 2000000000, ymax = -2000000000;
  float* pos = buf;
  for( x = 0; x < r->width; x++ ) {
    for( y = 0; y < r->height; y++ ) {
      for( k= 0; k < 3; k++ ) {
        //std::cout<<"  x="<<x<<" -> "<<pos[0]<<"    y="<<y<<" -> "<<pos[1]<<std::endl;
        MIN_MAX( xmin, xmax, pos[0] );
        MIN_MAX( ymin, ymax, pos[1] );
        pos += 2;
      }
    }
  }

//#ifndef NDEBUG
  std::cout<<"vips_lensfun_gen(): xmin="<<xmin<<" ymin="<<ymin<<" xmax="<<xmax<<" ymax="<<ymax<<std::endl;
//#endif

  s.left = xmin - window_offset - 5;
  s.top = ymin - window_offset - 5;
  s.width = xmax-xmin+window_size+11-1;
  s.height = ymax-ymin+window_size+11-1;

  VipsRect rimg = { 0, 0, ir->im->Xsize, ir->im->Ysize };
  vips_rect_intersectrect( &rimg, &s, &s );

  r_in.left = s.left + window_offset;
  r_in.top = s.top + window_offset;
  r_in.width = s.width - window_size + 1;
  r_in.height = s.height - window_size + 1;

  /**/
//#ifndef NDEBUG
  std::cout<<"vips_lensfun_gen(): "<<std::endl;
  std::cout<<"  input region:  top="<<s.top
	   <<" left="<<s.left
	   <<" width="<<s.width
	   <<" height="<<s.height<<std::endl
	   <<"  output region: top="<<oreg->valid.top
	   <<" left="<<oreg->valid.left
	   <<" width="<<oreg->valid.width
	   <<" height="<<oreg->valid.height<<std::endl;
//#endif
  /**/
  /* Prepare the input images
   */
  if(ir) {
//#ifndef NDEBUG
    std::cout<<"  preparing region ir:  top="<<s.top
        <<" left="<<s.left
        <<" width="<<s.width
        <<" height="<<s.height<<std::endl;
//#endif
    /**/
    if( vips_region_prepare( ir, &s ) )
      return( -1 );
  }

  /* Do the actual processing
   */
#ifndef NDEBUG
  std::cout<<"lensfun->in->Bands="<<lensfun->in->Bands<<std::endl;
#endif
  VipsInterpolateMethod interp_method =
      vips_interpolate_get_method ( lensfun->interpolate );
  int line_size = r->width * lensfun->in->Bands;
  pos = buf;
  for( y = 0; y < r->height; y++ ) {
    T *q = (T *)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
    for( x = 0; x < line_size; x+=lensfun->in->Bands ) {
      for( xx = 0; xx < lensfun->in->Bands; xx++ ) {
        int srcx = pos[0];
        int srcy = pos[1];
        if( vips_rect_includespoint(&r_in, srcx, srcy) ) {
          interp_method( lensfun->interpolate, &(q[x]), ir, pos[0], pos[1] );
          //T *p = (T *)VIPS_REGION_ADDR( ir, srcx, srcy );
          //q[x+xx] = p[xx];
        } else {
          q[x+xx] = PF::FormatInfo<T>::MIN;
        }
        pos += 2;
        //std::cout<<"x="<<x<<"  p["<<x<<"]="<<(uint32_t)p[x]<<"  pout["<<x<<"]="<<(uint32_t)q[x]<<std::endl;
      }
    }
  }
  /**/
#ifndef NDEBUG
  std::cout<<"vips_lensfun_gen(): "<<std::endl
	   <<"  bands = "<<oreg->im->Bands<<std::endl
	   <<"  fmt = "<<oreg->im->BandFmt<<std::endl
	   <<"  colorspace = "<<oreg->im->Type<<std::endl;
#endif
  /**/
  delete[] buf;

  return( 0 );
}


static int
vips_lensfun_gen( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion *ir = (VipsRegion *) seq;
  if( !ir ) return 1;

  int result = 0;

  switch( ir->im->BandFmt ) {
  case VIPS_FORMAT_UCHAR:
    result = vips_lensfun_gen_template<unsigned char>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_USHORT:
    result = vips_lensfun_gen_template<unsigned short int>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_FLOAT:
    result = vips_lensfun_gen_template<float>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_DOUBLE:
    result = vips_lensfun_gen_template<double>( oreg, seq, a, b, stop );
    break;
  }

  return result;
}


static int
vips_lensfun_build( VipsObject *object )
{
  VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
  VipsOperation *operation = VIPS_OPERATION( object );
  VipsLensFun *lensfun = (VipsLensFun *) object;
  int i;

  int window_size = vips_interpolate_get_window_size( lensfun->interpolate );
  int window_offset =
    vips_interpolate_get_window_offset( lensfun->interpolate );
  VipsDemandStyle hint;

  g_print("vips_lensfun_build() called. in=%p\n", lensfun->in);

  if( VIPS_OBJECT_CLASS( vips_lensfun_parent_class )->build( object ) )
    return( -1 );

  //lensfun->rand = 0;/*random();*/

  /* Normally SMALLTILE ...
   */
  hint = VIPS_DEMAND_STYLE_SMALLTILE;

  // Count total number of input images
  if( vips_image_pio_input( lensfun->in ) ||
      vips_check_coding_known( klass->nickname, lensfun->in ) )
    return( -1 );

#ifdef PF_HAS_LENSFUN
  PF::LensFunPar* lfpar = dynamic_cast<PF::LensFunPar*>( lensfun->processor->get_par() );

  const lfCamera** cameras = lensfun->ldb->FindCameras( lfpar->camera_maker().c_str(),
      lfpar->camera_model().c_str() );
  if( !cameras ) {
    g_print ("Cannot find the camera `%s %s' in database\n",
        lfpar->camera_maker().c_str(), lfpar->camera_model().c_str());
    return 1;
  }
  g_print("Camera `%s %s' found in database\n",
      lfpar->camera_maker().c_str(), lfpar->camera_model().c_str());
  const lfCamera *camera = cameras[0];
  lf_free (cameras);

  const lfLens **lenses = lensfun->ldb->FindLenses (camera, NULL, lfpar->lens().c_str());
  if (!lenses) {
	  g_print ("Cannot find the lens `%s' in database\n", lfpar->lens().c_str());
	  return 1;
  }
  const lfLens *lens = lenses[0];
  lf_free (lenses);

  g_print ("Lens `%s' found in database\n", lfpar->lens().c_str());

  lensfun->modifier = lfModifier::Create( lens, lens->CropFactor,
      lensfun->in->Xsize, lensfun->in->Ysize );
  int modflags = lensfun->modifier->Initialize(
      lens, LF_PF_U8, lfpar->get_focal_length(),
      lfpar->get_aperture(), lfpar->get_distance(), 1.0, lens->Type,
      LF_MODIFY_ALL, false );

  g_print("modflags: %d", modflags);

  if (modflags & LF_MODIFY_TCA)
    g_print ("[tca]");
  if (modflags & LF_MODIFY_VIGNETTING)
    g_print ("[vign]");
  if (modflags & LF_MODIFY_DISTORTION)
    g_print ("[dist]");
  if (modflags & LF_MODIFY_GEOMETRY)
    g_print ("[geom]");
  g_print (" ...\n");
#endif

  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( lensfun, "out", vips_image_new(), NULL );

  /* Set demand hints. 
  */
  std::cout<<"vips_lensfun_build(): lensfun->in="<<lensfun->in<<std::endl;
  VipsImage* invec[2] = {lensfun->in, NULL};
  if( vips_image_pipelinev( lensfun->out,
				 hint, lensfun->in, NULL ) )
    return( -1 );

  PF::OpParBase* par = lensfun->processor->get_par();

//#ifndef NDEBUG
  std::cout<<"vips_lensfun_build(): output format = "<<par->get_format()<<std::endl;
//#endif
/*
  vips_image_init_fields( lensfun->out,
      lensfun->in->Xsize,  lensfun->in->Ysize,
      lensfun->in->Bands, lensfun->in->BandFmt,
      lensfun->in->Coding,
      lensfun->in->Type,
			  1.0, 1.0);
*/
  vips_image_init_fields( lensfun->out,
        par->get_xsize(), par->get_ysize(),
        par->get_nbands(), par->get_format(),
        par->get_coding(),
        par->get_interpretation(),
        1.0, 1.0);
  if( vips_image_generate( lensfun->out,
      vips_start_one, vips_lensfun_gen, vips_stop_one,
      lensfun->in, lensfun ) )
    return( -1 );

  g_print("vips_lensfun_build() finished!!!\n");
  return( 0 );
}


static void
vips_lensfun_class_init( VipsLensFunClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( klass );
  VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( klass );

  gobject_class->set_property = vips_object_set_property;
  gobject_class->get_property = vips_object_get_property;

  vobject_class->nickname = "lensfun";
  vobject_class->description = _( "Optical corrections" );
  vobject_class->build = vips_lensfun_build;

  operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED/*+VIPS_OPERATION_NOCACHE*/;

  int argid = 0;

  VIPS_ARG_IMAGE( klass, "in", argid,
      _( "Input" ),
      _( "Input image" ),
      VIPS_ARGUMENT_REQUIRED_INPUT,
      G_STRUCT_OFFSET( VipsLensFun, in ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "out", argid, 
		  _( "Output" ), 
		  _( "Output image" ),
		  VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		  G_STRUCT_OFFSET( VipsLensFun, out ) );
  argid += 1;

  VIPS_ARG_POINTER( klass, "processor", argid, 
		    _( "Processor" ),
		    _( "Image processing object" ),
		    VIPS_ARGUMENT_REQUIRED_INPUT,
		    G_STRUCT_OFFSET( VipsLensFun, processor ) );
  argid += 1;

  VIPS_ARG_INTERPOLATE( klass, "interpolate", argid,
    _( "Interpolate" ),
    _( "Interpolate pixels with this" ),
    VIPS_ARGUMENT_REQUIRED_INPUT,
    G_STRUCT_OFFSET( VipsLensFun, interpolate ) );
}

static void
vips_lensfun_init( VipsLensFun *lensfun )
{
#ifdef PF_HAS_LENSFUN
  lensfun->ldb = lf_db_new();
  lensfun->ldb->Load ();
#endif
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
vips_lensfun( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, VipsInterpolate* interpolate, ... )
{
  va_list ap;
  int result;

  va_start( ap, interpolate );
  result = vips_call_split( "lensfun", ap, in, out, proc, interpolate );
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
