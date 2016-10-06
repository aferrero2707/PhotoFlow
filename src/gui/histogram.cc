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


#include "../base/imageprocessor.hh"
#include "histogram.hh"


//#define OPTIMIZE_SCROLLING


static std::ostream& operator <<( std::ostream& str, const VipsRect& r )
{
  str<<r.width<<","<<r.height<<"+"<<r.left<<"+"<<r.top;
  return str;
}


static void* histogram_start( struct _VipsImage *out, void *a, void *b )
{
  //std::cout<<"Starting histogramming sequence"<<std::endl;
  PF::Histogram* histogram = (PF::Histogram*)a;
  //std::cout<<"histogram="<<histogram<<std::endl;
  //std::cout<<"histogram->hist="<<histogram->hist<<std::endl;
  unsigned long int* hist = new unsigned long int[65536*3];
  for( int i = 0; i < 65536*3; i++ ) {
    hist[i] = 0;
  }
  return hist;
}


static int histogram_stop( void* seq, void *a, void *b )
{
  unsigned long int* hist = (unsigned long int*)seq;
  PF::Histogram* histogram = (PF::Histogram*)a;
  for( int i = 0; i < 65536*3; i++ ) {
    //if( hist[i]>0 ) std::cout<<"histogram->hist["<<i<<"] += "<<hist[i]<<std::endl;
    histogram->hist[i] += hist[i];
  }
  delete hist;
  return 1;
}



/* Loop over region, accumulating a sum in *tmp.
 */
static int histogram_scan( VipsRegion *region,
    void *seq, void *a, void *b, gboolean *stop )
{
  VipsRect *r = &region->valid;
  int lsk = VIPS_REGION_LSKIP( region );
  int bands = vips_image_get_bands( region->im );
  int lsz = bands * r->width;

  int x, y;
  VipsPel *p;

  unsigned long int* hist = (unsigned long int*)seq;
  unsigned long int* h1 = hist;
  unsigned long int* h2 = &(hist[65536]);
  unsigned long int* h3 = &(hist[65536*2]);

  p = VIPS_REGION_ADDR( region, r->left, r->top );
  for( y = 0; y < r->height; y++ ) {
    switch( vips_image_get_format( region->im ) ) {
    case VIPS_FORMAT_USHORT: {
      unsigned short int* pp = (unsigned short int*)p;
      for( x = 0; x < lsz; x+=bands ) {
        h1[ pp[x] ] += 1;
        h2[ pp[x+1] ] += 1;
        h3[ pp[x+2] ] += 1;
      }
      break;
    }
    case VIPS_FORMAT_FLOAT: {
      float* pp = (float*)p;
      for( x = 0; x < lsz; x+=bands ) {
        unsigned short int idx;
        PF::from_float( MAX( MIN(pp[x], 1), 0), idx );
        h1[ idx ] += 1;
        PF::from_float( MAX( MIN(pp[x+1], 1), 0), idx );
        h2[ idx ] += 1;
        PF::from_float( MAX( MIN(pp[x+2], 1), 0), idx );
        h3[ idx ] += 1;
      }
      break;
    }
    default:
      break;
    }
    p += lsk;
  }

  return( 0 );
}

gboolean PF::Histogram::queue_draw_cb (PF::Histogram::Update * update)
{
  update->histogram->queue_draw();
  //std::cout<<"update->histogram->queue_draw() called"<<std::endl;
  g_free (update);
  return FALSE;
}



PF::Histogram::Histogram( Pipeline* v ):
  PipelineSink( v ),
  display_merged( true ),
  active_layer( -1 )
{
  hist = new unsigned long int[65536*3];
  //std::cout<<"Histogram::Histogram(): hist="<<hist<<std::endl;
  set_size_request( 270, 150 );
}

PF::Histogram::~Histogram ()
{
  if(hist) delete hist;
}



#ifdef GTKMM_2
bool PF::Histogram::on_expose_event (GdkEventExpose * event)
{
  //std::cout<<"Histogram::on_expose_event() called"<<std::endl;
  int border_size = 2;

  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window )
    return true;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

#endif
#ifdef GTKMM_3
bool PF::Histogram::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  int border_size = 2;
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;
#endif
  cr->save();
  cr->set_source_rgba(0.2, 0.2, 0.2, 1.0);
  cr->paint();
  cr->restore();

  // Draw outer rectangle
  cr->set_antialias( Cairo::ANTIALIAS_GRAY );
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->set_line_width( 0.5 );
  cr->rectangle( double(0.5), double(0.5),
      double(allocation.get_width()-1),
      double(allocation.get_height()-1) );
  cr->stroke ();

  unsigned long int* h[3];
  h[0] = hist;
  h[1] = &(hist[65536]);
  h[2] = &(hist[65536*2]);

  unsigned long int* hh[3];
  hh[0] = new unsigned long int[width];
  hh[1] = new unsigned long int[width];
  hh[2] = new unsigned long int[width];

  unsigned long int max = 0;
  for( int i = 0; i < 3; i++ ) {
    for( int j = 0; j < width; j++ ) {
      hh[i][j] = 0;
    }
    for( int j = 0; j < 65536; j++ ) {
      float nj = j; nj /= 65536; nj *= width;
      int bin = (int)nj;
      //if(j==65535) std::cout<<"j="<<j<<"  bin="<<bin<<"  width="<<width<<std::endl;
      hh[i][bin] += h[i][j];
    }
    for( int j = 0; j < width; j++ ) {
      if( hh[i][j] > max) max = hh[i][j];
    }
  }

  for( int i = 0; i < 3; i++ ) {
    if( i == 0 ) cr->set_source_rgb( 0.9, 0., 0. );
    if( i == 1 ) cr->set_source_rgb( 0., 0.9, 0. );
    if( i == 2 ) cr->set_source_rgb( 0.2, 0.6, 1. );

    float ny = 0;
    if( max > 0 ) {
      ny = hh[i][0]; ny *= height; ny /= max;
    }
    float y = height; y -= ny; y -= 1;
    cr->move_to( x0, y+y0 );
    for( int j = 1; j < width; j++ ) {
      ny = 0;
      if( max > 0 ) {
        ny = hh[i][j]; ny *= height; ny /= max;
      }
      y = height; y -= ny; y -= 1;
      //y = (max>0) ? height - hh[i][j]*height/max - 1 : height-1;
      //std::cout<<"bin #"<<j<<": "<<hh[i][j]<<" ("<<max<<") -> "<<y<<std::endl;
      cr->line_to( j+x0, y+y0 );
    }
    cr->stroke();
  }

  return TRUE;
}





static VipsImage* convert_raw_data( VipsImage* raw )
{
  if( !raw ) return NULL;

  // The image to be displayed has two channels, we assume in this case that it represents RAW data
  // and we only show the first channel (the second channel contains the color information)
  VipsImage* band = NULL;
  if( vips_extract_band( raw, &band, 0, NULL ) ) {
    std::cout<<"Histogram::update(): vips_extract_band() failed."<<std::endl;
    return NULL;
  }

  vips_image_init_fields( band,
      raw->Xsize, raw->Ysize,
      1, raw->BandFmt,
      raw->Coding,
      raw->Type,
      1.0, 1.0);

  VipsImage* norm = NULL;
  double par1 = 1.0f/65535.0f;
  double par2 = 0;
  if( vips_linear( band, &norm, &par1, &par2, 1, NULL ) ) {
    PF_UNREF( band, "Histogram::update(): band unref after vips_linear() failure" );
    std::cout<<"Histogram::update(): vips_linear() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( band, "Histogram::update(): band unref after vips_linear()" );

  VipsImage* gamma = NULL;
  float exp = 2.2;
  if( vips_gamma( norm, &gamma, "exponent", exp, NULL ) ) {
    PF_UNREF( norm, "Histogram::update(): norm unref after vips_gamma() failure" );
    std::cout<<"Histogram::update(): vips_gamma() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( norm, "Histogram::update(): norm unref after vips_gamma()" );

  VipsImage* cast = gamma;
  /*
if( vips_cast_ushort( gamma, &cast, NULL ) ) {
  PF_UNREF( gamma, "Histogram::update(): gamma unref after vips_cast_ushort() failure" );
  std::cout<<"Histogram::update(): vips_cast_ushort() failed."<<std::endl;
  return NULL;
}
PF_UNREF( gamma, "Histogram::update(): gamma unref after vips_cast_ushort()" );
   */

  VipsImage* bandv[3] = {cast, cast, cast};
  VipsImage* out = NULL;
  if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
    PF_UNREF( cast, "Histogram::update(): cast unref after bandjoin failure" );
    std::cout<<"Histogram::update(): vips_bandjoin() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( cast, "Histogram::update(): cast unref after bandjoin" );

  return out;
}



void PF::Histogram::update( VipsRect* area )
{
  //PF::Pipeline* pipeline = pf_image->get_pipeline(0);

#ifdef DEBUG_DISPLAY
  std::cout<<"PF::Histogram::update(): called"<<std::endl;
#endif
  if( !get_pipeline() ) {
    std::cout<<"Histogram::update(): error: NULL pipeline"<<std::endl;
    return;
  }
  if( !get_pipeline()->get_output() ) {
    std::cout<<"Histogram::update(): error: NULL image"<<std::endl;
    return;
  }

  //return;

  VipsImage* image = NULL;
  bool do_merged = display_merged;
  //std::cout<<"Histogram::update(): do_merged="<<do_merged<<"  active_layer="<<active_layer<<std::endl;
  if( !do_merged ) {
    if( active_layer < 0 ) do_merged = true;
    else {
      PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
      if( !node ) do_merged = true;
      //std::cout<<"Histogram::update(): node="<<node<<std::endl;
      if( node->processor &&
          node->processor->get_par() &&
          node->processor->get_par()->is_map() )
        do_merged = true;
      if( get_pipeline()->get_image() ) {
        PF::Layer* temp_layer = get_pipeline()->get_image()->get_layer_manager().get_layer( active_layer );
        if( !temp_layer ) do_merged = true;
        if( !(temp_layer->is_visible()) ) do_merged = true;
      }
    }
  }
  if( do_merged ) {
    image = get_pipeline()->get_output();
#ifdef DEBUG_DISPLAY
    std::cout<<"Histogram::update(): image="<<image<<std::endl;
    std::cout<<"                     image->Bands="<<image->Bands<<std::endl;
    std::cout<<"                     image->BandFmt="<<image->BandFmt<<std::endl;
    std::cout<<"                     image size: "<<image->Xsize<<"x"<<image->Ysize<<std::endl;
#endif
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "Histogram::update(): merged image ref" );
    } else {
      image = convert_raw_data( image );
    }
  } else {
    PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
    if( !node ) return;
    if( !(node->blended) ) return;

    image = node->blended;
#ifdef DEBUG_DISPLAY
    std::cout<<"Histogram::update(): node->image("<<node->image<<")->Xsize="<<node->image->Xsize
        <<"    node->image->Ysize="<<node->image->Ysize<<std::endl;
    std::cout<<"Histogram::update(): node->blended("<<node->blended<<")->Xsize="<<node->blended->Xsize
        <<"    node->blended->Ysize="<<image->Ysize<<std::endl;
#endif
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "Histogram::update(): active image ref" );
    } else {
      image = convert_raw_data( image );
    }
  }
  if( !image ) return;

  for( int j = 0; j < 65536*3; j++ ) {
    hist[j] = 0;
  }

  //std::cout<<"before vips_sink()"<<std::endl;
  vips_sink( image, histogram_start, histogram_scan, histogram_stop, this, NULL );
  //std::cout<<"after vips_sink()"<<std::endl;
  PF_UNREF( image, "Histogram::update(): image unref after vips_sink()" );

  Update * update = g_new (Update, 1);
  update->histogram = this;
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
}



