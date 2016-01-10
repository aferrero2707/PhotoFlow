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
class BlendLuminance: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
  int pos, ch;
public:
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
class BlendLuminance<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>
{
  int pos, ch;
  double temp_top;
  double irgb[3];
  double rgb[3];
  ICCProfileData* data;
public:
  void set_icc_data( ICCProfileData* d ) { data = d; }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    if( !data ) {
      pos = x;
      for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
        out[pos] = top[pos];
      }
      return;
    }

    // RGB values of the bottom layer
    irgb[0] = (double(bottom[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[1] = (double(bottom[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[2] = (double(bottom[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    // RGB values of the top layer
    double ored = (double(top[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    double ogreen = (double(top[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    double oblue = (double(top[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    if( data->trc_type!=PF_TRC_LINEAR ) {
      // RGB values are encoded perceptually
      irgb[0] = data->perceptual_trc_vec[ (int)(irgb[0]*65535) ];
      irgb[1] = data->perceptual_trc_vec[ (int)(irgb[1]*65535) ];
      irgb[2] = data->perceptual_trc_vec[ (int)(irgb[2]*65535) ];
      ored = data->perceptual_trc_vec[ (int)(ored*65535) ];
      ogreen = data->perceptual_trc_vec[ (int)(ogreen*65535) ];
      oblue = data->perceptual_trc_vec[ (int)(oblue*65535) ];
    }

    float iL = data->Y_R*irgb[0] + data->Y_G*irgb[1] + data->Y_B*irgb[2];
    float oL = data->Y_R*ored + data->Y_G*ogreen + data->Y_B*oblue;
    float r = oL / MAX(iL, 0.0000000000000000001);

    rgb[0] = ored * r;
    rgb[1] = ogreen * r;
    rgb[2] = oblue * r;

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      out[pos] = (T)(( (rgb[ch]*opacity)+(irgb[ch]*(1.0f-opacity)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
      if( out[pos]<0 ) out[pos] = 0;
      if( out[pos]>1 ) out[pos] = 1;
      if( data->trc_type!=PF_TRC_LINEAR ) {
        out[pos] = data->perceptual_trc_inv_vec[ (int)(out[pos]*65535) ];
      }
    }
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendLuminance<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>
{
  BlendLuminance<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
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
class BlendLuminance<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>
{
  int pos, ch;
  double temp_top;
  double rgb[3];
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    for( ch=CHMIN; ch<=0; ch++, pos++ ) 
      out[pos] = static_cast<T>( opacity*top[pos]+(1.0f-opacity)*bottom[pos] );
    for( ; ch<=CHMAX; ch++, pos++ ) 
      out[pos] = bottom[pos];
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendLuminance<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>
{
  BlendLuminance<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};


