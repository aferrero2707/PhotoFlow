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
class BlendLuminosity: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
  int pos, ch;
  ICCProfile* data;
public:
  void set_icc_data( ICCProfile* d ) { data = d; }
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
class BlendLuminosity<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>
{
  int pos, ch;
  float temp_top;
  float irgb[3];
  float rgb[3];
  ICCProfile* data;
public:
  void set_icc_data( ICCProfile* d ) { data = d; }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    // RGB values of the bottom layer
    irgb[0] = (float(bottom[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[1] = (float(bottom[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    irgb[2] = (float(bottom[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    // RGB values of the top layer
    float ored = (double(top[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    float ogreen = (double(top[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    float oblue = (double(top[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
/*
    if( data->get_trc_type()!=PF_TRC_LINEAR ) {
      //std::cout<<"BlendLuminance: perceptual -> linear: "<<irgb[0]<<" -> "<<data->perceptual_trc_vec[ (int)(irgb[0]*65535) ]<<std::endl;
      // RGB values are encoded perceptually
      irgb[0] = data->perceptual2linear( irgb[0] );
      irgb[1] = data->perceptual2linear( irgb[1] );
      irgb[2] = data->perceptual2linear( irgb[2] );
      ored = data->perceptual2linear( ored );
      ogreen = data->perceptual2linear( ogreen );
      oblue = data->perceptual2linear( oblue );

      //irgb[0] = data->perceptual_trc_vec[ (int)(irgb[0]*65535) ];
      //irgb[1] = data->perceptual_trc_vec[ (int)(irgb[1]*65535) ];
      //irgb[2] = data->perceptual_trc_vec[ (int)(irgb[2]*65535) ];
      //ored = data->perceptual_trc_vec[ (int)(ored*65535) ];
      //ogreen = data->perceptual_trc_vec[ (int)(ogreen*65535) ];
      //oblue = data->perceptual_trc_vec[ (int)(oblue*65535) ];

      //irgb[0] = cmsEvalToneCurveFloat( data->perceptual_trc, irgb[0] );
      //irgb[1] = cmsEvalToneCurveFloat( data->perceptual_trc, irgb[0] );
      //irgb[2] = cmsEvalToneCurveFloat( data->perceptual_trc, irgb[0] );
    }
*/
    // Luminance value of the top layer
    //float lumi = PF::luminance( ored, ogreen, oblue );
    float iL = data->get_lightness( irgb[0], irgb[1], irgb[2] );
    float oL = data->get_lightness( ored, ogreen, oblue );
    float Ldiff = oL - iL;

    // Luminosity blend: the color of the bottom layer is mixed with
    // the luminance of the top one
    //rgb[0] = irgb[0]; rgb[1] = irgb[1]; rgb[2] = irgb[2];
    //PF::lc_blend( rgb[0], rgb[1], rgb[2], lumi );
    rgb[0] = irgb[0] + Ldiff;
    rgb[1] = irgb[1] + Ldiff;
    rgb[2] = irgb[2] + Ldiff;

    // Clip out-of-bound values
    float min = MIN3( rgb[0], rgb[1], rgb[2] );
    float max = MAX3( rgb[0], rgb[1], rgb[2] );
    if( min < FormatInfo<T>::MIN ) {
      float lumi = data->get_lightness( rgb[0], rgb[1], rgb[2] );
      float D = lumi - min;
      rgb[0] = lumi + (((rgb[0] - lumi) * lumi) / D);
      rgb[1] = lumi + (((rgb[1] - lumi) * lumi) / D);
      rgb[2] = lumi + (((rgb[2] - lumi) * lumi) / D);
    }
    if( max > FormatInfo<T>::MAX ) {
      float lumi = data->get_lightness( rgb[0], rgb[1], rgb[2] );
      float d = max - lumi;
      float D = FormatInfo<T>::MAX - min;
      rgb[0] = lumi + (((rgb[0] - lumi) * D) / d);
      rgb[1] = lumi + (((rgb[1] - lumi) * D) / d);
      rgb[2] = lumi + (((rgb[2] - lumi) * D) / d);
    }

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      out[pos] = (T)(( (rgb[ch]*opacity)+(irgb[ch]*(1.0f-opacity)) )*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
    }
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendLuminosity<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>
{
  BlendLuminosity<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false> blender;
  float opacity_real;
  ICCProfile* data;
public:
  void set_icc_data( ICCProfile* d ) { data = d; blender.set_icc_data(data); }
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
class BlendLuminosity<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>
{
  int pos, ch;
  double temp_top;
  double rgb[3];
  ICCProfile* data;
public:
  void set_icc_data( ICCProfile* d ) { data = d; }
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
class BlendLuminosity<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>: 
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>
{
  BlendLuminosity<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false> blender;
  float opacity_real;
  ICCProfile* data;
public:
  void set_icc_data( ICCProfile* d ) { data = d; blender.set_icc_data(data); }
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};


