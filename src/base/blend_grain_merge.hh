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
class BlendGrainMerge: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& /*opacity*/, T* /*bottom*/, T* /*top*/,
      T* /*out*/, const int& /*x*/, int& /*xomap*/) {}
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendGrainMerge<T, CS, CHMIN, CHMAX, false>: 
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  typename FormatInfo<T>::SIGNED ptop;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& /*xomap*/)
  {
    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ptop = ( ((typename FormatInfo<T>::SIGNED)(top[pos])-FormatInfo<T>::HALF)*2 + bottom[pos] );
      clip( opacity*ptop + (1.0f-opacity)*bottom[pos], out[pos] );
      //std::cout<<"  out="<<(int)out[pos]<<std::endl;
    }
  }
};



template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendGrainMerge<T, CS, CHMIN, CHMAX, true>: 
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  typename FormatInfo<T>::SIGNED ptop;
  float opacity_real;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;

    pos = x;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ptop = ( ((typename FormatInfo<T>::SIGNED)top[pos]-FormatInfo<T>::HALF)*2 + bottom[pos] );
      clip( opacity_real*ptop + (1.0f-opacity_real)*bottom[pos], out[pos] );
      //std::cout<<"  out="<<(int)out[pos]<<std::endl;
    }
  }
};



