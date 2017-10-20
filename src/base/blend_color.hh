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
class BlendColor: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
  int pos, ch;
public:
  void set_icc_data( ICCProfile* d ) { }
  void blend(const float& , T* , T* top, T* out, const int& x, int& )
  {
    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      out[pos] = top[pos];
    }
  }
};



/*
  RGB colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendColor<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>
{
  BlendLuminosity<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false> blend_lumi;
  int pos, ch;
  double temp_top;
  double irgb[3], rgb[3];
public:
  void set_icc_data( ICCProfile* d ) { blend_lumi.set_icc_data(d); }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap)
  {
    blend_lumi.blend( opacity, top, bottom, out, x, xomap );
    return;

    // RGB values of the bottom layer
    irgb[0] = (double(bottom[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[1] = (double(bottom[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[2] = (double(bottom[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    // Luminance value of the bottom layer
    double lumi = PF::luminance( irgb[0], irgb[1], irgb[2] );

    // RGB values of the top layer
    double ored = (double(top[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    double ogreen = (double(top[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    double oblue = (double(top[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    // Color blend: the color of the top layer is mixed with
    // the luminance of the bottom one
    rgb[0] = ored; rgb[1] = ogreen; rgb[2] = oblue;
    PF::lc_blend( rgb[0], rgb[1], rgb[2], lumi );

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      out[pos] = (T)(( (rgb[ch]*opacity)+(irgb[ch]*(1.0f-opacity)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
    }
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendColor<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>
{
  BlendColor<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
  void set_icc_data( ICCProfile* d ) { blender.set_icc_data(d); }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};




/*
  Lab colorspace
 */
template<typename T, int CHMIN, int CHMAX>
class BlendColor<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>
{
  int pos, ch;
  double temp_top;
  double rgb[3];
public:
  void set_icc_data( ICCProfile* d ) { }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    for( ch=CHMIN; ch<=0; ch++, pos++ ) 
      out[pos] = bottom[pos];
    for( ; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = static_cast<T>( opacity*top[pos]+(1.0f-opacity)*bottom[pos] );
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendColor<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>
{
  BlendColor<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
  void set_icc_data( ICCProfile* d ) { }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};


