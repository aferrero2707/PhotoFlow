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
#include "../operations/clone_stamp.hh"

#define PF_MAX_INPUT_IMAGES 10

static GObject* object_in;

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
  VipsImage* in[PF_MAX_INPUT_IMAGES];
  int ninput;

  /* The vector of input images.
   */
  VipsImage* out;

  /* The index at which input images start
   * in[0] always corresponds to the lower clone_stamp of the blending step,
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
  
  /* The preferred output style for this clone_stamp
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
  VipsRegion **ir = (VipsRegion **) seq;
  VipsCloneStamp *clone_stamp = (VipsCloneStamp *) b;
  

  /* Do the actual processing
   */

  /* Output area we are building.
   */
  VipsRect *r = &oreg->valid;
  VipsRect in_area, out_area;
  int i;
  int ninput = clone_stamp->ninput;
  VipsRect point_area;
  VipsRect point_clip;
  int point_clip_right, point_clip_bottom;
  int x, x0, y, y0, ch, row1, row2;
  int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 

  VipsRect image_area = {0, 0, ir[0]->im->Xsize, ir[0]->im->Ysize};

  if( !clone_stamp->processor ) return 1;
  if( !clone_stamp->processor->get_par() ) return 1;

  PF::CloneStampPar* par = dynamic_cast<PF::CloneStampPar*>( clone_stamp->processor->get_par() );
  if( !par ) return 1;

  T *p, *pout;
  
  if( vips_region_prepare( ir[0], r ) )
    return( -1 );

  for( y = 0; y < r->height; y++ ) {
    p = (T*)VIPS_REGION_ADDR( ir[0], r->left, r->top + y );
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

  /*
#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_gen(): "<<std::endl;
  if( clone_stamp->processor->get_par()->get_config_ui() )
    std::cout<<"  name: "<<clone_stamp->processor->get_par()->get_config_ui()->get_layer()->get_name()<<std::endl;
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

  PF::Property< std::list<PF::StrokesGroup> >& groups = par->get_strokes();
  std::list<PF::StrokesGroup>::iterator gi;
  std::list< PF::Stroke<PF::Stamp> >::iterator si;
  std::list< std::pair<unsigned int, unsigned int> >::iterator pi;

  for( gi = groups.get().begin(); gi != groups.get().end(); ++gi ) {

    PF::StrokesGroup& group = *gi;
    //std::cout<<"vips_clone_stamp_gen(): starting group"<<std::endl;
    //std::cout<<"  n. of strokes: "<<group.get_strokes().size()<<std::endl;

    bool prepared = false;
    int delta_row = group.get_delta_row();
    int delta_col = group.get_delta_col();

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

    for( si = group.get_strokes().begin(); si != group.get_strokes().end(); ++si ) {
 
      std::list< std::pair<unsigned int, unsigned int> >& points = si->get_points();
      //std::cout<<"vips_clone_stamp_gen(): starting stroke"<<std::endl;
      //std::cout<<"  n. of points: "<<points.size()<<std::endl;
    
      PF::Stamp& pen = si->get_pen();
      int pen_size = pen.get_size()/par->get_scale_factor();
      int pen_size2 = pen_size*pen_size;

      //std::cout<<"  pen size="<<pen.get_size()<<"  opacity="<<pen.get_opacity()
      //         <<"  smoothness="<<pen.get_smoothness()<<std::endl;

      point_area.width = point_area.height = pen_size*2 + 1;
      for( pi = points.begin(); pi != points.end(); ++pi ) {
        point_area.left = pi->first/par->get_scale_factor() - pen_size;
        point_area.top = pi->second/par->get_scale_factor() - pen_size;
        // The area covered by the current point is intersected with the output area.
        // If the point is completely outside of the output area, it is skipped.
        vips_rect_intersectrect( &out_area, &point_area, &point_clip );
        if( point_clip.width<1 || point_clip.height<1 ) continue;

        point_clip_right = point_clip.left + point_clip.width - 1;
        point_clip_bottom = point_clip.top + point_clip.height - 1;
        
        // We have at least one point to process, so we need to process the inout pixels
        if( !prepared ) {
          if( vips_region_prepare( ir[0], &in_area ) )
            return( -1 );
          prepared = true;
        }

        x0 = pi->first/par->get_scale_factor();
        y0 = pi->second/par->get_scale_factor();
        //std::cout<<"x0="<<x0<<"  y0="<<y0<<std::endl;
        for( y = 0; y <= pen_size; y++ ) {
          row1 = y0 - y;
          row2 = y0 + y;
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
            p =    (T*)VIPS_REGION_ADDR( ir[0], startcol-delta_col, row1-delta_row ); 
            pout = (T*)VIPS_REGION_ADDR( oreg, startcol, row1 ); 
            for( x = 0; x < colspan; x += oreg->im->Bands ) {
              for( ch = 0; ch < oreg->im->Bands; ch++ ) {
                pout[x+ch] = p[x+ch];
              }
              //std::cout<<"x="<<x<<"+"<<point_clip.left<<"="<<x+point_clip.left<<std::endl;
            }
          }
          if( (row2 != row1) && (row2 >= point_clip.top) && (row2 <= point_clip_bottom) ) {
            p =    (T*)VIPS_REGION_ADDR( ir[0], startcol-delta_col, row2-delta_row ); 
            pout = (T*)VIPS_REGION_ADDR( oreg, startcol, row2 ); 
            for( x = 0; x < colspan; x += oreg->im->Bands ) {
              for( ch = 0; ch < oreg->im->Bands; ch++ ) {
                pout[x+ch] = p[x+ch];
              }
            }
          }
        }
      }
    }
  }

  return( 0 );
}


static int
vips_clone_stamp_gen( VipsRegion *oreg, void *seq, void *a, void *b, gboolean *stop )
{
  VipsRegion **ir = (VipsRegion **) seq;
  if( !ir || !ir[0] ) return 1;

  int result = 0;

  switch( ir[0]->im->BandFmt ) {
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

  //clone_stamp->rand = 0;/*random();*/

  // Count total number of input images
  int nimg = clone_stamp->ninput;
  if(clone_stamp->imap) {
    nimg++;
  }
  if(clone_stamp->omap) {
    nimg++;
  }

  clone_stamp->in_all = (VipsImage**)im_malloc( clone_stamp->out, sizeof(VipsImage*)*(nimg+1) );
  if( !clone_stamp->in_all ) return( -1 );

  for( i = 0; i < clone_stamp->ninput; i++ ) {
    clone_stamp->in_all[i] = clone_stamp->in[i];
  }
  if( clone_stamp->imap ) { clone_stamp->in_all[i] = clone_stamp->imap; i+= 1; }
  if( clone_stamp->omap ) { clone_stamp->in_all[i] = clone_stamp->omap; i+= 1; }
  clone_stamp->in_all[nimg] = NULL;

  //if( clone_stamp->processor->identity() == 1 ) 
  //  return( vips_image_write( clone_stamp->in, conversion->out ) );

  for( i = 0; i < nimg; i++ ) {
    if( vips_image_pio_input( clone_stamp->in_all[i] ) || 
	vips_check_coding_known( klass->nickname, clone_stamp->in_all[i] ) )  
      return( -1 );
  }

  /* Get ready to write to @out. @out must be set via g_object_set() so
   * that vips can see the assignment. It'll complain that @out hasn't
   * been set otherwise.
   */
  g_object_set( clone_stamp, "out", vips_image_new(), NULL ); 

  /* Set demand hints. 
  */
  if( vips_image_pipeline_array( clone_stamp->out, 
				 clone_stamp->demand_hint, 
				 clone_stamp->in_all ) )
    return( -1 );

  PF::OpParBase* par = clone_stamp->processor->get_par();

#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_build(): clone_stamp->ninput="<<clone_stamp->ninput<<std::endl;
#endif

#ifndef NDEBUG
  std::cout<<"vips_clone_stamp_build(): output format = "<<par->get_format()<<std::endl;
#endif
  vips_image_init_fields( clone_stamp->out,
			  par->get_xsize(), par->get_ysize(), 
			  par->get_nbands(), par->get_format(),
			  par->get_coding(),
			  par->get_interpretation(),
			  1.0, 1.0);
  if(nimg > 0) {
    if( vips_image_generate( clone_stamp->out,
			     vips_start_many, vips_clone_stamp_gen, vips_stop_many, 
			     clone_stamp->in_all, clone_stamp ) )
      return( -1 );
  } else {
    if( vips_image_generate( clone_stamp->out, 
			     NULL, vips_clone_stamp_gen, NULL, NULL, clone_stamp ) )
      return( -1 );
  }

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

  VIPS_ARG_INT( klass, "ninput", argid, 
		_( "NInput" ), 
		_( "Number of input images" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsCloneStamp, ninput ),
		0, PF_MAX_INPUT_IMAGES, 0);
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

  VIPS_ARG_IMAGE( klass, "intensity_map", argid, 
		  _( "IntensityMap" ), 
		  _( "Intensity map" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT,
		  G_STRUCT_OFFSET( VipsCloneStamp, imap ) );
  argid += 1;

  VIPS_ARG_IMAGE( klass, "opacity_map", argid, 
		  _( "OpacityMap" ), 
		  _( "Opacity map" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT,
		  G_STRUCT_OFFSET( VipsCloneStamp, omap ) );
  argid += 1;

  VIPS_ARG_ENUM( klass, "demand_hint", argid, 
		 _( "DemandHint" ), 
		 _( "Preferred demand style" ),
		 VIPS_ARGUMENT_REQUIRED_INPUT,
		 G_STRUCT_OFFSET( VipsCloneStamp, demand_hint ),
		 VIPS_TYPE_DEMAND_STYLE, VIPS_DEMAND_STYLE_THINSTRIP );
  argid += 1;

  VIPS_ARG_INT( klass, "width", argid, 
		  _( "Width" ), 
		  _( "Image width" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsCloneStamp, width ),
		0, 10000000, 0);
  argid += 1;

  VIPS_ARG_INT( klass, "height", argid, 
		  _( "Height" ), 
		  _( "Image height" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsCloneStamp, height ),
		0, 10000000, 0);
  argid += 1;

  VIPS_ARG_INT( klass, "nbands", argid, 
		  _( "NBands" ), 
		  _( "Number of channels" ),
		  VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsCloneStamp, nbands ),
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
		    G_STRUCT_OFFSET( VipsCloneStamp, in )+sizeof(VipsImage*)*imgid );
    argid += 1;
  }
}

static void
vips_clone_stamp_init( VipsCloneStamp *clone_stamp )
{
  clone_stamp->in[0] = NULL;
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
vips_clone_stamp( int n, VipsImage **out, PF::ProcessorBase* proc, 
	    VipsImage* imap, VipsImage* omap, VipsDemandStyle demand_hint,
	    int width, int height, int nbands, ...)
{
  va_list ap;
  int result;

  va_start( ap, nbands );
  result = vips_call_split( "clone_stamp", ap, n, out, proc, imap, omap, demand_hint, width, height, nbands );
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
