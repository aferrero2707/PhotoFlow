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
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
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
  typename FormatInfo<T>::PROMOTED ptop, pbottom, overlay;
  typename FormatInfo<T>::PROMOTED psum, MAX2;
  float nbottom, sqrtbot;
  float sqrtMAX;
public:
  BlendSoftLight(): BlendBase<T, CS, CHMIN, CHMAX, false>(), 
                    psum((typename FormatInfo<T>::PROMOTED)(FormatInfo<T>::MAX) + FormatInfo<T>::MIN), 
                    MAX2((typename FormatInfo<T>::PROMOTED)(FormatInfo<T>::MAX)*FormatInfo<T>::MAX),
                    sqrtMAX( sqrt(FormatInfo<T>::MAX) ) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      if( top[pos] <= FormatInfo<T>::HALF ) {
        pbottom = (typename FormatInfo<T>::PROMOTED)bottom[pos];
        overlay = ((pbottom*top[pos])/FormatInfo<T>::MAX)*2 +
          ((pbottom*pbottom)/FormatInfo<T>::MAX)*(psum - top[pos]*2)/FormatInfo<T>::MAX;	
      } else {
        ptop = (typename FormatInfo<T>::PROMOTED)top[pos];
        nbottom = static_cast<float>(bottom[pos])/FormatInfo<T>::MAX;
        sqrtbot = sqrt(nbottom);
        overlay = ((psum-top[pos])*bottom[pos]/FormatInfo<T>::MAX)*2 +
          (typename FormatInfo<T>::PROMOTED)(sqrtbot*(ptop*2-psum));        
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
