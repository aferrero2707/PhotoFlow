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
class BlendOverlay: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};


template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendOverlay<T, CS, CHMIN, CHMAX, false>:
public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, overlay;
  typename FormatInfo<T>::PROMOTED psum;
public:
  BlendOverlay(): BlendBase<T, CS, CHMIN, CHMAX, false>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      if( bottom[pos] < FormatInfo<T>::HALF )
        overlay = (((typename FormatInfo<T>::PROMOTED)top[pos])*bottom[pos]/FormatInfo<T>::MAX)*2;
      else
        overlay = psum - ((psum-top[pos])*(psum-bottom[pos])/FormatInfo<T>::RANGE)*2;

      clip( opacity*overlay + (1.0f-opacity)*bottom[pos], out[pos] );
    }
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendOverlay<T, CS, CHMIN, CHMAX, true>:
public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, overlay;
  typename FormatInfo<T>::PROMOTED psum;
  float opacity_real;
public:
  BlendOverlay(): BlendBase<T, CS, CHMIN, CHMAX, true>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap)
  {
    //int i = x;
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      if( bottom[pos] < FormatInfo<T>::HALF )
        overlay = (((typename FormatInfo<T>::PROMOTED)top[pos])*bottom[pos]/FormatInfo<T>::MAX)*2;
      else
        overlay = psum - ((psum-top[pos])*(psum-bottom[pos])/FormatInfo<T>::RANGE)*2;
      clip( opacity_real*overlay + (1.0f-opacity_real)*bottom[pos], out[pos] );
    }
  }
};






/**
 * Overlay blend implemented using GIMP definition:
 *
 * gimp_composite_overlay_any_any_any_generic:
 * @ctx: The compositing context.
 *
 * Perform an RGB[A] overlay operation between the pixel sources
 * ctx->A and ctx->B, using the generalised algorithm:
 *
 * D =  A * (A + (2 * B) * (255 - A))
 *
 **/
template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX, bool has_omap>
class BlendOverlayGimp: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};


template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendOverlayGimp<T, CS, CHMIN, CHMAX, false>:
public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, overlay;
  typename FormatInfo<T>::PROMOTED psum;
  float nbottom, ntop, noverlay;
public:
  BlendOverlayGimp(): BlendBase<T, CS, CHMIN, CHMAX, false>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ntop = ( ((float)top[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      nbottom = ( ((float)bottom[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      noverlay = nbottom * (nbottom + (ntop*2.0f)*(1.0f-nbottom));
      overlay = (typename FormatInfo<T>::PROMOTED)(noverlay * FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
      clip( opacity*overlay + (1.0f-opacity)*bottom[pos], out[pos] );
    }
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendOverlayGimp<T, CS, CHMIN, CHMAX, true>:
public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, overlay;
  typename FormatInfo<T>::PROMOTED psum;
  float nbottom, ntop, noverlay;
  float opacity_real;
public:
  BlendOverlayGimp(): BlendBase<T, CS, CHMIN, CHMAX, true>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ntop = ( ((float)top[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      nbottom = ( ((float)bottom[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      noverlay = nbottom * (nbottom + (ntop*2.0f)*(1.0f-nbottom));
      overlay = (typename FormatInfo<T>::PROMOTED)(noverlay * FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
      clip( opacity_real*overlay + (1.0f-opacity_real)*bottom[pos], out[pos] );
    }
  }
};
