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


void PF::PixelBuffer::copy( VipsRegion* region, VipsRect src_rect, int xoffs, int yoffs )
{
  //float *px1 = (float *) VIPS_REGION_ADDR( region, src_rect.left, src_rect.top );
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

  //std::cout<<"clip.width*bl2="<<clip.width*bl2<<"  rs1="<<rs1<<std::endl;
  for( int y = ystart; y <= yend; y++ ) {
    int dy1 = y - src_rect.top;
    int dy2 = y - rect.top;

    int dx1 = xstart - src_rect.left;
    int dx2 = xstart - rect.left;

    //guint8* p1 = px1 + rs1*dy1 + dx1*bl1;
    //std::cout<<"VIPS_REGION_ADDR( region, "<<xstart<<", "<<y<<" );"<<std::endl;
    float* p1 = (float *) VIPS_REGION_ADDR( region, xstart, y );
    guint8* p2 = px2 + rs2*dy2 + dx2*bl2;
    for( int x = 0; x < clip.width*bl2; x++ ) {
      if(p1[x] >= 1) p2[x] = 255;
      else if(p1[x] <= 0) p2[x] = 0;
      else p2[x] = static_cast<guint8>(p1[x]*255);
    }
    //std::cout<<"y="<<y<<",  memcpy( "<<(void*)p2<<", "<<(void*)p1<<", "<<clip.width*bl2<<" );"<<std::endl;
    //memcpy( p2, p1, clip.width*bl2 );
  }
}



void PF::PixelBuffer::draw_point( int x, int y, PixelBuffer& inbuf )
{
  guint8* inpx = inbuf.get_pxbuf()->get_pixels();
  guint8* px = get_pxbuf()->get_pixels();
  const int rs = get_pxbuf()->get_rowstride();
  const int bl = 3; /*buf->get_byte_length();*/

  int buf_left = get_rect().left;
  int buf_right = get_rect().left+get_rect().width-1;
  int buf_top = get_rect().top;
  int buf_bottom = get_rect().top+get_rect().height-1;

  if( (x>=buf_left) && (x<=buf_right) &&
      (y>=buf_top) && (y<=buf_bottom) ) {
    guint8* inp = inpx + rs*(y-buf_top) + (x-buf_left)*bl;
    guint8* p = px + rs*(y-buf_top) + (x-buf_left)*bl;
    PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
  }
}



void PF::PixelBuffer::fill( const VipsRect& area, PixelBuffer& inbuf )
{
  guint8* inpx = inbuf.get_pxbuf()->get_pixels();
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
      PX_MOD( inpx, p2 );
      //PX_MOD( inpx[1], p2[1] );
      //PX_MOD( inpx[2], p2[2] );
      inpx += bl2;
      p2 += bl2;
    }
    //memcpy( p2, p1, clip.width*bl2 );
  }
}


void PF::PixelBuffer::draw_circle( int x0, int y0, int radius, guint8 r, guint8 g, guint8 b, bool filled )
{

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
      if( left2 <= right && (!filled) ) {
        for( int x = left; x <= left2; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
        p = px + rs*(row1-buf_top) + (right2+1-buf_left)*bl;
        for( int x = right2; x <= right; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
      } else {
        for( int x = left; x <= right; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
      }
    }
    if( (row2 != row1) && (row2 >= buf_top) && (row2 <= buf_bottom) ) {
      guint8* p = px + rs*(row2-buf_top) + (left-buf_left)*bl;
      if( left2 <= right && (!filled) ) {
        for( int x = left; x <= left2; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
        p = px + rs*(row2-buf_top) + (right2+1-buf_left)*bl;
        for( int x = right2; x <= right; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
      } else {
        for( int x = left; x <= right; x++, p += bl ) {
          p[0] = r; p[1] = g; p[2] = b;
        }
      }
    }
  }
}


void PF::PixelBuffer::draw_circle( int x0, int y0, int radius, PixelBuffer& inbuf )
{
  guint8* inpx = inbuf.get_pxbuf()->get_pixels();
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
      guint8* inp = inpx + rs*(row1-buf_top) + (left-buf_left)*bl;
      guint8* p = px + rs*(row1-buf_top) + (left-buf_left)*bl;
      if( left2 <= right ) {
        for( int x = left; x <= left2; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
        inp = inpx + rs*(row1-buf_top) + (right2+1-buf_left)*bl;
        p = px + rs*(row1-buf_top) + (right2+1-buf_left)*bl;
        for( int x = right2; x <= right; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
      } else {
        for( int x = left; x <= right; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
      }
    }
    if( (row2 != row1) && (row2 >= buf_top) && (row2 <= buf_bottom) ) {
      guint8* inp = inpx + rs*(row2-buf_top) + (left-buf_left)*bl;
      guint8* p = px + rs*(row2-buf_top) + (left-buf_left)*bl;
      if( left2 <= right ) {
        for( int x = left; x <= left2; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
        inp = inpx + rs*(row2-buf_top) + (right2+1-buf_left)*bl;
        p = px + rs*(row2-buf_top) + (right2+1-buf_left)*bl;
        for( int x = right2; x <= right; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); // PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
      } else {
        for( int x = left; x <= right; x++, inp += bl, p += bl ) {
          PX_MOD( inp, p ); //PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        }
      }
    }
  }
}



void PF::PixelBuffer::draw_line( int x1, int y1, int x2, int y2, PixelBuffer& inbuf )
{
  guint8* inpx = inbuf.get_pxbuf()->get_pixels();
  guint8* px = get_pxbuf()->get_pixels();
  const int rs = get_pxbuf()->get_rowstride();
  const int bl = 3; /*buf->get_byte_length();*/

  int buf_left = get_rect().left;
  int buf_right = get_rect().left+get_rect().width-1;
  int buf_top = get_rect().top;
  int buf_bottom = get_rect().top+get_rect().height-1;

  int dx = x2-x1;
  int dy = y2-y1;

  int x1_ = x1, x2_ = x2, y1_ = y1, y2_ = y2;

  if( (dx==0) && (dy==0) ) {
    return;
    if( (x1>=buf_left) && (x1<=buf_right) &&
        (y1>=buf_top) && (y1<=buf_bottom) ) {
      guint8* inp = inpx + rs*(y1-buf_top) + (x1-buf_left)*bl;
      guint8* p = px + rs*(y1-buf_top) + (x1-buf_left)*bl;
      PX_MOD( inp, p ); //PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
      //p[0] = 255; p[1] = p[2] = 255;
    }
    return;
  }

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


      guint8* inp = inpx + rs*(y1-buf_top) + (xstart-buf_left)*bl;
      guint8* p = px + rs*(y1-buf_top) + (xstart-buf_left)*bl;
      for( int x = xstart; x <= xend; x++, inp += bl, p += bl ) {
        //if( x!=x2_ )
        PX_MOD( inp, p ); //PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
        //std::cout<<"draw_line(): in="<<(int)inp[0]<<","<<(int)inp[1]<<","<<(int)inp[2]<<"  p="<<(int)p[0]<<","<<(int)p[1]<<","<<(int)p[2]<<std::endl;
        //p[0] = 255; p[1] = p[2] = 255;
      }
    } else {
      if( y1 > y2 ) {
        int tmp = y2; y2 = y1; y1 = tmp;
        dy = y2-y1;

        tmp = x2; x2 = x1; x1 = tmp;
        dx = x2-x1;
      }

      for( int y = y1; y <= y2; y++ ) {

        if( y < buf_top ) continue;
        if( y > buf_bottom ) continue;

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

        guint8* inp = inpx + rs*(y-buf_top) + (xstart-buf_left)*bl;
        guint8* p = px + rs*(y-buf_top) + (xstart-buf_left)*bl;
        for( int x = xstart; x <= xend; x++, inp += bl, p += bl ) {
          //if( x!=x2_ && y!=y2_ )
          PX_MOD( inp, p ); //PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
          //p[0] = 255; p[1] = p[2] = 255;
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

      if( y < buf_top ) continue;
      if( y > buf_bottom ) continue;

      int x = x1 + (y-y1)*dx/dy;
      if( x < buf_left ) continue;
      if( x > buf_right ) continue;
      guint8* inp = inpx + rs*(y-buf_top) + (x-buf_left)*bl;
      guint8* p = px + rs*(y-buf_top) + (x-buf_left)*bl;
      //if( x!=x2_ && y!=y2_ )
      PX_MOD( inp, p ); //PX_MOD( inp[1], p[1] ); PX_MOD( inp[2], p[2] );
      //p[0] = 255; p[1] = p[2] = 255;
    }
  }
}



void PF::PixelBuffer::draw_line( int x1, int y1, int x2, int y2, guint8 val )
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

  int x1_ = x1, x2_ = x2, y1_ = y1, y2_ = y2;

  if( (dx==0) && (dy==0) ) {
    return;
    if( (x1>=buf_left) && (x1<=buf_right) &&
        (y1>=buf_top) && (y1<=buf_bottom) ) {
      guint8* p = px + rs*(y1-buf_top) + (x1-buf_left)*bl;
      p[0] = p[1] = p[2] = val;
    }
    return;
  }

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
        //if( x!=x2_ )
        //std::cout<<"draw_line(): in="<<(int)inp[0]<<","<<(int)inp[1]<<","<<(int)inp[2]<<"  p="<<(int)p[0]<<","<<(int)p[1]<<","<<(int)p[2]<<std::endl;
        p[0] = p[1] = p[2] = val;
      }
    } else {
      if( y1 > y2 ) {
        int tmp = y2; y2 = y1; y1 = tmp;
        dy = y2-y1;

        tmp = x2; x2 = x1; x1 = tmp;
        dx = x2-x1;
      }

      for( int y = y1; y <= y2; y++ ) {

        if( y < buf_top ) continue;
        if( y > buf_bottom ) continue;

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
          //if( x!=x2_ && y!=y2_ )
          p[0] = p[1] = p[2] = val;
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

      if( y < buf_top ) continue;
      if( y > buf_bottom ) continue;

      int x = x1 + (y-y1)*dx/dy;
      if( x < buf_left ) continue;
      if( x > buf_right ) continue;
      guint8* p = px + rs*(y-buf_top) + (x-buf_left)*bl;
      //if( x!=x2_ && y!=y2_ )
      p[0] = p[1] = p[2] = val;
    }
  }
}
