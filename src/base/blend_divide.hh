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



template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX, bool has_omap>
class BlendDivide: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendDivide<T, CS, CHMIN, CHMAX, false>:
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  typename FormatInfo<T>::PROMOTED ptop;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      //ptop = top[pos];
      ptop = ((typename FormatInfo<T>::PROMOTED)top[pos])*bottom[pos]/FormatInfo<T>::MAX;
      clip( opacity*ptop + (1.0f-opacity)*bottom[pos], out[pos] );
      //std::cout<<"  out="<<(int)out[pos]<<std::endl;
    }
  }
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendDivide<T, CS, CHMIN, CHMAX, true>:
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  typename FormatInfo<T>::PROMOTED ptop;
  float opacity_real;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      //ptop = top[pos];
      ptop = ((typename FormatInfo<T>::PROMOTED)top[pos])*bottom[pos]/FormatInfo<T>::MAX;
      clip( opacity_real*ptop + (1.0f-opacity_real)*bottom[pos], out[pos] );
      //std::cout<<"  out="<<(int)out[pos]<<std::endl;
    }
  }
};



template<colorspace_t CS, int CHMIN, int CHMAX>
class BlendDivide<float, CS, CHMIN, CHMAX, false>:
  public BlendBase<float, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
public:
  void blend(const float& opacity, float* bottom, float* top, float* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      if( bottom[pos] == 0 ) out[pos] = 0;
      else if( fabs(top[pos]) < 1.0E-16 ) out[pos] = bottom[pos]*1.E16;
      else out[pos] = opacity*(bottom[pos]/top[pos]) + (1.0f-opacity)*bottom[pos];
    }
  }
};



template<colorspace_t CS, int CHMIN, int CHMAX>
class BlendDivide<float, CS, CHMIN, CHMAX, true>:
  public BlendBase<float, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  float opacity_real;
public:
  void blend(const float& opacity, float* bottom, float* top, float* out, const int& x, int& xomap)
  {
    opacity_real = opacity*this->pmap[xomap];
    xomap += 1;

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      if( bottom[pos] == 0 ) out[pos] = 0;
      else if( fabs(top[pos]) < 1.0E-16 ) out[pos] = bottom[pos]*1.E16;
      else out[pos] = opacity_real*(bottom[pos]/top[pos]) + (1.0f-opacity_real)*bottom[pos];
    }
  }
};



/*
  Greyscale colorspace
 */

template<typename T, int CHMIN, int CHMAX>
class BlendDivide<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>
{
  typename FormatInfo<T>::PROMOTED ptop;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    ptop = top[x];
    ptop *= bottom[x];
    clip( opacity*ptop/FormatInfo<T>::RANGE + (1.0f-opacity)*bottom[x], out[x] );
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendDivide<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>
{
  typename FormatInfo<T>::PROMOTED ptop;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    ptop = top[x];
    ptop *= bottom[x];
    clip( opacity_real*ptop/FormatInfo<T>::RANGE + (1.0f-opacity_real)*bottom[x], out[x] );
  }
};


template<int CHMIN, int CHMAX>
class BlendDivide<float, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>:
  public BlendBase<float, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>
{
public:
  void blend(const float& opacity, float* bottom, float* top, float* out, const int& x, int& /*xomap*/)
  {
    if( bottom[x] == 0 ) out[x] = 0;
    else if( fabs(top[x]) < 1.0E-16 ) out[x] = bottom[x]*1.E16;
    else out[x] = opacity*(bottom[x]/top[x]) + (1.0f-opacity)*bottom[x];
  }
};

template<int CHMIN, int CHMAX>
class BlendDivide<float, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>:
  public BlendBase<float, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>
{
public:
  void blend(const float& opacity, float* bottom, float* top, float* out, const int& x, int& xomap)
  {
    float opacity_real = opacity*this->pmap[xomap];
    xomap += 1;
    if( bottom[x] == 0 ) out[x] = 0;
    else if( fabs(top[x]) < 1.0E-16 ) out[x] = bottom[x]*1.E16;
    else out[x] = opacity_real*(bottom[x]/top[x]) + (1.0f-opacity_real)*bottom[x];
  }
};


