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

#ifndef PF_ARRAY_2D_H
#define PF_ARRAY_2D_H

#include <stdlib.h>
#include <string>
#include <iostream>

//#define ARRAY2D_DEBUG 1
//#include "pixelmatrix.hh"

namespace PF {


  template<class T>
  class Array2D
  {
    unsigned int width, height;
    // Offset of the start of the image data
    unsigned int r_offset, c_offset;

    unsigned int size_allocated;

  public:
    T* buf;
    T** matrix;
    // The matrix shifted according to the configured offset
    // The () operator uses this one to return the pixel values
    T** matrix_shifted;

  public:
    Array2D();
    ~Array2D();

    unsigned int GetWidth() { return width; }
    unsigned int GetHeight() { return height; }
    unsigned int GetRowOffset() { return r_offset; }
    unsigned int GetColOffset() { return c_offset; }

    void SetWidth(unsigned int w) { width = w; }
    void SetHeight(unsigned int h) { height = h; }
    void SetRowOffset(unsigned int offs) { r_offset = offs; }
    void SetColOffset(unsigned int offs) { c_offset = offs; }

    bool Init(unsigned int w, unsigned int h, unsigned int r_offset, unsigned int c_offset);
    void Resize(unsigned int width, unsigned int height);
    void SetRowColOffset(unsigned int roffs, unsigned int coffs);
    void SetXYOffset(unsigned int xoffs, unsigned int yoffs);
    void Reset();

    T* GetBuffer() { return buf; }
    T& Get(unsigned int r, unsigned int c);
    T& GetLocal(unsigned int r, unsigned int c);

    // use with indices
    T* operator[](int r) {
      T* ptr = matrix_shifted[r] + c_offset;
      T* ptr2 = matrix[r-r_offset];
      //std::cout<<"r="<<r<<"  r_offset="<<r_offset<<"  ptr="<<ptr<<"  ptr2="<<ptr2<<std::endl;
      return matrix_shifted[r];
    }


    // use as pointer to data
    operator T*()
    {
        // only if owner this will return a valid pointer
        return buf;
    }


  };


  template<class T>
  Array2D<T>::Array2D()
  {
    buf = 0;
    matrix = 0;
    size_allocated = 0;
    width = height = r_offset = c_offset = 0;
  }


  template<class T>
  Array2D<T>::~Array2D()
  {
#ifdef ARRAY2D_DEBUG
    std::cout<<"Array2D<T>::~Array2D(): matrix="<<(void*)matrix<<"  buf="<<(void*)buf<<std::endl;
#endif
    if( matrix ) {
      free( matrix );
#ifdef ARRAY2D_DEBUG
      std::cout<<"Array2D<T>::~Array2D(): matrix deallocated"<<std::endl;
#endif
    }
    if( buf ) {
      free( buf );
#ifdef ARRAY2D_DEBUG
      std::cout<<"Array2D<T>::~Array2D(): buffer deallocated"<<std::endl;
#endif
    }
  }


  template<class T>
  void Array2D<T>::Reset()
  {
#ifdef ARRAY2D_DEBUG
    std::cout<<"Array2D<T>::~Array2D(): matrix="<<(void*)matrix<<"  buf="<<(void*)buf<<std::endl;
#endif
    if( matrix ) {
      free( matrix );
      matrix = NULL;
#ifdef ARRAY2D_DEBUG
      std::cout<<"Array2D<T>::~Array2D(): matrix deallocated"<<std::endl;
#endif
    }
    if( buf ) {
      free( buf );
      buf = NULL;
#ifdef ARRAY2D_DEBUG
      std::cout<<"Array2D<T>::~Array2D(): buffer deallocated"<<std::endl;
#endif
    }
  }


  template<class T>
  bool Array2D<T>::Init(unsigned int w, unsigned int h, unsigned int r_offset, unsigned int c_offset)
  {
    if(buf && GetWidth()==w && GetHeight()==h && 
       r_offset == GetRowOffset() && c_offset == GetColOffset()) 
      return false;

    /*
      Main buffer initialization
    */
    // Step 1: check if global buffer needs to be (re)allocated
    unsigned int offs = c_offset;
    unsigned int size_new = sizeof(T)*w*h;
#ifdef ARRAY2D_DEBUG
    std::cout<<"Array2D<T>::Init("<<w<<","<<h<<","<<r_offset<<","<<c_offset<<"): size_allocated="
	     <<size_allocated<<"  size_new="<<size_new<<std::endl;
#endif
    if(size_new > size_allocated) {
#ifdef ARRAY2D_DEBUG
      std::cout<<"Array2D<T>::Init("<<w<<","<<h<<","<<r_offset<<","<<c_offset<<"): old buf="<<buf;
#endif
      //Array2DManager::Ref().Allocate(size_new);
      buf = (T*)realloc(buf,size_new);
      size_allocated = size_new;
#ifdef ARRAY2D_DEBUG
      std::cout<<"  new buf="<<buf<<std::endl;
#endif
#ifdef ARRAY2D_DEBUG
      std::cout<<"  reallocated buffer"<<std::endl;
#endif
    }

    /*
      Array2D initialization
    */
    // Step 2: check if the row vector of the pixel matrix needs to be (re)allocated
    if(GetHeight() != h) {
      matrix = (T**)realloc(matrix, sizeof(T*)*h*2);
#ifdef ARRAY2D_DEBUG
      std::cout<<"  matrix reallocated."<<std::endl;
#endif
    }

    // Step 3: check if the row pointers need to be (re)assigned
    if(GetWidth() != w || GetHeight() != h) {
      for(int j = 0; j < (int)h; j++) {
	matrix[j] = &(buf[j*w]);
#ifdef ARRAY2D_DEBUG
	//std::cout<<"  matrix["<<j<<"] = &(buf["<<j<<"*"<<w<<"])"
	//	       <<std::endl;
#endif
      }
    }


#ifdef ARRAY2D_DEBUG
    std::cout<<"  r_offset="<<r_offset<<"  c_offset="<<c_offset<<std::endl;
    std::cout<<"  GetRowOffset()="<<GetRowOffset()<<"  GetColOffset()="<<GetColOffset()<<std::endl;
#endif
    // Step 4: check if the shifted matrix needs to be (re)initialized
    if(GetHeight() != h || r_offset != GetRowOffset()) {
      matrix_shifted = (&(matrix[h])) - r_offset;
      //matrix_shifted = matrix - r_offset;
#ifdef ARRAY2D_DEBUG
      std::cout<<"  matrix_shifted updated"<<std::endl;
#endif
    }
    if(GetHeight() != h || c_offset != GetColOffset()) {
      for(int j = 0; j < (int)h; j++) {
	matrix_shifted[j+r_offset] = matrix[j] - c_offset;
#ifdef ARRAY2D_DEBUG
	//std::cout<<"  matrix_shifted["<<j+r_offset<<"] = matrix["<<j<<"-"<<c_offset<<std::endl;
#endif
      }
#ifdef ARRAY2D_DEBUG
      std::cout<<"  matrix_shifted rows updated"<<std::endl;
#endif
    }


    SetWidth(w);
    SetHeight(h);
    SetRowOffset(r_offset);
    SetColOffset(c_offset);
#ifdef ARRAY2D_DEBUG
    std::cout<<"Array2D initialization ended."
	     <<std::endl;
#endif
    return true;
  }


  template<class T>
  void Array2D<T>::Resize(unsigned int w, unsigned int h)
  {
    Init(w,h,GetRowOffset(),GetColOffset());
  }


  template<class T>
  void Array2D<T>::SetRowColOffset(unsigned int roffs, unsigned int coffs)
  {
    Init(GetWidth(),GetHeight(),roffs,coffs);
  }


  template<class T>
  void Array2D<T>::SetXYOffset(unsigned int xoffs, unsigned int yoffs)
  {
    Init(GetWidth(),GetHeight(),yoffs,xoffs);
  }


  template<class T>
  T& Array2D<T>::Get(unsigned int r, unsigned int c)
  {
    return matrix_shifted[r][c];
  }


  template<class T>
  T& Array2D<T>::GetLocal(unsigned int r, unsigned int c)
  {
    return matrix[r][c];
  }


}

#endif
