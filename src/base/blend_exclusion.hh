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
class BlendExclusion: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendExclusion<T, CS, CHMIN, CHMAX, false>:
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T temp_top;
  typename FormatInfo<T>::PROMOTED sum;
  typename FormatInfo<T>::PROMOTED max;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    max = 2;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      sum = bottom[pos]; sum += top[pos];
      if( sum < 0 ) sum = 0;
      if( sum > max ) sum = max;
      if( sum > 1 ) sum = max - sum;
      out[pos] = (T)( opacity*sum + (1.0f-opacity)*bottom[pos] );
    }
  }
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendExclusion<T, CS, CHMIN, CHMAX, true>:
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  T temp_top;
  float opacity_real;
  typename FormatInfo<T>::PROMOTED sum;
  typename FormatInfo<T>::PROMOTED max;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    max = 2;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      sum = bottom[pos]; sum += top[pos];
      if( sum < 0 ) sum = 0;
      if( sum > max ) sum = max;
      if( sum > 1 ) sum = max - sum;
      out[pos] = (T)( opacity_real*sum + (1.0f-opacity)*bottom[pos] );
      //std::cout<<"  out="<<(int)out[pos]<<std::endl;
    }
  }
};



/*
  Greyscale colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendExclusion<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>
{
  T temp_top;
  typename FormatInfo<T>::PROMOTED sum;
  typename FormatInfo<T>::PROMOTED max;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    max = 2;
    sum = bottom[x]; sum += top[x];
    if( sum < 0 ) sum = 0;
    if( sum > max ) sum = max;
    if( sum > 1 ) sum = max - sum;
    out[x] = (T)( opacity*sum + (1.0f-opacity)*bottom[x] );
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendExclusion<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>
{
  T temp_top;
  float opacity_real;
  typename FormatInfo<T>::PROMOTED sum;
  typename FormatInfo<T>::PROMOTED max;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    max = 2;
    sum = bottom[x]; sum += top[x];
    if( sum < 0 ) sum = 0;
    if( sum > max ) sum = max;
    if( sum > 1 ) sum = max - sum;
    out[x] = (T)( opacity_real*sum + (1.0f-opacity)*bottom[x] );
  }
};


