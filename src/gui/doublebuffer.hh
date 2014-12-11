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
  public:

    static void pixbuf_cleanup(const guint8* data) { if( data ) delete [] data; }

    Glib::RefPtr< Gdk::Pixbuf > get_pxbuf() { return buf; }
    VipsRect get_rect() { return rect; }

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
      int xend = clip.left+clip.width-1;
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

			// We add the offset of the image relative to the buffer to src_rect
			src_rect.left += xoffs;
			src_rect.top += yoffs;
			VipsRect clip;
      vips_rect_intersectrect (&src_rect, &rect, &clip);
			if( clip.width <= 0 ||
					clip.height <= 0 ) return;
      int xstart = clip.left;
      int ystart = clip.top;
      int xend = clip.left+clip.width-1;
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
      int xend = clip.left+clip.width-1;
      int yend = clip.top+clip.height-1;
			int x, y;

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
      int xend = (clip.left+clip.width-1);
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
