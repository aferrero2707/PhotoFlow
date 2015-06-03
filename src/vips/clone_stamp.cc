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

#include "../base/array2d.hh"
#include "../base/processor.hh"
#include "../base/layer.hh"
#include "../operations/clone_stamp.hh"

#define PF_MAX_INPUT_IMAGES 10

static GObject* object_in;

//#undef NDEBUG

/**/
#define VIPS_TYPE_CLONE_STAMP (vips_clone_stamp_get_type())
#define VIPS_CLONE_STAMP( obj ) \
	(G_TYPE_CHECK_INSTANCE_CAST( (obj), \
		VIPS_TYPE_CLONE_STAMP, VipsCloneStamp ))
#define VIPS_CLONE_STAMP_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_CAST( (klass), \
		VIPS_TYPE_CLONE_STAMP, VipsCloneStampClass))
#define VIPS_IS_CLONE_STAMP( obj ) \
	(G_TYPE_CHECK_INSTANCE_TYPE( (obj), VIPS_TYPE_CLONE_STAMP ))
#define VIPS_IS_CLONE_STAMP_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_TYPE( (klass), VIPS_TYPE_CLONE_STAMP ))
#define VIPS_CLONE_STAMP_GET_CLASS( obj ) \
	(G_TYPE_INSTANCE_GET_CLASS( (obj), \
		VIPS_TYPE_CLONE_STAMP, VipsCloneStampClass ))
/**/
typedef struct _VipsCloneStamp {
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

  int group_num;

  int stroke_num;
} VipsCloneStamp;

/*
typedef struct _VipsCloneStampClass {
	VipsOperationClass parent_class;
} VipsCloneStampClass;
*/
typedef VipsOperationClass VipsCloneStampClass;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

G_DEFINE_TYPE( VipsCloneStamp, vips_clone_stamp, VIPS_TYPE_OPERATION );

#ifdef __cplusplus
}
#endif /*__cplusplus*/



template<class T>
static int
vips_clone_stamp_gen_template( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion *ir = (VipsRegion *) seq;
  VipsCloneStamp *clone_stamp = (VipsCloneStamp *) b;
  

  /* Do the actual processing
   */

  /* Output area we are building.
   */
  VipsRect *r = &oreg->valid;
  VipsRect in_area, out_area;
  int i;
  VipsRect point_area;
  VipsRect point_clip;
  int point_clip_right, point_clip_bottom;
  int x, x0, y, y0, ch, row1, row2, col, mx, my1, my2;
  int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

  VipsRect image_area = {0, 0, ir->im->Xsize, ir->im->Ysize};

  if( !clone_stamp->processor ) return 1;
  if( !clone_stamp->processor->get_par() ) return 1;

  PF::CloneStampPar* par = dynamic_cast<PF::CloneStampPar*>( clone_stamp->processor->get_par() );
  if( !par ) return 1;

  T *p, *pout, *pbgd;
  
  if( !ir )
    return( -1 );
  if( vips_region_prepare( ir, r ) )
    return( -1 );

  for( y = 0; y < r->height; y++ ) {
    p = (T*)VIPS_REGION_ADDR( ir, r->left, r->top + y );
    pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
    memcpy( pout, p, sizeof(T)*line_size );
    /*
    for( x = 0; x < line_size; x += oreg->im->Bands ) {
      for( ch = 0; ch < oreg->im->Bands; ch++ ) {
        pout[x+ch] = p[x+ch];
      }
    }
    */
  }

  /**/
#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_gen(): "<<std::endl;
  if( clone_stamp->processor->get_par()->get_config_ui() )
    std::cout<<"  name: "<<clone_stamp->processor->get_par()->get_config_ui()->get_layer()->get_name()<<std::endl;
  std::cout<<"  output region: top="<<oreg->valid.top
	   <<" left="<<oreg->valid.left
	   <<" width="<<oreg->valid.width
	   <<" height="<<oreg->valid.height<<std::endl;
#endif
  /**/

  std::vector<PF::StrokesGroup>& groups = par->get_strokes();
  std::list< std::pair<unsigned int, unsigned int> >::iterator pi;

#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_gen(): n. of groups: "<<groups.size()<<std::endl;
#endif
  if( groups.size() <= clone_stamp->group_num )
      return 1;
  PF::StrokesGroup& group = groups[clone_stamp->group_num];

  bool prepared = false;
  int delta_row = group.get_delta_row()/par->get_scale_factor();
  int delta_col = group.get_delta_col()/par->get_scale_factor();
#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_gen(): starting group #"<<clone_stamp->group_num<<std::endl;
  std::cout<<"  n. of strokes: "<<group.get_strokes().size()<<std::endl;
  std::cout<<"  Drow: "<<delta_row<<"  Dcol: "<<delta_col<<std::endl;
#endif

  // Input area = oreg area translated by (delta_row, delta_col)
  // and intersected with the image area
  in_area.width = r->width;
  in_area.height = r->height;
  in_area.top = r->top - delta_row;
  in_area.left = r->left - delta_col;
  vips_rect_intersectrect( &in_area, &image_area, &in_area );

  // Output area: input area translated by (delta_row, delta_col)
  out_area.left = in_area.left + delta_col;
  out_area.top = in_area.top + delta_row;
  out_area.width = in_area.width;
  out_area.height = in_area.height;

  PF::Array2D<float> opacity_max;
  opacity_max.Init( out_area.width, out_area.height, out_area.top, out_area.left );
  for( int ic = 0; ic < out_area.width; ic++ ) {
    for( int ir = 0; ir < out_area.height; ir++ ) {
      opacity_max.GetLocal(ir, ic) = 0;
    }
  }

  // We keep a copy of the original pixels tan needs to be blended with the cloned ones
  PF::Array2D<T> bgd;
  bgd.Init( out_area.width*oreg->im->Bands, out_area.height,
      out_area.top, out_area.left*oreg->im->Bands );
  for( y = 0; y < out_area.height; y++ ) {
    pout = (T*)VIPS_REGION_ADDR( oreg, out_area.left, out_area.top + y );
    for( x = 0; x < out_area.width*oreg->im->Bands; x++ ) {
      bgd.Get( out_area.top+y, out_area.left*oreg->im->Bands+x ) = pout[x];
    }
  }

  PF::Stroke<PF::Stamp>& stroke = group.get_strokes()[clone_stamp->stroke_num];

  std::list< std::pair<unsigned int, unsigned int> >& points = stroke.get_points();
#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_gen(): starting stroke"<<std::endl;
  std::cout<<"  n. of points: "<<points.size()<<std::endl;
#endif

  PF::Stamp& pen = stroke.get_pen();
  int pen_size = pen.get_size()/par->get_scale_factor();
  int pen_size2 = pen_size*pen_size;

  //std::cout<<"  pen size="<<pen_size<<"  opacity="<<pen.get_opacity()
  //         <<"  smoothness="<<pen.get_smoothness()<<std::endl;

  PF::StampMask* resized_mask = NULL;
  PF::StampMask* mask = &(pen.get_mask());
  if( pen_size != pen.get_size() ) {
    resized_mask = new PF::StampMask;
    resized_mask->init( pen_size*2+1, pen.get_opacity(), pen.get_smoothness() );
    mask = resized_mask;
  }

  point_area.width = point_area.height = pen_size*2 + 1;
  for( pi = points.begin(); pi != points.end(); ++pi ) {
    x0 = pi->first/par->get_scale_factor();
    y0 = pi->second/par->get_scale_factor();
#ifndef NDEBUG
    std::cout<<"  point @ x0="<<x0<<"  y0="<<y0<<std::endl;
#endif

    point_area.left = x0 - pen_size;
    point_area.top = y0 - pen_size;
    // The area covered by the current point is intersected with the output area.
    // If the point is completely outside of the output area, it is skipped.
    vips_rect_intersectrect( &out_area, &point_area, &point_clip );
    if( point_clip.width<1 || point_clip.height<1 ) continue;

    point_clip_right = point_clip.left + point_clip.width - 1;
    point_clip_bottom = point_clip.top + point_clip.height - 1;

    // We have at least one point to process, so we need to process the input pixels
    if( !prepared ) {
#ifndef NDEBUG
      std::cout<<"  preparing region ir:  top="<<in_area.top
          <<" left="<<in_area.left
          <<" width="<<in_area.width
          <<" height="<<in_area.height<<std::endl;
#endif
      if( vips_region_prepare( ir, &in_area ) )
        return( -1 );
      prepared = true;
    }

    for( y = 0; y <= pen_size; y++ ) {
      row1 = y0 - y;
      row2 = y0 + y;
      my1 = pen_size - y;
      my2 = pen_size + y;
      //int L = pen.get_size() - y;
      int D = (int)sqrt( pen_size2 - y*y );
      int startcol = x0 - D;
      if( startcol < point_clip.left )
        startcol = point_clip.left;
      int endcol = x0 + D;
      if( endcol >= point_clip_right )
        endcol = point_clip_right;
      int colspan = (endcol + 1 - startcol)*oreg->im->Bands;

      //endcol = x0;

      //std::cout<<"row1="<<row1<<"  row2="<<row2<<"  startcol="<<startcol<<"  endcol="<<endcol<<"  colspan="<<colspan<<std::endl;
      //std::cout<<"point_clip.left="<<point_clip.left<<"  point_clip.top="<<point_clip.top
      //         <<"  point_clip.width="<<point_clip.width<<"  point_clip.height="<<point_clip.height<<std::endl;

      /**/
      if( (row1 >= point_clip.top) && (row1 <= point_clip_bottom) ) {
        p =    (T*)VIPS_REGION_ADDR( ir, startcol-delta_col, row1-delta_row );
        pbgd = &( bgd.Get(row1, startcol*oreg->im->Bands) );
        pout = (T*)VIPS_REGION_ADDR( oreg, startcol, row1 );
        mx = startcol - x0 + pen_size;
        for( x = 0, col = startcol; x < colspan; x += oreg->im->Bands, col++, mx++ ) {
          float mval = mask->get( mx, my1 );
          //std::cout<<"    opacity_max.Get("<<row1<<", "<<col<<")="<<opacity_max.Get(row1, col)<<"    mval="<<mval<<std::endl;
          if( mval < opacity_max.Get(row1, col) )
            continue;
          for( ch = 0; ch < oreg->im->Bands; ch++ ) {
            float val = mval*p[x+ch] + (1.0f-mval)*pbgd[x+ch];
            pout[x+ch] = static_cast<T>(val);
          }
          opacity_max.Get(row1, col) = mval;
          //std::cout<<"x="<<x<<"+"<<point_clip.left<<"="<<x+point_clip.left<<std::endl;
        }
      }
      if( (row2 != row1) && (row2 >= point_clip.top) && (row2 <= point_clip_bottom) ) {
        p =    (T*)VIPS_REGION_ADDR( ir, startcol-delta_col, row2-delta_row );
        pbgd = &( bgd.Get(row2, startcol*oreg->im->Bands) );
        pout = (T*)VIPS_REGION_ADDR( oreg, startcol, row2 );
        mx = startcol - x0 + pen_size;
        for( x = 0, col = startcol; x < colspan; x += oreg->im->Bands, col++, mx++ ) {
          float mval = mask->get( mx, my2 );
          //std::cout<<"    opacity_max.Get("<<row2<<", "<<col<<")="<<opacity_max.Get(row2, col)<<"    mval="<<mval<<std::endl;
          if( mval < opacity_max.Get(row2, col) )
            continue;
          for( ch = 0; ch < oreg->im->Bands; ch++ ) {
            float val = mval*p[x+ch] + (1.0f-mval)*pbgd[x+ch];
            pout[x+ch] = static_cast<T>(val);
            //pout[x+ch] = p[x+ch];
          }
          opacity_max.Get(row2, col) = mval;
        }
      }
    }
  }

  if( resized_mask ) delete resized_mask;

  return( 0 );
}


static int
vips_clone_stamp_gen( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion *ir = (VipsRegion *) seq;
  //g_print("vips_clone_stamp_gen() called, ir=%p\n", ir);
  if( !ir ) return 1;

  int result = 0;

  switch( ir->im->BandFmt ) {
  case VIPS_FORMAT_UCHAR:
    result = vips_clone_stamp_gen_template<unsigned char>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_USHORT:
    result = vips_clone_stamp_gen_template<unsigned short int>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_FLOAT:
    result = vips_clone_stamp_gen_template<float>( oreg, seq, a, b, stop );
    break;
  case VIPS_FORMAT_DOUBLE:
    result = vips_clone_stamp_gen_template<double>( oreg, seq, a, b, stop );
    break;
  }

  return result;
}

  
static int
vips_clone_stamp_build( VipsObject *object )
{
  VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
  VipsOperation *operation = VIPS_OPERATION( object );
  VipsCloneStamp *clone_stamp = (VipsCloneStamp *) object;
  int i;

  if( VIPS_OBJECT_CLASS( vips_clone_stamp_parent_class )->build( object ) )
    return( -1 );

  if( !clone_stamp->in )
    return( -1 );

  //clone_stamp->rand = 0;/*random();*/

  //if( clone_stamp->processor->identity() == 1 ) 
  //  return( vips_image_write( clone_stamp->in, conversion->out ) );

  if( vips_image_pio_input( clone_stamp->in ) ||
	vips_check_coding_known( klass->nickname, clone_stamp->in ) )
      return( -1 );

  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( clone_stamp, "out", vips_image_new(), NULL ); 

  /* Set demand hints. 
  */
  if( vips_image_pipelinev( clone_stamp->out,
      VIPS_DEMAND_STYLE_ANY,
      clone_stamp->in, NULL ) )
    return( -1 );

  PF::OpParBase* par = clone_stamp->processor->get_par();

#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_build(): output format = "<<par->get_format()<<std::endl;
#endif
  vips_image_init_fields( clone_stamp->out,
			  clone_stamp->in->Xsize,  clone_stamp->in->Ysize,
			  clone_stamp->in->Bands, clone_stamp->in->BandFmt,
			  clone_stamp->in->Coding,
			  clone_stamp->in->Type,
			  1.0, 1.0);
  if( vips_image_generate( clone_stamp->out,
      vips_start_one, vips_clone_stamp_gen, vips_stop_one,
      clone_stamp->in, clone_stamp ) )
    return( -1 );

  return( 0 );
}


static void
vips_clone_stamp_class_init( VipsCloneStampClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( klass );
  VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( klass );

  gobject_class->set_property = vips_object_set_property;
  gobject_class->get_property = vips_object_get_property;

  vobject_class->nickname = "clone_stamp";
  vobject_class->description = _( "Photoflow clone_stamp" );
  vobject_class->build = vips_clone_stamp_build;

  operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED/*+VIPS_OPERATION_NOCACHE*/;

  int argid = 0;

  VIPS_ARG_IMAGE( klass, "in", argid,
      _( "Input" ),
      _( "Input image" ),
      VIPS_ARGUMENT_REQUIRED_INPUT,
      G_STRUCT_OFFSET( VipsCloneStamp, in ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "out", argid, 
		  _( "Output" ), 
		  _( "Output image" ),
		  VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		  G_STRUCT_OFFSET( VipsCloneStamp, out ) );
  argid += 1;

  VIPS_ARG_POINTER( klass, "processor", argid, 
		    _( "Processor" ),
		    _( "Image processing object" ),
		    VIPS_ARGUMENT_REQUIRED_INPUT,
		    G_STRUCT_OFFSET( VipsCloneStamp, processor ) );
  argid += 1;

  VIPS_ARG_INT( klass, "group_num", argid,
      _( "StrokeNum" ),
      _( "Stroke number" ),
      VIPS_ARGUMENT_REQUIRED_INPUT,
    G_STRUCT_OFFSET( VipsCloneStamp, group_num ),
    -1, 1000, -1);
  argid += 1;

  VIPS_ARG_INT( klass, "stroke_num", argid,
      _( "StrokeNum" ),
      _( "Stroke number" ),
      VIPS_ARGUMENT_REQUIRED_INPUT,
    G_STRUCT_OFFSET( VipsCloneStamp, stroke_num ),
    -1, 1000, -1);
  argid += 1;
}

static void
vips_clone_stamp_init( VipsCloneStamp *clone_stamp )
{
  //clone_stamp->in = NULL;
  clone_stamp->group_num = -1;
  clone_stamp->stroke_num = -1;
}

/**
 * vips_clone_stamp:
 * @in: input image
 * @out: output image
 * @...: %NULL-terminated list of optional named arguments
 *
 * Returns: 0 on success, -1 on error.
 */
int
vips_clone_stamp( VipsImage* in, VipsImage **out, PF::ProcessorBase* proc, int group_num, int stroke_num, ...)
{
  va_list ap;
  int result;

  va_start( ap, stroke_num );
  result = vips_call_split( "clone_stamp", ap, in, out, proc, group_num, stroke_num );
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

    if (vips_call("clone_stamp", NULL, &imap, 0, gradient, NULL, NULL, NULL))
      verror ();
    //g_object_unref( imap );

    area = vips_area_new_array_object( 1 );
    array = (VipsImage **) area->data;
    array[0] = in;
    g_object_ref( array[0] );
    if (vips_call("clone_stamp", area, &out, 0, bc, imap, NULL, NULL))
      verror ();
    vips_area_unref( area );
    g_object_unref( out );
    in = out;

    for(int i = 0; i < N; i++) {
      area = vips_area_new_array_object( 1 );
      array = (VipsImage **) area->data;
      array[0] = in;
      g_object_ref( array[0] );
      if (vips_call("clone_stamp", area, &out, 0, invert, NULL, NULL, NULL))
        verror ();
      vips_area_unref( area );
*/
