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

  class Pen
  {
    std::vector<float> color;
    unsigned int size;
    float opacity;
  public:
    Pen() {}
    void set_size( unsigned int s ) { size = s; }
    unsigned int get_size() const { return size; }

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
  };


  inline bool operator ==(const Pen& l, const Pen& r)
  {
    if( l.get_color() != r.get_color() ) return false;
    if( l.get_opacity() != r.get_opacity() ) return false;
    if( l.get_size() != r.get_size() ) return false;
    return true;
  }

  inline bool operator !=(const Pen& l, const Pen& r)
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




  class Stroke
  {
    Pen pen;
    std::list<Segment> segments;
    std::list< std::pair<unsigned int, unsigned int> > points;

  public:
    Stroke() {}

    Pen& get_pen() { return pen; }
    const Pen& get_pen() const { return pen; }
    std::list< std::pair<unsigned int, unsigned int> >& get_points() { return points; }
    const std::list< std::pair<unsigned int, unsigned int> >& get_points() const { return points; }
    std::list< Segment >& get_segments() { return segments; }
  };


  inline bool operator ==(const Stroke& l, const Stroke& r)
  {
    if( l.get_pen() != r.get_pen() ) return false;
    if( l.get_points() != r.get_points() ) return false;
    return true;
  }

  inline bool operator !=(const Stroke& l, const Stroke& r)
  {
    return( !(l==r) );
  }


  class RawBuffer
  {
    std::string file_name;
    int fd;
    void* buf;
    unsigned char* pxmask;

    // Requested image fields
    int xsize;
    int ysize;
    int bands;
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

    void draw_row( Pen& pen, unsigned int row, 
                   unsigned int startcol, unsigned int endcol );
    void draw_point( Pen& pen, unsigned int x, unsigned int y, VipsRect& update, bool update_pyramid );
    void draw_segment( Pen& pen, Segment& segment );

    void start_stroke();
    void end_stroke();

    //void draw_stroke( Stroke& stroke );
  };


};

#endif
