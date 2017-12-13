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
class BlendLCH: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
  int pos, ch;
  ICCProfile* data;
public:
  void set_channel(int ch) {}
  void set_icc_data( ICCProfile* d, VipsBandFormat fmt ) {
    data = d;
  }
  void set_img2lab_transform(ICCTransform* t) { }
  void set_lab2img_transform(ICCTransform* t) { }

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
class BlendLCH<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false>
{
  int pos, ch;
  float temp_top, temp_out;
  float brgb[3];
  float trgb[3];
  float orgb[3];
  ICCProfile* data;
  ICCTransform* img2lab;
  ICCTransform* lab2img;
  int tgtch;
public:
  void set_channel(int ch) {tgtch = ch;}
  void set_icc_data( ICCProfile* d, VipsBandFormat fmt ) {
    data = d;
  }
  void set_img2lab_transform(ICCTransform* t) { img2lab = t; }
  void set_lab2img_transform(ICCTransform* t) { lab2img = t; }

  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    //std::cout<<"BlendLCH: data="<<data<<std::endl;
    if( !data ) {
      pos = x;
      for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
        out[pos] = top[pos];
      }
      return;
    }

    //std::cout<<"BlendLCH: data->trc_type="<<data->trc_type<<std::endl;

    // RGB values of the bottom layer
    brgb[0] = (float(bottom[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    brgb[1] = (float(bottom[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    brgb[2] = (float(bottom[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    // RGB values of the bottom layer
    trgb[0] = (float(top[x]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    trgb[1] = (float(top[x+1]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
    trgb[2] = (float(top[x+2]) + FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;

    float blab[3], tlab[3], olab[3], blch[3], tlch[3], olch[3];
    img2lab->apply(brgb, blab, 1);
    img2lab->apply(trgb, tlab, 1);
    if(tgtch == 0) {
      olab[0] = (tlab[0]*opacity)+(blab[0]*(1.0f-opacity));
      olab[1] = blab[1];
      olab[2] = blab[2];
    } else if(tgtch == 1) {
      PF::Lab2LCH(blab, blch, 1);
      PF::Lab2LCH(tlab, tlch, 1);
      olch[0] = blch[0];
      olch[1] = (tlch[1]*opacity)+(blch[1]*(1.0f-opacity));
      olch[2] = blch[2];
      PF::LCH2Lab(olch, olab, 1);
    } else if(tgtch == 2) {
      PF::Lab2LCH(blab, blch, 1);
      PF::Lab2LCH(tlab, tlch, 1);
      olch[0] = blch[0];
      olch[1] = blch[1];
      olch[2] = (tlch[2]*opacity)+(blch[2]*(1.0f-opacity));
      PF::LCH2Lab(olch, olab, 1);
    }
    lab2img->apply(olab, orgb, 1);

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      out[pos] = (T)(orgb[ch]*FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
    }
  }
};

template<typename T, int CHMIN, int CHMAX>
class BlendLCH<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, true>
{
  BlendLCH<T, PF_COLORSPACE_RGB, CHMIN, CHMAX, false> blender;
  float opacity_real;
  ICCProfile* data;
public:
  void set_channel(int ch) {blender.set_channel(ch);}
  void set_icc_data( ICCProfile* d, VipsBandFormat fmt ) {
    data = d;
    blender.set_icc_data(data, fmt);
  }
  void set_img2lab_transform(ICCTransform* t) { blender.set_img2lab_transform(t); }
  void set_lab2img_transform(ICCTransform* t) { blender.set_lab2img_transform(t); }

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
class BlendLCH<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>:
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false>
{
  int pos, ch;
  double temp_top;
  double rgb[3];
  ICCProfile* data;
  int tgtch;
public:
  void set_channel(int ch) {tgtch = ch;}
  void set_icc_data( ICCProfile* d, VipsBandFormat fmt ) { data = d; }
  void set_img2lab_transform(ICCTransform* t) { }
  void set_lab2img_transform(ICCTransform* t) { }

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
class BlendLCH<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>:
  public BlendBase<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, true>
{
  BlendLCH<T, PF_COLORSPACE_LAB, CHMIN, CHMAX, false> blender;
  float opacity_real;
  ICCProfile* data;
public:
  void set_channel(int ch) {blender.set_channel(ch);}
  void set_icc_data( ICCProfile* d, VipsBandFormat fmt ) { data = d; blender.set_icc_data(data, fmt); }
  void set_img2lab_transform(ICCTransform* t) { }
  void set_lab2img_transform(ICCTransform* t) { }

  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};


