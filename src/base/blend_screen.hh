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
class BlendScreen: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};


/*
  Default
 */
template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendScreen<T, CS, CHMIN, CHMAX, false>: 
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, screen;
  typename FormatInfo<T>::PROMOTED psum;
public:
  BlendScreen(): BlendBase<T, CS, CHMIN, CHMAX, false>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    //psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      //ptop = psum - top[pos];
      //ibottom = psum - bottom[pos];
      screen = psum - ((psum-top[pos])*(psum-bottom[pos])/FormatInfo<T>::RANGE);
      //std::cout<<"bottom="<<(int)bottom[pos]<<"  top="<<(int)top[pos]<<"  psum="<<(int)psum<<"  screen="<<(int)screen<<std::endl;
      //std::cout<<"  (psum-top[pos])="<<(int)(psum-top[pos])<<"  (psum-bottom[pos])="<<(int)(psum-bottom[pos])
      //       <<"  range="<<(int)FormatInfo<T>::RANGE<<"    ((psum-top[pos])*(psum-bottom[pos])/FormatInfo<T>::RANGE)="
      //       <<(int)((psum-top[pos])*(psum-bottom[pos])/FormatInfo<T>::RANGE)<<std::endl;
      clip( opacity*screen + (1.0f-opacity)*bottom[pos], out[pos] );
    }
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendScreen<T, CS, CHMIN, CHMAX, true>: 
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, screen;
  typename FormatInfo<T>::PROMOTED psum;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    float opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ptop = psum - top[pos];
      ibottom = psum - bottom[pos];
      screen = psum - (ptop*ibottom/FormatInfo<T>::RANGE);
      clip( opacity_real*screen + (1.0f-opacity_real)*bottom[pos], out[pos] );
    }
  }
};
