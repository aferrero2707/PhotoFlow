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

#include <string.h>
#include <iostream>

#include "../base/array2d.hh"
#include "../base/format_info.hh"
#include "../base/operation.hh"
#include "../base/processor.hh"
#include "../base/rawbuffer.hh"

#include "diskbuffer.hh"
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

  inline bool operator==(const RGBColor& lhs, const RGBColor& rhs)
  {
    return( (lhs.r==rhs.r) && (lhs.g==rhs.g) && (lhs.b==rhs.b) );
  }

  inline bool operator!=(const RGBColor& lhs, const RGBColor& rhs)
  {
    return( !(lhs == rhs) );
  }

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



  class DrawPar: public OpParBase
  {
    Property<float> pen_grey, pen_R, pen_G, pen_B, pen_L, pen_a, pen_b, pen_C, pen_M, pen_Y, pen_K;
    Property<float> bgd_grey, bgd_R, bgd_G, bgd_B, bgd_L, bgd_a, bgd_b, bgd_C, bgd_M, bgd_Y, bgd_K;
    Property<RGBColor> pen_color;
    Property<RGBColor> bgd_color;
    Property<int> pen_size;
    Property<float> pen_opacity;
    Property<float> pen_smoothness;
    Property< std::list< Stroke<Pencil> > > strokes;

		ProcessorBase* diskbuf;
    RawBuffer* rawbuf;

    unsigned int scale_factor;

    Pencil pen;

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

    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    Pencil& get_pen() { return pen; }

    RawBuffer* get_rawbuf() { return rawbuf; }

    void init_buffer( unsigned int level );

    unsigned int get_scale_factor() { return scale_factor; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);

    void start_stroke()
    {
      start_stroke( pen_size.get(), pen_opacity.get(), pen_smoothness.get() );
    }
    void start_stroke( unsigned int pen_size, float opacity, float smoothness );
    void end_stroke();

    Property< std::list< Stroke<Pencil> > >& get_strokes() { return strokes; }

    void draw_point( int x, int y, VipsRect& update );
  };

  

  template < OP_TEMPLATE_DEF > 
  class DrawProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, DrawPar* par)
    {
      //std::cout<<"DrawProc::render() called"<<std::endl;
      DrawPar* opar = par;//dynamic_cast<DrawPar*>(par);
      if( !opar ) return;
      std::list< Stroke<Pencil> >& strokes = opar->get_strokes().get();
      VipsRect *r = &oreg->valid;
      //std::cout<<"nbands: "<<oreg->im->Bands<<std::endl;
      //std::cout<<"r->left="<<r->left<<"  r->top="<<r->top
      //         <<"  r->width="<<r->width<<"  r->height="<<r->height<<std::endl;
      int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands; 
      //int width = r->width;
      //int height = r->height;

      //T* p;
      //T* pin;
      T* pout;
      T* ptemp;
      int x, x0, y, y0, ch, row1, row2;
      int point_clip_right, point_clip_bottom;

      T** temp = new T*[r->height];
      for( y = 0; y < r->height; y++) temp[y] = new T[line_size];

      pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top ); 
      //std::cout<<"pout="<<(void*)pout<<std::endl;

      T val[16];
      //std::cout<<"DrawProc: bgd color: "<<opar->get_bgd_color().get().r<<" "
      //    <<opar->get_bgd_color().get().g<<" "<<opar->get_bgd_color().get().b<<std::endl;
      val[0] = (T)(opar->get_bgd_color().get().r*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      val[1] = (T)(opar->get_bgd_color().get().g*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      val[2] = (T)(opar->get_bgd_color().get().b*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      //val[0] = (T)(1*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      //val[1] = (T)(0*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      //val[2] = (T)(0*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
      for( y = 0; y < r->height; y++ ) {
        //p = (T*)VIPS_REGION_ADDR( ireg[in_first], point_clip.left, point_clip.top + y ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y ); 
        for( x = 0; x < line_size; x += oreg->im->Bands ) {
          for( ch = 0; ch < oreg->im->Bands; ch++ ) {
            pout[x+ch] = val[ch];
          }
        }
      }

      PF::Array2D<float> opacity_max;
      opacity_max.Init( r->width, r->height, r->top, r->left );

      std::list< Stroke<Pencil> >::iterator si;
      std::list< std::pair<int, int> >::iterator pi;
      VipsRect point_area;
      VipsRect point_clip;
      VipsRect stroke_area;
      VipsRect stroke_clip;
      //std::cout<<"DrawProc::render(): strokes.size()="<<strokes.size()<<std::endl;
      int sn = 0;
      for( si = strokes.begin(); si != strokes.end(); ++si, sn++ ) {
        //std::cout<<"stroke area: "<<si->get_area().width<<","<<si->get_area().height
        //    <<"+"<<si->get_area().left<<"+"<<si->get_area().top<<std::endl;
        stroke_area.left = si->get_area().left/static_cast<int>(opar->get_scale_factor());
        stroke_area.top = si->get_area().top/static_cast<int>(opar->get_scale_factor());
        stroke_area.width = si->get_area().width/static_cast<int>(opar->get_scale_factor());
        stroke_area.height = si->get_area().height/static_cast<int>(opar->get_scale_factor());
        //std::cout<<"stroke area: "<<stroke_area.width<<","<<stroke_area.height
        //    <<"+"<<stroke_area.left<<"+"<<stroke_area.top<<std::endl;
        vips_rect_intersectrect( r, &stroke_area, &stroke_clip );
        //std::cout<<"stroke clip: "<<stroke_clip.width<<","<<stroke_clip.height
        //    <<"+"<<stroke_clip.left<<"+"<<stroke_clip.top<<std::endl;
        if( (stroke_clip.width<1) || (stroke_clip.height<1) ) continue;
        // copy current region to temp buffer
        if( temp ) {
          for( y = 0; y < r->height; y++ ) {
            //p = (T*)VIPS_REGION_ADDR( ireg[in_first], point_clip.left, point_clip.top + y );
            pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
            memcpy( temp[y], pout, line_size*sizeof(T) );
          }
        }

        Pencil& pen = si->get_pen();
        //std::cout<<"  pen color: "<<pen.get_channel(0)<<std::endl;
        int pen_size = (int)(pen.get_size()/opar->get_scale_factor());
        int pen_size2 = pen_size*pen_size;
        for( ch = 0; ch < oreg->im->Bands; ch++ ) {
          val[ch] = (T)(pen.get_channel(ch)*FormatInfo<T>::RANGE + FormatInfo<T>::MIN);
        }

        for( int ic = 0; ic < r->width; ic++ ) {
          for( int ir = 0; ir < r->height; ir++ ) {
            opacity_max.GetLocal(ir, ic) = 0;
          }
        }

        point_area.width = point_area.height = pen_size*2 + 1;
        PF::PencilMask* resized_mask = NULL;
        PF::PencilMask* mask = &(pen.get_mask());
        if( pen_size != pen.get_size() ) {
          resized_mask = new PF::PencilMask;
          resized_mask->init( point_area.width, pen.get_opacity(), pen.get_smoothness() );
          mask = resized_mask;
        }

        std::list< std::pair<int, int> >& points = si->get_points();
        //std::cout<<"DrawProc::render(): points.size()="<<points.size()<<std::endl;
        int pn = 0;
        for( pi = points.begin(); pi != points.end(); ++pi, pn++ ) {
          //std::cout<<"Drawing point "<<pi->first<<","<<pi->second<<std::endl;
          point_area.left = pi->first/static_cast<int>(opar->get_scale_factor()) - pen_size;
          point_area.top = pi->second/static_cast<int>(opar->get_scale_factor()) - pen_size;
          //std::cout<<"Point area: "<<point_area.width<<","<<point_area.height
          //    <<"+"<<point_area.left<<"+"<<point_area.top<<std::endl;
          vips_rect_intersectrect( r, &point_area, &point_clip );
          //std::cout<<"Point clip: "<<point_clip.width<<","<<point_clip.height
          //    <<"+"<<point_clip.left<<"+"<<point_clip.top<<std::endl;
          if( (point_clip.width<1) || (point_clip.height<1) ) continue;
          point_clip_right = point_clip.left + point_clip.width - 1;
          point_clip_bottom = point_clip.top + point_clip.height - 1;

          x0 = pi->first/opar->get_scale_factor();
          y0 = pi->second/opar->get_scale_factor();
          for( y = 0; y < point_area.height; y++ ) {
            int row = y + point_area.top;
            //std::cout<<"  row="<<row<<std::endl;
            if( row < point_clip.top ) continue;
            if( row > point_clip_bottom ) break;
            pout = (T*)VIPS_REGION_ADDR( oreg, point_area.left, row );
            ptemp = &(temp[point_area.top-r->top+y][(point_area.left-r->left)*oreg->im->Bands]);
            for( x = 0; x < point_area.width; x++, pout += oreg->im->Bands, ptemp += oreg->im->Bands ) {
              int col = x+point_area.left;
              float mval = mask->get( x, y );
              //mval = mval/2+0.5;
              //std::cout<<"    opacity_max.Get("<<row<<", "<<col<<")="<<opacity_max.Get(row, col)<<"    mval="<<mval<<std::endl;
              if( (col < point_clip.left) ) continue;
              if( (col > point_clip_right) ) break;
              if( mval < 0.00001 ) continue;
              if( mval <= opacity_max.Get(row, col) ) continue;
              opacity_max.Get(row, col) = mval;
              for( ch = 0; ch < oreg->im->Bands; ch++ ) {
                float out = mval*val[ch] + (1.0f-mval)*pout[ch];
                //std::cout<<"    pout["<<ch<<"]= "<<pout[ch]<<"  out["<<ch<<"]="<<out<<std::endl;
                //if( pn == 0 )
                  ptemp[ch] = static_cast<T>(out);
              }
            }
            //if( pn > 0 && y > 1 ) break;
          }
        }

        // copy temp buffer back to current region
        if( temp ) {
          for( y = 0; y < r->height; y++ ) {
            //p = (T*)VIPS_REGION_ADDR( ireg[in_first], point_clip.left, point_clip.top + y );
            pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );
            memcpy( pout, temp[y], line_size*sizeof(T) );
          }
        }
      }

      if( temp ) {
        for( y = 0; y < r->height; y++) delete[] temp[y];
        delete[] temp;
      }
    }
  };


  ProcessorBase* new_draw();
}

#endif 



