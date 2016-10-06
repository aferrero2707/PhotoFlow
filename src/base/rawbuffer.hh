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


#ifndef RAW_BUFFER_H
#define RAW_BUFFER_H

#include <math.h>

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>


#include <vips/vips.h>
//#include <vips/vips>

#include "pftypes.hh"

#include "format_info.hh"

#include "property.hh"

#include "imagepyramid.hh"



namespace PF 
{

class PencilMask
{
  unsigned int size;
  float opacity;
  float smoothness;
  float** mask;
public:
  PencilMask(): size(0), opacity(1), smoothness(0), mask(NULL)
{
}

  PencilMask( const PencilMask& sm ):
    size(0),
    opacity(1),
    smoothness(0),
    mask(NULL)
  {
    init( sm.get_size(), sm.get_opacity(), sm.get_smoothness() );
  }

  ~PencilMask()
  {
    //std::cout<<"~PencilMask() called. mask="<<mask<<std::endl;
    //return;
    if( mask ) {
      for( unsigned int i = 0; i < size; i++) delete[] mask[i];
      delete[] mask;
      mask = NULL;
    }
  }

  PencilMask& operator=( const PencilMask& sm )
  {
    init( sm.get_size(), sm.get_opacity(), sm.get_smoothness() );
    return ( *this );
  }

  unsigned int get_size() const { return size; }
  float get_opacity() const { return opacity; }
  float get_smoothness() const { return smoothness; }

  void init(unsigned int s, float op, float sm);

  float get(int x, int y )
  {
    if( !mask ) {
      std::cout<<"PencilMask::get(): WARNING: mask = NULL"<<std::endl;
      return 1;
    }
    if( x < 0 || x >= (int)size) {
      std::cout<<"PencilMask::get(): WARNING: x value out of range ("<<x<<")"<<std::endl;
      return 1;
    }
    if( y < 0 || y >= (int)size) {
      std::cout<<"PencilMask::get(): WARNING: y value out of range ("<<y<<")"<<std::endl;
      return 1;
    }
    return mask[y][x];
  }
};



class Pencil
{
  std::vector<float> color;
  int size;
  float opacity;
  float smoothness;
  PencilMask mask;
public:
  Pencil(): size(0), opacity(1), smoothness(0.5) {}
  Pencil( const Pencil& p )
  {
    //std::cout<<"Stamp(const Stamp& s) called."<<std::endl;
    size = p.get_size();
    opacity = p.get_opacity();
    smoothness = p.get_smoothness();
    init_mask();
  }

  void set_size( int s ) { size = s; init_mask(); }
  int get_size() const { return size; }

  void set_channel( unsigned int ch, float val )
  {
    if( color.size() <= ch )
      color.resize( ch+1 );
    color[ch] = val;
  }
  float get_channel( unsigned int ch ) const
  {
    if( color.size() <= ch ) return 0;
    return color[ch];
  }
  std::vector<float>& get_color() { return color; }
  const std::vector<float>& get_color() const { return color; }

  void set_opacity( float val ) { opacity = val; }
  float get_opacity() const { return opacity; }

  void set_smoothness( float val ) { smoothness = val; init_mask(); }
  float get_smoothness() const { return smoothness; }

  void init_mask()
  {
    mask.init( size*2+1, opacity, smoothness );
  }
  PencilMask& get_mask() { return mask; }

  Pencil & operator=(const Pencil &p)
  {
    color = p.get_color();
    size = p.get_size();
    opacity = p.get_opacity();
    smoothness = p.get_smoothness();
    init_mask();
    return(*this);
  }
};


inline
bool operator ==(const Pencil& l, const Pencil& r)
{
  if( l.get_color() != r.get_color() ) return false;
  if( l.get_opacity() != r.get_opacity() ) return false;
  if( l.get_size() != r.get_size() ) return false;
  if( l.get_smoothness() != r.get_smoothness() ) return false;
  return true;
}

inline
bool operator !=(const Pencil& l, const Pencil& r)
{
  return( !(l==r) );
}




class Segment
{
  unsigned int x1, y1, x2, y2;
public:
  Segment(): x1(0), y1(0), x2(0), y2(0) {}
  void set( unsigned int _x1, unsigned int _y1,
      unsigned int _x2, unsigned int _y2 )
  {
    x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2;
  }
  unsigned int get_x1() { return x1; }
  unsigned int get_y1() { return y1; }
  unsigned int get_x2() { return x2; }
  unsigned int get_y2() { return y2; }
};




template <class Pen>
class Stroke
{
  Pen pen;
  std::list<Segment> segments;
  std::list< std::pair<int, int> > points;

  VipsRect area;

public:
  Stroke() {}

  Pen& get_pen() { return pen; }
  const Pen& get_pen() const { return pen; }
  std::list< std::pair<int, int> >& get_points() { return points; }
  const std::list< std::pair<int, int> >& get_points() const { return points; }
  std::list< Segment >& get_segments() { return segments; }
  VipsRect& get_area() { return area; }
  void compute_area();
};


template <class Pen>
void PF::Stroke<Pen>::compute_area()
{
  int xmin = 10000000, xmax = -10000000;
  int ymin = 10000000, ymax = -10000000;
  std::list< std::pair<int, int> >::iterator pi;
  for( pi = points.begin(); pi != points.end(); pi++ ) {
    int x1 = pi->first - pen.get_size();
    int x2 = pi->first + pen.get_size();
    int y1 = pi->second - pen.get_size();
    int y2 = pi->second + pen.get_size();

    xmin = MIN( xmin, x1 );
    xmax = MAX( xmax, x2 );
    ymin = MIN( ymin, y1 );
    ymax = MAX( ymax, y2 );
  }

  //std::cout<<"Stroke<Pen>::compute_area(): points.size()="<<points.size()<<"  xmin="<<xmin<<"  xmax="<<xmax<<"  ymin="<<ymin<<"  ymax="<<ymax<<std::endl;

  area.left = xmin;
  area.top = ymin;
  area.width = xmax - xmin + 1;
  area.height = ymax - ymin + 1;
}



template <class Pen>
inline bool operator ==(const Stroke<Pen>& l, const Stroke<Pen>& r)
{
  if( l.get_pen() != r.get_pen() ) return false;
  if( l.get_points() != r.get_points() ) return false;
  return true;
}

template <class Pen>
inline bool operator !=(const Stroke<Pen>& l, const Stroke<Pen>& r)
{
  return( !(l==r) );
}


inline std::istream& operator >>( std::istream& str, Pencil& pen )
{
  unsigned int nch;
  str>>nch;
  std::vector<float>& color = pen.get_color();
  color.clear();
  for( unsigned int i = 0; i < nch; i++ ) {
    float val;
    str>>val;
    color.push_back( val );
  }
  unsigned int size;
  //float opacity;
  //str>>size>>opacity;
  float sharpness;
  str>>size>>sharpness;
  pen.set_size( size );
  //pen.set_opacity( opacity );
  pen.set_smoothness( 1.0f-sharpness );
  return str;
}

inline std::ostream& operator <<( std::ostream& str, const Pencil& pen )
{
  const std::vector<float>& color = pen.get_color();
  str<<color.size()<<" ";
  std::list< std::pair<unsigned int, unsigned int> >::iterator i;
  for( unsigned int i = 0; i < color.size(); i++ ) {
    str<<color[i]<<" ";
  }
  //str<<pen.get_size()<<" "<<pen.get_opacity()<<" ";
  str<<pen.get_size()<<" "<<1.0f-pen.get_smoothness()<<" ";
  return str;
}



template <class Pen>
inline std::istream& operator >>( std::istream& str, Stroke<Pen>& stroke )
{
  str>>stroke.get_pen();
  std::list< std::pair<int, int> >& points = stroke.get_points();
  points.clear();
  int npoints;
  str>>npoints;
  for( int i = 0; i < npoints; i++ ) {
    int a, b;
    str>>a>>b;
    points.push_back( std::make_pair(a,b) );
  }
  return str;
}

template <class Pen>
inline std::ostream& operator <<( std::ostream& str, const Stroke<Pen>& stroke )
{
  str<<stroke.get_pen();
  const std::list< std::pair<int, int> >& points = stroke.get_points();
  str<<points.size()<<" ";
  std::list< std::pair<int, int> >::const_iterator i;
  for( i = points.begin(); i != points.end(); i++ ) {
    str<<i->first<<" "<<i->second<<" ";
  }
  return str;
}


/*
  template <class Pen>
  inline std::istream& operator >>( std::istream& str, std::list< Stroke<Pen> >& strokes )
  {
    strokes.clear();
    int nstrokes;
    str>>nstrokes;
    for( int i = 0; i < nstrokes; i++ ) {
      strokes.push_back( Stroke<Pen>() );
      Stroke<Pen>& stroke = strokes.back();
      str>>stroke;
    }
    return str;
  }

  template <class Pen>
  inline std::ostream& operator <<( std::ostream& str, const std::list< Stroke<Pen> >& strokes )
  {
    str<<strokes.size()<<" ";
    typename std::list< Stroke<Pen> >::const_iterator i;
    for( i = strokes.begin(); i != strokes.end(); i++ ) {
      str<<(*i);
    }
    return str;
  }
 */

/*
  template <class T>
  inline bool operator ==(const std::list<T>& l, const std::list<T>& r)
  {
    if( l.size() != r.size() ) return false;
    typename std::list<T>::const_iterator i, j;
    for( i = l.begin(), j = r.begin(); i != l.end(); i++, j++ ) {
      if( (*i) != (*j) ) return false;
    }
    return true;
  }

  template <class T>
  inline bool operator !=(const std::list<T>& l, const std::list<T>& r)
  {
    return( !(l==r) );
  }
 */

/*
  template <class T>
  inline std::istream& operator >>( std::istream& str, std::list<T>& list )
  {
    list.clear();
    int n;
    str>>n;
    for( int i = 0; i < n; i++ ) {
      list.push_back( T() );
      T& val = list.back();
      str>>val;
    }
    return str;
  }

  template <class T>
  inline std::ostream& operator <<( std::ostream& str, const std::list<T>& list )
  {
    str<<list.size()<<" ";
    typename std::list<T>::const_iterator i;
    for( i = list.begin(); i != list.end(); i++ ) {
      str<<(*i);
    }
    return str;
  }
 */


template<> inline
void set_gobject_property< std::list< Stroke<Pencil> > >(gpointer object, const std::string name,
    const std::list< Stroke<Pencil> >& value)
    {
    }



class RawBuffer
{
  std::string file_name;
  int fd;
  void* buf;
  unsigned char* pxmask;

  // Requested image fields
  unsigned int xsize;
  unsigned int ysize;
  unsigned int bands;
  VipsBandFormat format;
  VipsCoding coding;
  VipsInterpretation interpretation;

  std::vector<float> bgd_color;

  VipsImage* image;

  ImagePyramid pyramid;

  std::vector< std::list< std::pair<unsigned int, unsigned int> > > stroke_ranges;

public:
  RawBuffer();
  RawBuffer( std::string file_name );

  virtual ~RawBuffer()
  {
    std::cout<<"PF::RawBuffer::~RawBuffer(): deleting "<<(void*)this<<std::endl;
    if( fd >= 0 ) {
      close( fd );
      unlink( file_name.c_str() );
      std::cout<<"PF::RawBuffer::~RawBuffer(): "<<file_name<<" removed."<<std::endl;
    }
  }

  std::string get_file_name() { return file_name; }
  int get_fd() { return fd; }

  int get_xsize() { return xsize; }
  int get_ysize() { return ysize; }

  void set_xsize(int sz) { xsize = sz; }
  void set_ysize(int sz) { ysize = sz; }

  int get_nbands() { return bands; }
  void set_nbands( int n ) { bands = n; }

  VipsInterpretation get_interpretation() { return interpretation; }
  colorspace_t get_colorspace() { return( PF::convert_colorspace( get_interpretation() ) ); }
  void set_interpretation( VipsInterpretation val ) { interpretation = val; }

  VipsBandFormat get_format() { return format; }
  void set_format( VipsBandFormat fmt ) { format = fmt; }

  VipsCoding get_coding() { return coding; }
  void set_coding( VipsCoding c ) { coding = c; }

  std::vector<float> get_bgd_color() { return bgd_color; }

  ImagePyramid& get_pyramid() { return pyramid; }

  void init( const std::vector<float>& bgd_color );

  void draw_row( Pencil& pen, unsigned int row,
      unsigned int startcol, unsigned int endcol );
  void draw_point( Pencil& pen, unsigned int x, unsigned int y, VipsRect& update, bool update_pyramid );
  void draw_segment( Pencil& pen, Segment& segment );

  void start_stroke();
  void end_stroke();

  //void draw_stroke( Stroke& stroke );
};


};

#endif
