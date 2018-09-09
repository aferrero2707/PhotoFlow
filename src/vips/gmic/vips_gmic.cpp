/* Pass VIPS images through gmic
 *
 * AF, 6/10/14
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


#include <vips/vips.h>
#include <vips/dispatch.h>

#include <limits.h>

#include <iostream>
#include <fstream>

#include "../../base/photoflow.hh"

#ifdef _
#undef _
#endif

#define _(S) (S)
//#include "CImg.h"
#include "gmic/src/gmic.h"
//#include "gmic.h"


static char* custom_gmic_commands = 0;
static GMutex* gmic_mutex = 0;


using namespace cimg_library;


typedef struct _VipsGMicPoolElement
{
  gmic* instance;
  bool used;
} VipsGMicPoolElement;



class GmicPool
{
  std::vector<VipsGMicPoolElement*> pool;
  GMutex* mutex;
public:
  GmicPool()
  {
    mutex = vips_g_mutex_new();
  }

  gmic* get_gmic()
  {
    g_mutex_lock( mutex );
    VipsGMicPoolElement* result = NULL;
    for( unsigned int i = 0; i < pool.size(); i++ ) {
      if( pool[i]->used ) continue;
      result = pool[i];
    }
    if( !result ) {
      result = new VipsGMicPoolElement();
      result->instance = new gmic( 0, custom_gmic_commands, false, 0, 0 );
      pool.push_back( result );
    }
    result->used = true;
    g_mutex_unlock( mutex );
    return( result->instance );
  }

  void release_gmic( gmic* instance )
  {
    g_mutex_lock( mutex );
    for( unsigned int i = 0; i < pool.size(); i++ ) {
      if( pool[i]->instance != instance ) continue;
      pool[i]->used = false;
      break;
    }
    g_mutex_unlock( mutex );
  }
};

static GmicPool gmic_pool;


typedef struct _VipsGMic {
	VipsOperation parent_instance;

	VipsArrayImage *in;
	VipsImage *out;
	char *command;
	int padding;
	double x_scale;
	double y_scale;
} VipsGMic;

typedef VipsOperationClass VipsGMicClass;

#define VIPS_TYPE_GMIC (vips_gmic_get_type())
#define VIPS_GMIC( obj ) \
	(G_TYPE_CHECK_INSTANCE_CAST( (obj), VIPS_TYPE_GMIC, VipsGMic ))
#define VIPS_GMIC_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_CAST( (klass), VIPS_TYPE_GMIC, VipsGMicClass))
#define VIPS_IS_GMIC( obj ) \
	(G_TYPE_CHECK_INSTANCE_TYPE( (obj), VIPS_TYPE_GMIC ))
#define VIPS_IS_GMIC_CLASS( klass ) \
	(G_TYPE_CHECK_CLASS_TYPE( (klass), VIPS_TYPE_GMIC ))
#define VIPS_GMIC_GET_CLASS( obj ) \
	(G_TYPE_INSTANCE_GET_CLASS( (obj), VIPS_TYPE_GMIC, VipsGMicClass ))

extern "C" {
	G_DEFINE_TYPE( VipsGMic, vips_gmic, VIPS_TYPE_OPERATION );
}

static int 
vips_gmic_get_tile_border( VipsGMic *vipsgmic )
{
	return( vipsgmic->padding );
}


// Convert input data to floating point values in the [0..255] range
template <typename T> float vips_to_gmic(const T& val)
{
  return( static_cast<float>(val) );
}

template<> float vips_to_gmic(const float& val)
{
  return( val*255 );
}

template<> float vips_to_gmic(const unsigned short& val)
{
  return( static_cast<float>(val)/257 );
}


// Convert floating point data from gmic to output VIPS format
template <typename T>  void vips_from_gmic(const float& in, T& out)
{
  // clip values to [0..255]
  if( in >= 0 && in <= 255 )
    out = static_cast<T>(in);
  else if( in < 0 )
    out = 0;
  else out = 255;
}

template<> void vips_from_gmic(const float& val, float& out)
{
  // clip values to [0..255]
  out = val/255; return;
  if( val >= 0 && val <= 255 )
    out = val/255;
  else if( val < 0 )
    out = 0;
  else out = 1;
}

template<> void vips_from_gmic(const float& val, unsigned short& out)
{
  // clip values to [0..255]
  if( val >= 0 && val <= 255 )
    out = static_cast<unsigned short>(val*257);
  else if( val < 0 )
    out = 0;
  else out = 65535;
}


#define INDEX( IMG, X, Y, Z, C ) \
	((*(IMG))( (guint) (X), (guint) (Y), (guint) (Z), C ))

// copy part of a vips region into a cimg
template<typename T> static void
vips_to_gmic( VipsRegion *in, VipsRect *area, CImg<float> *img )
{
	VipsImage *im = in->im;

  //std::cout<<"vips_to_gmic():  im->Bands="<<im->Bands<<"  img->spectrum()="<<img->spectrum()<<std::endl;

	for( int y = 0; y < area->height; y++ ) {
		T *p = (T *) VIPS_REGION_ADDR( in, area->left, area->top + y );

		for( int x = 0; x < area->width; x++ ) {
			for( int c = 0; c < im->Bands; c++ )
				INDEX( img, x, y, 0, c ) = vips_to_gmic( p[c] );

			p += im->Bands;
		}
	}
}

// write a CImg to a vips region
// fill out->valid, img has pixels in img_rect
template<typename T> static void
vips_from_gmic( gmic_image<float> *img, VipsRect *img_rect, VipsRegion *out )
{
	VipsImage *im = out->im;
	VipsRect *valid = &out->valid;

  //std::cout<<"vips_from_gmic():  im->Bands="<<im->Bands<<"  img->spectrum()="<<img->spectrum()<<std::endl;

  g_assert( vips_rect_includesrect( img_rect, valid ) );

	int x_off = valid->left - img_rect->left;
	int y_off = valid->top - img_rect->top;

	for( int y = 0; y < valid->height; y++ ) {
		T *p = (T *)VIPS_REGION_ADDR( out, valid->left, valid->top + y );

		for( int x = 0; x < valid->width; x++ ) {
		  if( img->spectrum() < im->Bands ) {
		    for( int c = 0; c < im->Bands; c++ )
		      vips_from_gmic( INDEX( img, x + x_off, y + y_off, 0, 0 ), p[c] );
		  } else {
		    for( int c = 0; c < im->Bands; c++ )
		      vips_from_gmic( INDEX( img, x + x_off, y + y_off, 0, c ), p[c] );
		  }
			p += im->Bands;
		}
	}
}

/* One of these for each thread.
 */
struct VipsGMicSequence { 
	VipsRegion **ir;
	gmic *gmic_instance;
};

static int
vips_gmic_stop( void *vseq, void *a, void *b )
{
  VipsGMicSequence *seq = (VipsGMicSequence *) vseq;

  if( seq->ir ) {
    int i;

    for( i = 0; seq->ir[i]; i++ )
      g_object_unref( seq->ir[i] );
    VIPS_FREE( seq->ir );
  }

  //g_mutex_lock( gmic_mutex );
  //delete seq->gmic_instance;
  //g_mutex_unlock( gmic_mutex );
  gmic_pool.release_gmic( seq->gmic_instance );

  VIPS_FREE( seq );

  return( 0 );
}

static void *
vips_gmic_start( VipsImage *out, void *a, void *b )
{
	VipsImage **in = (VipsImage **) a;

	VipsGMicSequence *seq;
	int i, n;

	if( !(seq = VIPS_NEW( NULL, VipsGMicSequence )) )
		return( NULL ); 

  //printf("vips_gmic_start(): in[0]=%p\n",in[0]);

	/* Make a region for each input image. 
	 */
	for( n = 0; in[n]; n++ )
		;

	if( !(seq->ir = VIPS_ARRAY( NULL, n + 1, VipsRegion * )) ) {
		vips_gmic_stop( seq, NULL, NULL );
		return( NULL );
	}

	for( i = 0; i < n; i++ )
		if( !(seq->ir[i] = vips_region_new( in[i] )) ) {
			vips_gmic_stop( seq, NULL, NULL );
			return( NULL );
		}
	seq->ir[n] = NULL;

  /* Make a gmic for this thread.
   */
  //g_mutex_lock( gmic_mutex );
  //std::cout<<"Allocating G'MIC object"<<std::endl;
  //seq->gmic_instance = new gmic( 0, custom_gmic_commands, false, 0, 0 );
  //std::cout<<"G'MIC object created: seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<std::endl;
  //g_mutex_unlock( gmic_mutex );
  seq->gmic_instance = gmic_pool.get_gmic();

	return( (void *) seq );
}

template<typename T> static int
vips_gmic_gen_template( VipsRegion *oreg, 
	VipsGMicSequence *seq, void *a, void *b, gboolean *stop )
{
	VipsGMic *vipsgmic = (VipsGMic *) b;
	int ninput = VIPS_AREA( vipsgmic->in )->n;
	const int tile_border = vips_gmic_get_tile_border( vipsgmic );
	const VipsRect *r = &oreg->valid;

	VipsRect need;
	VipsRect image;

	need = *r;
	vips_rect_marginadjust( &need, tile_border );
	image.left = 0;
	image.top = 0;
	image.width = seq->ir[0]->im->Xsize;
	image.height = seq->ir[0]->im->Ysize;
	vips_rect_intersectrect( &need, &image, &need );

  //return 0;

	ninput = 0;
	for( int i = 0; seq->ir[i]; i++ ) {
		if( vips_region_prepare( seq->ir[i], &need ) ) 
			return( -1 );
		ninput += 1;
	}

	gmic_list<float> images;
	gmic_list<char> images_names;

	try {
		images.assign( (guint) ninput );

		for( int i = 0; seq->ir[i]; i++ ) {
			gmic_image<float> &img = images._data[i];
			img.assign( need.width, need.height, 
				1, seq->ir[i]->im->Bands );
			std::cout<<"img.assign("<<need.width<<", "<<need.height<<" ,1, "<<seq->ir[i]->im->Bands<<")"<<std::endl;
			vips_to_gmic<T>( seq->ir[0], &need, &img );
		}
		//std::cout<<"Running G'MIC command: \""<<vipsgmic->command<<"\"  seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<std::endl;
		//std::cout<<"  gmic_instance->verbosity="<<seq->gmic_instance->verbosity<<std::endl;
		//g_mutex_lock( gmic_mutex );
		char* command = strdup( vipsgmic->command );
		seq->gmic_instance->run( command, images, images_names );
		free( command );
		//g_mutex_unlock( gmic_mutex );
    //std::cout<<"G'MIC command: \""<<vipsgmic->command<<"\"  seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<" finished"<<std::endl;
		vips_from_gmic<T>( &images._data[0], &need, oreg );
	}
	catch( gmic_exception e ) { 
		images.assign( (guint) 0 );

    //std::cout<<"G'MIC command: \""<<vipsgmic->command<<"\"  seq="<<seq<<"  gmic_instance="<<seq->gmic_instance<<" failed!"<<std::endl;
	  //g_mutex_lock( gmic_mutex );
	  //delete seq->gmic_instance;
	  //g_mutex_unlock( gmic_mutex );
		vips_error( "VipsGMic", "%s", e.what() );

		return( -1 );
	}
	images.assign( (guint) 0 );

  //g_mutex_lock( gmic_mutex );
	//delete seq->gmic_instance;
  //g_mutex_unlock( gmic_mutex );

	return( 0 );
}

static int
vips_gmic_gen( VipsRegion *oreg, void *vseq, void *a, void *b, gboolean *stop )
{
	VipsGMicSequence *seq = (VipsGMicSequence *) vseq;

	//std::cout<<"vips_gmic_gen() called."<<std::endl;

	switch( seq->ir[0]->im->BandFmt ) {
	case VIPS_FORMAT_UCHAR:
		//return( vips_gmic_gen_template<unsigned char>( oreg, 
		//	seq, a, b, stop ) );
		break;

	case VIPS_FORMAT_USHORT:
		//return( vips_gmic_gen_template<unsigned short int>( oreg, 
		//	seq, a, b, stop ) );
		break;

	case VIPS_FORMAT_FLOAT:
		return( vips_gmic_gen_template<float>( oreg, 
			seq, a, b, stop ) );
		break;

	default:
		g_assert( 0 );
		break;
	}

	return( 0 );
}

/* Save a bit of typing.
 */
#define UC VIPS_FORMAT_UCHAR
#define C VIPS_FORMAT_CHAR
#define US VIPS_FORMAT_USHORT
#define S VIPS_FORMAT_SHORT
#define UI VIPS_FORMAT_UINT
#define I VIPS_FORMAT_INT
#define F VIPS_FORMAT_FLOAT
#define X VIPS_FORMAT_COMPLEX
#define D VIPS_FORMAT_DOUBLE
#define DX VIPS_FORMAT_DPCOMPLEX

/* Type promotion. 
 */
static const VipsBandFormat vips_gmic_format_table[10] = {
/* UC   C  US  S   UI  I   F   X   D   DX */
   UC,  F, US, F,  F,  F,  F,  F,  F,  F
};

static int 
vips_gmic_build( VipsObject *object )
{
	VipsObjectClass *klass = VIPS_OBJECT_GET_CLASS( object );
	VipsGMic *vipsgmic = (VipsGMic *) object;

	VipsImage **in;
	VipsImage **t;
	int ninput;
	VipsBandFormat format;

	if( VIPS_OBJECT_CLASS( vips_gmic_parent_class )->build( object ) )
		return( -1 );

	in = vips_array_image_get( vipsgmic->in, &ninput );

	for( int i = 0; i < ninput; i++ ) 
		if( vips_image_pio_input( in[i] ) || 
			vips_check_coding_known( klass->nickname, in[i] ) )  
			return( -1 );

	/* Cast all inputs up to the largest common supported format.
	 */
	format = VIPS_FORMAT_UCHAR;
	for( int i = 0; i < ninput; i++ ) 
		format = VIPS_MAX( format, in[i]->BandFmt ); 
	format = vips_gmic_format_table[format];
	t = (VipsImage **) vips_object_local_array( object, ninput );
	for( int i = 0; i < ninput; i++ )
		if( vips_cast( in[i], &t[i], format, NULL ) )
			return( -1 );
	in = t;

	g_object_set( vipsgmic, "out", vips_image_new(), NULL ); 

	if( vips_image_pipeline_array( vipsgmic->out, 
		VIPS_DEMAND_STYLE_SMALLTILE, in ) )
		return( -1 );

	if( vips_image_generate( vipsgmic->out,
		vips_gmic_start, vips_gmic_gen, vips_gmic_stop, 
		in, vipsgmic ) )
		return( -1 );

	return( 0 );
}

static void
vips_gmic_class_init( VipsGMicClass *klass )
{
	GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
	VipsObjectClass *vobject_class = VIPS_OBJECT_CLASS( klass );
	VipsOperationClass *operation_class = VIPS_OPERATION_CLASS( klass );

	gobject_class->set_property = vips_object_set_property;
	gobject_class->get_property = vips_object_get_property;

	vobject_class->nickname = "gmic";
	vobject_class->description = _( "Vips G'MIC" );
	vobject_class->build = vips_gmic_build;

	operation_class->flags = VIPS_OPERATION_SEQUENTIAL_UNBUFFERED;

	VIPS_ARG_BOXED( klass, "in", 0, 
		_( "Input" ), 
		_( "Array of input images" ),
		VIPS_ARGUMENT_REQUIRED_INPUT,
		G_STRUCT_OFFSET( VipsGMic, in ),
		VIPS_TYPE_ARRAY_IMAGE );

	VIPS_ARG_IMAGE( klass, "out", 1,
		_( "Output" ), 
		_( "Output image" ),
		VIPS_ARGUMENT_REQUIRED_OUTPUT, 
		G_STRUCT_OFFSET( VipsGMic, out ) );

	VIPS_ARG_INT( klass, "padding", 3,
		_( "padding" ), 
		_( "Tile overlap" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsGMic, padding ),
		0, INT_MAX, 0);

	VIPS_ARG_DOUBLE( klass, "x_scale", 4,
		_( "x_scale" ), 
		_( "X Scale" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsGMic, x_scale ),
		0, 100000000, 1);

	VIPS_ARG_DOUBLE( klass, "y_scale", 5,
		_( "y_scale" ), 
		_( "Y Scale" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsGMic, y_scale ),
		0, 100000000, 1);

	VIPS_ARG_STRING( klass, "command", 10, 
		_( "command" ),
		_( "G'MIC command string" ),
		VIPS_ARGUMENT_REQUIRED_INPUT, 
		G_STRUCT_OFFSET( VipsGMic, command ),
		NULL );
}

static void
vips_gmic_init( VipsGMic *vipsgmic )
{
  if( !custom_gmic_commands ) {
    std::cout<<"Loading G'MIC custom commands..."<<std::endl;
    char fname[500]; fname[0] = 0;
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
    snprintf( fname, 499, "%s\\gmic_def.gmic", PF::PhotoFlow::Instance().get_data_dir().c_str() );
    std::cout<<"G'MIC commands definition file: "<<fname<<std::endl;
    struct stat buffer;
    int stat_result = stat( fname, &buffer );
    if( stat_result != 0 ) {
      fname[0] = 0;
    }
#elif defined(__APPLE__) && defined (__MACH__)
    //snprintf( fname, 499, "%s/../share/photoflow/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
    snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_data_dir().c_str() );
    std::cout<<"G'MIC commands definition file: "<<fname<<std::endl;
    struct stat buffer;
    int stat_result = stat( fname, &buffer );
    if( stat_result != 0 ) {
      fname[0] = 0;
    }
#else
    if( getenv("HOME") ) {
      //snprintf( fname, 499, "%s/.photoflow/gmic_update.gmic", getenv("HOME") );
      snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_data_dir().c_str() );
      std::cout<<"G'MIC custom commands file: "<<fname<<std::endl;
      struct stat buffer;
      int stat_result = stat( fname, &buffer );
      if( stat_result != 0 ) {
        //snprintf( fname, 499, "%s/gmic_def.gmic", PF::PhotoFlow::Instance().get_base_dir().c_str() );
        //stat_result = stat( fname, &buffer );
        //if( stat_result != 0 ) {
          fname[0] = 0;
          //}
      }
    }
#endif
    if( strlen( fname ) > 0 ) {
      std::ifstream t;
      int length;
      t.open(fname);      // open input file
      t.seekg(0, std::ios::end);    // go to the end
      length = t.tellg();           // report location (this is the length)
      t.seekg(0, std::ios::beg);    // go back to the beginning
      custom_gmic_commands = new char[length];    // allocate memory for a buffer of appropriate dimension
      t.read(custom_gmic_commands, length);       // read the whole file into the buffer
      t.close();                    // close file handle
      std::cout<<"G'MIC custom commands loaded"<<std::endl;
    }
  }

  if( !gmic_mutex ) {
    gmic_mutex = vips_g_mutex_new();
  }
}

extern "C" {
/**
 * vips_gmic:
 * @in: (array length=n) (transfer none): array of input images
 * @out: output image
 * @n: number of input images
 * @padding: overlap tiles by this much
 * @x_scale: 
 * @y_scale: 
 * @command: command to execute
 *
 * Returns: 0 on success, -1 on failure. 
 */
G_MODULE_EXPORT int
vips_gmic( VipsImage **in, VipsImage **out, int n, 
	int padding, double x_scale, double y_scale, const char *command, ... )
{
	VipsArrayImage *array; 
	va_list ap;
	int result;

#ifndef NDEBUG
	printf("vips_gmic(): padding=%d\n", padding);
#endif

	array = vips_array_image_new( in, n ); 
	va_start( ap, command );
	result = vips_call_split( "gmic", ap, array, out, 
		padding, x_scale, y_scale, command );
	va_end( ap );
	vips_area_unref( VIPS_AREA( array ) );

	return( result );
}
}

extern "C" {
/* This is called on module load.
 */
const gchar *
g_module_check_init( GModule *module )
{
#ifdef DEBUG
	printf( "vips_gmic: module init\n" ); 
#endif /*DEBUG*/

	vips_gmic_get_type();

	/* We can't be unloaded, there would be chaos.
	 */
	//g_module_make_resident( module );

	return( NULL ); 
}
}
