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

    Glib::RefPtr< Gdk::Pixbuf > get_pxbuf() { return buf; }
    VipsRect get_rect() { return rect; }

    void resize(int x, int y, int w, int h)
    {
      if( !(buf) ||
					buf->get_width() != w ||
					buf->get_height() != h )
				buf = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB,
																	 false, 8, w, h );
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

    void copy( VipsRegion* region, const VipsRect& src_rect )
    {
      guint8 *px1 = (guchar *) VIPS_REGION_ADDR( region, src_rect.left, src_rect.top );
      int rs1 = VIPS_REGION_LSKIP( region );
      int bl1 = 3; /*buf->get_byte_length();*/

      guint8* px2 = buf->get_pixels();
      int rs2 = buf->get_rowstride();
      int bl2 = 3; /*buf->get_byte_length();*/

      VipsRect clip;
      vips_rect_intersectrect (&src_rect, &rect, &clip);
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
