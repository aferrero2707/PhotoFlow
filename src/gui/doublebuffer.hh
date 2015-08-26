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

#ifndef DOUBLE_BUFFER_HH
#define DOUBLE_BUFFER_HH

#include <gtkmm.h>
#include <vips/vips.h>


namespace PF
{

class PixelBuffer
{
  Glib::RefPtr< Gdk::Pixbuf > buf;
  VipsRect rect;
  bool dirty;
public:

  static void pixbuf_cleanup(const guint8* data) { if( data ) delete [] data; }

  PixelBuffer(): dirty(true)
  {
    rect.left = rect.top = rect.width = rect.height = 0;
  }

  bool is_dirty() { return dirty; }
  void set_dirty( bool d )
  {
    std::cout<<"PixelBuffer::set_dirty("<<d<<") called."<<std::endl;
    dirty = d;
  }

  Glib::RefPtr< Gdk::Pixbuf > get_pxbuf() { return buf; }
  VipsRect& get_rect() { return rect; }

  void resize( const VipsRect& rect )
  {
    resize( rect.left, rect.top, rect.width, rect.height );
  }

  void resize(int x, int y, int w, int h)
  {
    if( !(buf) ||
        buf->get_width() != w ||
        buf->get_height() != h ) {
      // Set rowstride so that it is compatible with Cairo
      int stride = Cairo::ImageSurface::format_stride_for_width( Cairo::FORMAT_RGB24, w );
      guint8* data = new guint8[stride*h*3];
      if( !data ) return;
      buf = Gdk::Pixbuf::create_from_data( data, Gdk::COLORSPACE_RGB,
          false, 8, w, h, stride,
          &PixelBuffer::pixbuf_cleanup );
    }
    rect.left = x;
    rect.top = y;
    rect.width = w;
    rect.height = h;
  }

  void copy( PixelBuffer& src )
  {
    if( !buf || !(src.get_pxbuf()) )
      return;
    guint8* px1 = src.get_pxbuf()->get_pixels();
    int rs1 = src.get_pxbuf()->get_rowstride();
    int bl1 = 3; /*src.get_pxbuf()->get_byte_length();*/
    guint8* px2 = buf->get_pixels();
    int rs2 = buf->get_rowstride();
    int bl2 = 3; /*buf->get_byte_length();*/

    VipsRect src_rect=src.get_rect(), clip;
    vips_rect_intersectrect( &src_rect, &rect, &clip );
    int xstart = clip.left;
    int ystart = clip.top;
    //int xend = clip.left+clip.width-1;
    int yend = clip.top+clip.height-1;

    for( int y = ystart; y <= yend; y++ ) {
      int dy1 = y - src_rect.top;
      int dy2 = y - rect.top;

      int dx1 = xstart - src_rect.left;
      int dx2 = xstart - rect.left;

      guint8* p1 = px1 + rs1*dy1 + dx1*bl1;
      guint8* p2 = px2 + rs2*dy2 + dx2*bl2;

      memcpy( p2, p1, clip.width*bl2 );
    }
  }

  void copy( VipsRegion* region, VipsRect src_rect, int xoffs=0, int yoffs=0 )
  {
    guint8 *px1 = (guchar *) VIPS_REGION_ADDR( region, src_rect.left, src_rect.top );
    int rs1 = VIPS_REGION_LSKIP( region );
    int bl1 = 3; /*buf->get_byte_length();*/

    guint8* px2 = buf->get_pixels();
    int rs2 = buf->get_rowstride();
    int bl2 = 3; /*buf->get_byte_length();*/

#ifndef NDEBUG
    std::cout<<"PixelBuffer::copy()"<<std::endl;
    std::cout<<"  src_rect="<<src_rect.width<<"x"<<src_rect.height<<"+"<<src_rect.left<<"+"<<src_rect.top<<std::endl;
    std::cout<<"  xoffs="<<xoffs<<"  yoffs="<<yoffs<<std::endl;
    std::cout<<"  rect="<<rect.width<<"x"<<rect.height<<"+"<<rect.left<<"+"<<rect.top<<std::endl;
#endif

    // We add the offset of the image relative to the buffer to src_rect
    src_rect.left += xoffs;
    src_rect.top += yoffs;
    VipsRect clip;
    vips_rect_intersectrect (&src_rect, &rect, &clip);
    if( clip.width <= 0 ||
        clip.height <= 0 ) return;
    int xstart = clip.left;
    int ystart = clip.top;
    //int xend = clip.left+clip.width-1;
    int yend = clip.top+clip.height-1;

#ifndef NDEBUG
    std::cout<<"  src_rect(2)="<<src_rect.width<<"x"<<src_rect.height<<"+"<<src_rect.left<<"+"<<src_rect.top<<std::endl;
    std::cout<<"  clip="<<clip.width<<"x"<<clip.height<<"+"<<clip.left<<"+"<<clip.top<<std::endl;
    std::cout<<"  xstart="<<xstart<<"  ystart="<<ystart<<"  yend="<<yend<<std::endl;
#endif

    for( int y = ystart; y <= yend; y++ ) {
      int dy1 = y - src_rect.top;
      int dy2 = y - rect.top;

      int dx1 = xstart - src_rect.left;
      int dx2 = xstart - rect.left;

      guint8* p1 = px1 + rs1*dy1 + dx1*bl1;
      guint8* p2 = px2 + rs2*dy2 + dx2*bl2;

      memcpy( p2, p1, clip.width*bl2 );
    }
  }

  void fill( const VipsRect& area, guint8 val )
  {
    guint8* px2 = buf->get_pixels();
    int rs2 = buf->get_rowstride();
    int bl2 = 3; /*buf->get_byte_length();*/

    // We add the offset of the image relative to the buffer to src_rect
    VipsRect clip;
    vips_rect_intersectrect (&area, &rect, &clip);
    if( clip.width <= 0 ||
        clip.height <= 0 ) return;
    int xstart = clip.left;
    int ystart = clip.top;
    //int xend = clip.left+clip.width-1;
    int yend = clip.top+clip.height-1;
    int y; /*x*/;

    for( y = ystart; y <= yend; y++ ) {
      int dy2 = y - rect.top;

      int dx2 = xstart - rect.left;

      guint8* p2 = px2 + rs2*dy2 + dx2*bl2;

      memset( p2, val, clip.width*bl2 );
    }
  }

  void fill( const VipsRect& area, guint8 r, guint8 g, guint8 b )
  {
    guint8* px2 = buf->get_pixels();
    int rs2 = buf->get_rowstride();
    int bl2 = 3; /*buf->get_byte_length();*/

    // We add the offset of the image relative to the buffer to src_rect
    VipsRect clip;
    vips_rect_intersectrect (&area, &rect, &clip);
    if( clip.width <= 0 ||
        clip.height <= 0 ) return;
    int xstart = clip.left;
    int ystart = clip.top;
    //int xend = (clip.left+clip.width-1);
    int dx = clip.width*bl2;
    int yend = clip.top+clip.height-1;
    int x, y;

    for( y = ystart; y <= yend; y++ ) {
      int dy2 = y - rect.top;

      int dx2 = xstart - rect.left;

      guint8* p2 = px2 + rs2*dy2 + dx2*bl2;

      for( x = 0; x <= dx; x+=bl2 ) {
        p2[0] = r;
        p2[1] = g;
        p2[2] = b;
        p2 += bl2;
      }
      //memcpy( p2, p1, clip.width*bl2 );
    }
  }

  void draw_circle( int x0, int y0, int radius )
  {
#define PX_MOD( px ) { int _px = px; _px += 127; if(_px>255) _px -= 255; px = (guint8)_px; }

    guint8* px = get_pxbuf()->get_pixels();
    const int rs = get_pxbuf()->get_rowstride();
    const int bl = 3; /*buf->get_byte_length();*/

    int buf_left = get_rect().left;
    int buf_right = get_rect().left+get_rect().width-1;
    int buf_top = get_rect().top;
    int buf_bottom = get_rect().top+get_rect().height-1;

    int r2 = radius*radius;

    for( int y = 0; y <= radius; y++ ) {
      int row1 = y0 - y;
      int row2 = y0 + y;
      //int L = pen.get_size() - y;
      int D = (int)(sqrt( r2 - y*y )-0.00);
      int left = x0 - D;
      if( left < buf_left )
        left = buf_left;
      int right = x0 + D;
      if( right >= buf_right )
        right = buf_right;
      int colspan = (right + 1 - left)*3;

      int left2 = right+1;
      int right2 = left-1;
      if( y < radius ) {
        int D2 = (int)(sqrt( r2 - (y+1)*(y+1) )-0.00);
        left2 = x0 - D2;
        if( left2 < buf_left )
          left2 = buf_left;
        right2 = x0 + D2;
        if( right2 >= buf_right )
          right2 = buf_right;
      }


      //endcol = x0;

      /*
          std::cout<<"x0="<<x0<<"  y0="<<y0<<"  D="<<D<<std::endl;
          std::cout<<"row1="<<row1<<"  row2="<<row2<<"  startcol="<<startcol<<"  endcol="<<endcol<<"  colspan="<<colspan<<std::endl;
          std::cout<<"point_clip.left="<<point_clip.left<<"  point_clip.top="<<point_clip.top
                   <<"  point_clip.width="<<point_clip.width<<"  point_clip.height="<<point_clip.height<<std::endl;
       */
      /**/
      if( (row1 >= buf_top) && (row1 <= buf_bottom) ) {
        guint8* p = px + rs*(row1-buf_top) + (left-buf_left)*bl;
        if( left2 <= right ) {
          for( int x = left; x <= left2; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
          p = px + rs*(row1-buf_top) + (right2+1-buf_left)*bl;
          for( int x = right2; x <= right; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
        } else {
          for( int x = left; x <= right; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
        }
      }
      if( (row2 != row1) && (row2 >= buf_top) && (row2 <= buf_bottom) ) {
        guint8* p = px + rs*(row2-buf_top) + (left-buf_left)*bl;
        if( left2 <= right ) {
          for( int x = left; x <= left2; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
          p = px + rs*(row2-buf_top) + (right2+1-buf_left)*bl;
          for( int x = right2; x <= right; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
        } else {
          for( int x = left; x <= right; x++, p += bl ) {
            PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
          }
        }
      }
    }
  }
};



class DoubleBuffer
{
  PixelBuffer buf[2];
  int active_id;

  GMutex* mutex;
public:
  DoubleBuffer(): active_id(0)
  {
    mutex = vips_g_mutex_new();
  }

  PixelBuffer& get_active() { return buf[active_id]; }
  PixelBuffer& get_inactive() { return buf[1-active_id]; }

  void swap() { active_id = 1-active_id; }

  void resize_active(int x, int y, int w, int h)
  {
    get_active().resize( x, y, w, h );
  }

  void copy_buffers()
  {
    get_active().copy( get_inactive() );
  }

  void lock()
  {
    g_mutex_lock( mutex );
  }

  void unlock()
  {
    g_mutex_unlock( mutex );
  }
};

}


#endif
