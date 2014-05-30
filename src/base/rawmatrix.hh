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

#ifndef PF_RAW_MATRIX_H
#define PF_RAW_MATRIX_H

#include "array2d.hh"

namespace PF {


  struct RawPixel
  {
    float data;
    unsigned char color;
    //float color;
  };


  class RawMatrixRow
  {
    RawPixel* pixels;
  public:
    RawMatrixRow( RawPixel* px ): pixels ( px )
    {
      //std::cout<<"RawMatrixRow::RawMatrixRow(): pixels="<<pixels<<std::endl;
    }

    RawMatrixRow( const RawMatrixRow& row ): pixels( row.get_pixels() )
    {
      //std::cout<<"RawMatrixRow::RawMatrixRow( const RawMatrixRow& row ): pixels="<<pixels<<std::endl;
    }

    RawPixel* get_pixels() const { return pixels; }

    float operator[](int c) {
      //std::cout<<"RawMatrixRow::operator[]: pixels="<<pixels<<std::endl;
      return pixels[c].data;
    }

    unsigned short int color(int c) {
      return pixels[c].color;
    }
  };


  class RawMatrix
  {
    unsigned int width, height;
    unsigned int r_offset, c_offset;
    RawPixel **buf;
    RawPixel **rows;

  public:
    RawMatrix(): 
      width( 0 ), height( 0 ),
      r_offset( 0 ), c_offset( 0 ),
      buf( NULL ), rows( NULL ) 
    {
    }
    ~RawMatrix() {}

    unsigned int GetWidth() { return width; }
    unsigned int GetHeight() { return height; }

    void init(unsigned int w, unsigned int h, unsigned int r_offs, unsigned int c_offs)
    {
      width = w;
      height = h;
      r_offset = r_offs;
      c_offset = c_offs;
      buf = (RawPixel**)realloc( buf, sizeof(RawPixel*)*height );
      rows = buf - r_offset;
      for( unsigned int i = 0; i < height; i++ )
	buf[i] = NULL;
    }


    void set_row( unsigned int row, RawPixel* ptr )
    {
      rows[row] = ptr - c_offset;
      //std::cout<<"RawMatrix::set_row("<<row<<","<<(void*)ptr<<": rows["<<row<<"]="<<rows[row]<<"  c_offset="<<c_offset<<std::endl;
    }

    RawMatrixRow operator[](int r) {
      //std::cout<<"RawMatrix::operator[]: rows["<<r<<"]="<<rows[r]<<"  r_offset="<<r_offset<<std::endl;
      return( RawMatrixRow(rows[r]) );
    }
  };

}

#endif
