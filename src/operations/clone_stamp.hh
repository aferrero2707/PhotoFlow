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

#ifndef PF_CLONE_STAMP_H
#define PF_CLONE_STAMP_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/rawbuffer.hh"

//#include "diskbuffer.hh"
//#include "blender.hh"

namespace PF 
{
  
  class Stamp
  {
    unsigned int size;
    float opacity;
    float smoothness;
  public:
    Stamp() {}
    void set_size( unsigned int s ) { size = s; }
    unsigned int get_size() const { return size; }

    void set_opacity( float val ) { opacity = val; }
    float get_opacity() const { return opacity; }

    void set_smoothness( float val ) { smoothness = val; }
    float get_smoothness() const { return smoothness; }
  };


  inline bool operator ==(const Stamp& l, const Stamp& r)
  {
    if( l.get_opacity() != r.get_opacity() ) return false;
    if( l.get_smoothness() != r.get_smoothness() ) return false;
    if( l.get_size() != r.get_size() ) return false;
    return true;
  }

  inline bool operator !=(const Stamp& l, const Stamp& r)
  {
    return( !(l==r) );
  }


  inline std::istream& operator >>( std::istream& str, Stamp& pen )
  {
    unsigned int size;
    float opacity;
    float smoothness;
    str>>size>>opacity>>smoothness;
    pen.set_size( size );
    pen.set_opacity( opacity );
    pen.set_smoothness( smoothness );
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const Stamp& pen )
  {
    str<<pen.get_size()<<" "<<pen.get_opacity()<<" "<<pen.get_smoothness()<<" ";
    return str;
  }



  class StrokesGroup
  {
    int delta_row, delta_col;
    std::list< Stroke<Stamp> > strokes;
  public:
    StrokesGroup(): delta_row(0), delta_col(0)
    {
    }

    int get_delta_row() const { return delta_row; }
    int set_delta_row( int d ) { delta_row = d; }
    int get_delta_col() const { return delta_col; }
    int set_delta_col( int d ) { delta_col = d; }
    std::list< Stroke<Stamp> >& get_strokes() { return strokes; }
    const std::list< Stroke<Stamp> >& get_strokes() const { return strokes; }
  };



  inline bool operator ==(const StrokesGroup& l, const StrokesGroup& r)
  {
    if( l.get_delta_row() != r.get_delta_row() ) return false;
    if( l.get_delta_col() != r.get_delta_col() ) return false;
    if( l.get_strokes() != r.get_strokes() ) return false;
    return true;
  }

  inline bool operator !=(const StrokesGroup& l, const StrokesGroup& r)
  {
    return( !(l==r) );
  }


  inline std::istream& operator >>( std::istream& str, StrokesGroup& group )
  {
    int delta_row, delta_col;
    str>>delta_row>>delta_col;
    str>>group.get_strokes();
    group.set_delta_row( delta_row );
    group.set_delta_col( delta_col );
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const StrokesGroup& group )
  {
    str<<group.get_delta_row()<<" "<<group.get_delta_col()<<" ";
    str<<group.get_strokes();
    return str;
  }



  template<> inline
  void set_gobject_property< std::list<StrokesGroup> >(gpointer object, const std::string name, 
                                                       const std::list<StrokesGroup>& value)
  {
  }



  class CloneStampPar: public OpParBase
  {
    Property<int> stamp_size;
    Property<float> stamp_opacity;
    Property<float> stamp_smoothness;
    Property< std::list<StrokesGroup> > strokes;

    int scale_factor;

    Stamp pen;

  public:
    CloneStampPar();
    ~CloneStampPar();

    bool has_intensity() { return false; }
    bool needs_input() { return true; }

    Stamp& get_pen() { return pen; }

    int get_scale_factor() { return scale_factor; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);

    void new_group( int dr, int dc );

    void start_stroke()
    {
      start_stroke( stamp_size.get(), stamp_opacity.get(), stamp_smoothness.get() );
    }
    void start_stroke( unsigned int pen_size, float opacity, float smoothness );
    void end_stroke();

    Property< std::list<StrokesGroup> >& get_strokes() { return strokes; }

    void draw_point( unsigned int x, unsigned int y, VipsRect& update );
  };

  

  template < OP_TEMPLATE_DEF > 
  class CloneStampProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, CloneStampPar* par)
    {
    }
  };


  ProcessorBase* new_clone_stamp();
}

#endif 


