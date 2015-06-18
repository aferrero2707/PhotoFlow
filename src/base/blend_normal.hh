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
class BlendNormal: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};


/*
  Greyscale colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, false>
{
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    out[x] = (T)(opacity*top[x] + (1.0f-opacity)*bottom[x]);
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, true>
{
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    out[x] = (T)(opacity_real*top[x] + (1.0f-opacity_real)*bottom[x]);
  }
};


/*
  RGB colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    // The target channel(s) get blended
    //std::cout<<"x: "<<x<<"  opacity: "<<opacity<<std::endl;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      //std::cout<<"pos: "<<pos<<"  bottom: "<<(int)bottom[pos]<<"  top: "<<(int)top[pos];
      out[pos] = (T)(opacity*top[pos] + (1.0f-opacity)*bottom[pos]);
      //std::cout<<"  out: "<<(int)out[pos]<<std::endl;
    }

    /*
    int i = x;
    //std::cout<<"  in: "<<(int)in[i]<<"  out: "<<(int)out[i]<<std::endl;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    //std::cout<<"  in: "<<(int)in[i]<<"  out: "<<(int)out[i]<<std::endl;
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    */
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    // The target channel(s) get blended
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = (T)(opacity_real*top[pos] + (1.0f-opacity_real)*bottom[pos]);

    /*
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    */
  }
};


/*
  LAB colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    // The target channel(s) get blended
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = (T)(opacity*top[pos] + (1.0f-opacity)*bottom[pos]);

    /*
    int i = x;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    */
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
 
    pos = x;
    // The target channel(s) get blended
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = (T)(opacity_real*top[pos] + (1.0f-opacity_real)*bottom[pos]);

    /*
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    */
  }
};


/*
  CMYK colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_CMYK, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_CMYK, CHMIN, CHMAX, false>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    // The target channel(s) get blended
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = (T)(opacity*top[pos] + (1.0f-opacity)*bottom[pos]);

    /*
    int i = x;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    */
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendNormal<T, PF_COLORSPACE_CMYK, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_CMYK, CHMIN, CHMAX, true>
{
  int ch, pos;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    // The target channel(s) get blended
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = (T)(opacity_real*top[pos] + (1.0f-opacity_real)*bottom[pos]);

    /*
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    */
  }
};
