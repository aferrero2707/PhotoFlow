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
class BlendSoftLight: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) {}
};


/*
  Default
 */
template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendSoftLight<T, CS, CHMIN, CHMAX, false>: 
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, pbottom, sqrttop, overlay;
  typename FormatInfo<T>::PROMOTED psum;
public:
  BlendSoftLight(): BlendBase<T, CS, CHMIN, CHMAX, false>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ptop = (typename FormatInfo<T>::PROMOTED)top[pos];
      pbottom = (typename FormatInfo<T>::PROMOTED)bottom[pos];
      if( bottom[pos] < FormatInfo<T>::HALF )
        overlay = (ptop*bottom[pos]/FormatInfo<T>::MAX)*2 +
          (ptop*ptop/FormatInfo<T>::MAX)*(psum - pbottom*2);	
      else {
        sqrttop = (typename FormatInfo<T>::PROMOTED)(sqrt(top[pos]));
        overlay = ((psum-bottom[pos])*top[pos]/FormatInfo<T>::MAX)*2 +
          sqrttop*(pbottom*2-psum)/FormatInfo<T>::MAX;
      }
      clip( opacity*overlay + (1.0f-opacity)*bottom[pos], out[pos] );
    }
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendSoftLight<T, CS, CHMIN, CHMAX, true>: 
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  BlendSoftLight<T, CS, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, bottom, top, out, x, xomap );
  }
};
