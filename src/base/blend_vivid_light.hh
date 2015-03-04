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


//#include "vivid_light.hh"


static float color_burn(float top, float bottom)
{
  if( top == 1 ) return 0;
  //float result = 1.0f – (1.0f – bottom) / top;
  float result = 1;
  return( (result<0) ? 0 : result );
}


static float color_dodge(float top, float bottom)
{
  if( top == 1 ) return 0;
  float result = bottom / (1.0f-top);
  return( (result>1) ? 1 : result );
}


template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX, bool has_omap>
class BlendVividLight: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) {}
};


/*
  Default
 */
template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendVividLight<T, CS, CHMIN, CHMAX, false>: 
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, pbottom, vivid;
  float ntop, nbottom, nvivid;
  typename FormatInfo<T>::PROMOTED psum;
public:
  BlendVividLight(): BlendBase<T, CS, CHMIN, CHMAX, false>(), psum(FormatInfo<T>::MAX + FormatInfo<T>::MIN) {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
#ifdef DO_WARNINGS
#warning "TODO: optimize vivid light blend"
#warning "TODO: handle vivid light blend for Lab and CMYK"
#endif
    pos = x;
    psum = FormatInfo<T>::MAX + FormatInfo<T>::MIN;
    for( ch=CHMIN; ch<=CHMAX; ch++, pos++ ) {
      ptop = top[pos]; pbottom = bottom[pos];
      ntop = ( ((float)top[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      nbottom = ( ((float)bottom[pos])+FormatInfo<T>::MIN )/FormatInfo<T>::RANGE;
      if( ntop <= 0.5 )
        //vivid = (ptop*FormatInfo<T>::RANGE / (FormatInfo<T>::RANGE-pbottom*2));
        //result = ((top==0) ? 0 : 1.0f – (1.0f – bottom) / (top*2.0f));
        //result = ((top==0) ? 0 : 1.0f – (1.0f – bottom) / (top*2.0f));
        nvivid = ((ntop==0) ? 0 : 1.0f - (1.0f - nbottom) / (ntop*2.0f));
      else
        //vivid = (FormatInfo<T>::RANGE - (FormatInfo<T>::RANGE-ptop) / (2*(pbottom-FormatInfo<T>::HALF)));
        nvivid = ((ntop==1) ? 1 : nbottom / (2.0f*(1.0f-ntop)));
      //nvivid = PF::vivid_light( ntop, nbottom );
      if( nvivid < 0 ) nvivid = 0;
      if( nvivid > 1 ) nvivid = 1;
      vivid = (typename FormatInfo<T>::PROMOTED)(nvivid * FormatInfo<T>::RANGE - FormatInfo<T>::MIN);
      clip( opacity*vivid + (1.0f-opacity)*bottom[pos], out[pos] );
    }
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendVividLight<T, CS, CHMIN, CHMAX, true>: 
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  int ch, pos;
  T ibottom;
  typename FormatInfo<T>::PROMOTED ptop, pbottom, vivid;
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
      ptop = top[pos]; pbottom = bottom[pos];
      if( bottom[pos] <= FormatInfo<T>::HALF )
        vivid = (ptop*FormatInfo<T>::RANGE / (FormatInfo<T>::RANGE-pbottom*2));
      else
        vivid = (FormatInfo<T>::RANGE - (FormatInfo<T>::RANGE-ptop) / (2*(pbottom-FormatInfo<T>::HALF)));
      clip( opacity_real*vivid + (1.0f-opacity_real)*bottom[pos], out[pos] );
    }
  }
};
