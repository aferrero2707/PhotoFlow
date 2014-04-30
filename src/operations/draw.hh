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

#ifndef PF_DRAW_H
#define PF_DRAW_H

#include <iostream>

#include "../base/format_info.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/rawbuffer.hh"

#include "blender.hh"

namespace PF 
{

  struct RGBColor
  {
    float r, g, b;

    RGBColor(): r(0), g(0), b(0) {}
    RGBColor( float _r, float _g, float _b ): r(_r), g(_g), b(_b) {}
    RGBColor( const RGBColor& color ):
      r( color.r ), g( color.g ), b( color.b )
    {
    }
    RGBColor & operator=(const RGBColor &c)
    {
      r = c.r;
      g = c.g;
      b = c.b;
      return(*this);
    }
  };

  inline std::istream& operator >>( std::istream& str, RGBColor& color )
  {
    str>>color.r>>color.g>>color.b;
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const RGBColor& color )
  {
    str<<color.r<<" "<<color.g<<" "<<color.b<<" ";
    return str;
  }

  template<> inline
  void set_gobject_property<RGBColor>(gpointer object, const std::string name, const RGBColor& value)
  {
    //g_object_set( object, name.c_str(), value, NULL );
  }



  inline std::istream& operator >>( std::istream& str, Pen& pen )
  {
    unsigned int nch;
    str>>nch;
    std::vector<float>& color = pen.get_color();
    color.clear();
    for( int i = 0; i < nch; i++ ) {
      float val;
      str>>val;
      color.push_back( val );
    }
    unsigned int size;
    float opacity;
    str>>size>>opacity;
    pen.set_size( size );
    pen.set_opacity( opacity );
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const Pen& pen )
  {
    const std::vector<float>& color = pen.get_color();
    str<<color.size()<<" ";
    std::list< std::pair<unsigned int, unsigned int> >::iterator i;
    for( unsigned int i = 0; i < color.size(); i++ ) {
      str<<color[i]<<" ";
    }
    str<<pen.get_size()<<" "<<pen.get_opacity()<<" ";
    return str;
  }



  inline std::istream& operator >>( std::istream& str, Stroke& stroke )
  {
    str>>stroke.get_pen();
    std::list< std::pair<unsigned int, unsigned int> >& points = stroke.get_points();
    points.clear();
    int npoints;
    str>>npoints;
    for( int i = 0; i < npoints; i++ ) {
      unsigned int a, b;
      str>>a>>b;
      points.push_back( std::make_pair(a,b) );
    }
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const Stroke& stroke )
  {
    str<<stroke.get_pen();
    const std::list< std::pair<unsigned int, unsigned int> >& points = stroke.get_points();
    str<<points.size()<<" ";
    std::list< std::pair<unsigned int, unsigned int> >::const_iterator i;
    for( i = points.begin(); i != points.end(); i++ ) {
      str<<i->first<<" "<<i->second<<" ";
    }
    return str;
  }



  inline std::istream& operator >>( std::istream& str, std::list<Stroke>& strokes )
  {
    strokes.clear();
    int nstrokes;
    str>>nstrokes;
    for( int i = 0; i < nstrokes; i++ ) {
      strokes.push_back( Stroke() );
      Stroke& stroke = strokes.back();
      str>>stroke;
    }
    return str;
  }

  inline std::ostream& operator <<( std::ostream& str, const std::list<Stroke>& strokes )
  {
    str<<strokes.size()<<" ";
    std::list<Stroke>::const_iterator i;
    for( i = strokes.begin(); i != strokes.end(); i++ ) {
      str<<(*i);
    }
    return str;
  }

  template<> inline
  void set_gobject_property< std::list<Stroke> >(gpointer object, const std::string name, 
						 const std::list<Stroke>& value)
  {
  }




  class DrawPar: public BlenderPar
  {
    Property<float> pen_grey, pen_R, pen_G, pen_B, pen_L, pen_a, pen_b, pen_C, pen_M, pen_Y, pen_K;
    Property<float> bgd_grey, bgd_R, bgd_G, bgd_B, bgd_L, bgd_a, bgd_b, bgd_C, bgd_M, bgd_Y, bgd_K;
    Property<RGBColor> pen_color;
    Property<RGBColor> bgd_color;
    Property<int> pen_size;
    Property<float> pen_opacity;
    Property< std::list<Stroke> > strokes;


    RawBuffer* rawbuf;

    Pen pen;

  public:
    DrawPar();
    ~DrawPar();

    Property<float>& get_pen_grey() { return pen_grey; }
    Property<float>& get_pen_R() { return pen_R; }
    Property<float>& get_pen_G() { return pen_G; }
    Property<float>& get_pen_B() { return pen_B; }
    Property<float>& get_pen_L() { return pen_L; }
    Property<float>& get_pen_a() { return pen_a; }
    Property<float>& get_pen_b() { return pen_a; }
    Property<float>& get_pen_C() { return pen_C; }
    Property<float>& get_pen_M() { return pen_M; }
    Property<float>& get_pen_Y() { return pen_Y; }
    Property<float>& get_pen_K() { return pen_K; }

    Property<float>& get_bgd_grey() { return bgd_grey; }
    Property<float>& get_bgd_R() { return bgd_R; }
    Property<float>& get_bgd_G() { return bgd_G; }
    Property<float>& get_bgd_B() { return bgd_B; }
    Property<float>& get_bgd_L() { return bgd_L; }
    Property<float>& get_bgd_a() { return bgd_a; }
    Property<float>& get_bgd_b() { return bgd_a; }
    Property<float>& get_bgd_C() { return bgd_C; }
    Property<float>& get_bgd_M() { return bgd_M; }
    Property<float>& get_bgd_Y() { return bgd_Y; }
    Property<float>& get_bgd_K() { return bgd_K; }

    Property<RGBColor>& get_pen_color() { return pen_color; }
    Property<RGBColor>& get_bgd_color() { return bgd_color; }

    Pen& get_pen() { return pen; }

    RawBuffer* get_rawbuf() { return rawbuf; }

    void init_buffer( unsigned int level );

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);

    void start_stroke()
    {
      start_stroke( pen_size.get(), pen_opacity.get() );
    }
    void start_stroke( unsigned int pen_size, float opacity );
    void end_stroke();

    void draw_point( unsigned int x, unsigned int y, VipsRect& update );
  };

  

  template < OP_TEMPLATE_DEF > 
  class DrawProc: public BlenderProc<OP_TEMPLATE_IMP>
  {
  };


  ProcessorBase* new_draw();
}

#endif 


