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



template<typename T, colorspace_t colorspace, bool has_omap>
class BlendNormal: public BlendBase<T, colorspace, has_omap>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) {}
};


/*
  Greyscale colorspace
 */
template<typename T>
class BlendNormal<T, PF_COLORSPACE_GRAYSCALE, false>: public BlendBase<T, PF_COLORSPACE_GRAYSCALE, false>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    out[x] = (T)(opacity*out[x] + (1.0f-opacity)*in[x]);
  }
};

template<typename T>
class BlendNormal<T, PF_COLORSPACE_GRAYSCALE, true>: public BlendBase<T, PF_COLORSPACE_GRAYSCALE, true>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    out[x] = (T)(opacity_real*out[x] + (1.0f-opacity_real)*in[x]);
  }
};


/*
  RGB colorspace
 */
template<typename T>
class BlendNormal<T, PF_COLORSPACE_RGB, false>: public BlendBase<T, PF_COLORSPACE_RGB, false>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    //std::cout<<"x: "<<x<<"  opacity: "<<opacity<<std::endl;
    int i = x;
    //std::cout<<"  in: "<<(int)in[i]<<"  out: "<<(int)out[i]<<std::endl;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    //std::cout<<"  in: "<<(int)in[i]<<"  out: "<<(int)out[i]<<std::endl;
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
  }
};

template<typename T>
class BlendNormal<T, PF_COLORSPACE_RGB, true>: public BlendBase<T, PF_COLORSPACE_RGB, true>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
  }
};


/*
  LAB colorspace
 */
template<typename T>
class BlendNormal<T, PF_COLORSPACE_LAB, false>: public BlendBase<T, PF_COLORSPACE_LAB, false>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    int i = x;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
  }
};

template<typename T>
class BlendNormal<T, PF_COLORSPACE_LAB, true>: public BlendBase<T, PF_COLORSPACE_LAB, true>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
  }
};


/*
  CMYK colorspace
 */
template<typename T>
class BlendNormal<T, PF_COLORSPACE_CMYK, false>: public BlendBase<T, PF_COLORSPACE_CMYK, false>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    int i = x;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
    i++;
    out[i] = (T)(opacity*out[i] + (1.0f-opacity)*in[i]);
  }
};

template<typename T>
class BlendNormal<T, PF_COLORSPACE_CMYK, true>: public BlendBase<T, PF_COLORSPACE_CMYK, true>
{
public:
  void blend(const float& opacity, T* in, T* out, const int& x, int& xomap) 
  {
    int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
    i += 1;
    out[i] = (T)(opacity_real*out[i] + (1.0f-opacity_real)*in[i]);
  }
};
