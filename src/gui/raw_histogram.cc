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
#include "raw_histogram.hh"


//#define OPTIMIZE_SCROLLING


static std::ostream& operator <<( std::ostream& str, const VipsRect& r )
{
  str<<r.width<<","<<r.height<<"+"<<r.left<<"+"<<r.top;
  return str;
}


PF::RawHistogram::RawHistogram(  Image* i  ): image(i)
{
  hist = new unsigned long int[65536*3];
  //std::cout<<"Histogram::Histogram(): hist="<<hist<<std::endl;
  drawing_area.set_size_request( 290, 100 );
}

PF::RawHistogram::~RawHistogram ()
{
  if(hist) delete hist;
}



#ifdef GTKMM_2
bool PF::RawHistogram::on_expose_event (GdkEventExpose * event)
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
bool PF::RawHistogram::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
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

