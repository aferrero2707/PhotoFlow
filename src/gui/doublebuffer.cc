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

#include <iostream>

#include "doublebuffer.hh"


void PF::PixelBuffer::draw_line( int x1, int y1, int x2, int y2 )
{
  guint8* px = get_pxbuf()->get_pixels();
  const int rs = get_pxbuf()->get_rowstride();
  const int bl = 3; /*buf->get_byte_length();*/

  int buf_left = get_rect().left;
  int buf_right = get_rect().left+get_rect().width-1;
  int buf_top = get_rect().top;
  int buf_bottom = get_rect().top+get_rect().height-1;

  int dx = x2-x1;
  int dy = y2-y1;

  int xmin = (x1<=x2) ? x1 : x2;
  int xmax = (x1>x2) ? x1 : x2;

  if( abs(dx) > abs(dy) ) {
    // Line is more horizontal than vertical
    int xstart = -1, xend = -1;
    if( dy==0 ) {

      if( y1 < buf_top ) return;
      if( y1 > buf_bottom ) return;

      int xstart = xmin;
      if( xstart > buf_right ) return;
      if( xstart < buf_left ) xstart = buf_left;

      int xend = xmax;
      if( xend < buf_left ) return;
      if( xend > buf_right ) xend = buf_right;


      guint8* p = px + rs*(y1-buf_top) + (xstart-buf_left)*bl;
      for( int x = xstart; x <= xend; x++, p += bl ) {
        PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
      }
    } else {
      if( y1 > y2 ) {
        int tmp = y2; y2 = y1; y1 = tmp;
        dy = y2-y1;

        tmp = x2; x2 = x1; x1 = tmp;
        dx = x2-x1;
      }

      for( int y = y1; y <= y2; y++ ) {

        int x1_ = x1 + (y-y1)*dx/dy;
        int x2_ = x1 + (y+1-y1)*dx/dy;
        //std::cout<<"x1="<<x1<<" y1="<<y1<<" x2="<<x2<<" y2="<<y2<<"    y="<<y<<" x1_="<<x1_<<" x2_="<<x2_<<std::endl;
        int xstart = (x1_<=x2_) ? x1_ : x2_;
        int xend = (x1_>x2_) ? x1_ : x2_;
        xend -= 1;

        if( xstart > buf_right ) continue;
        if( xstart < buf_left ) xstart = buf_left;
        if( xstart < xmin ) xstart = xmin;

        if( xend < buf_left ) continue;
        if( xend > buf_right ) xend = buf_right;
        if( xend > xmax ) xend = xmax;

        guint8* p = px + rs*(y-buf_top) + (xstart-buf_left)*bl;
        for( int x = xstart; x <= xend; x++, p += bl ) {
          PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
        }
      }
    }
  } else {
    // Line is more vertical than horizontal, so for each y value
    // there is a single pixel to be drawn
    if( y1 > y2 ) {
      int tmp = y2; y2 = y1; y1 = tmp;
      dy = y2-y1;

      tmp = x2; x2 = x1; x1 = tmp;
      dx = x2-x1;
    }

    for( int y = y1; y <= y2; y++ ) {

      int x = x1 + (y-y1)*dx/dy;
      guint8* p = px + rs*(y-buf_top) + (x-buf_left)*bl;
      PX_MOD( p[0] ); PX_MOD( p[1] ); PX_MOD( p[2] );
    }
  }
}
