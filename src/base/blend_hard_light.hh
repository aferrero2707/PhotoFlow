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
class BlendHardLight: public BlendBase<T, colorspace, CHMIN, CHMAX, has_omap>
{
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) {}
};


/*
  Hard light blending is equivalent to Overlay with bottom and top layers swapped
 */
template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendHardLight<T, CS, CHMIN, CHMAX, false>: 
  public BlendBase<T, CS, CHMIN, CHMAX, false>
{
  BlendOverlay<T, CS, CHMIN, CHMAX, false> blender;
public:
  BlendHardLight(): BlendBase<T, CS, CHMIN, CHMAX, false>() {}
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    blender.blend( opacity, top, bottom, out, x, xomap );
  }
};

template<typename T, colorspace_t CS, int CHMIN, int CHMAX>
class BlendHardLight<T, CS, CHMIN, CHMAX, true>: 
  public BlendBase<T, CS, CHMIN, CHMAX, true>
{
  BlendOverlay<T, CS, CHMIN, CHMAX, false> blender;
  float opacity_real;
public:
  void blend(const float& opacity, T* bottom, T* top, T* out, const int& x, int& xomap) 
  {
    //int i = x;
    opacity_real = opacity*(this->pmap[xomap]+FormatInfo<T>::MIN)/(FormatInfo<T>::RANGE);
    xomap += 1;
    blender.blend( opacity_real, top, bottom, out, x, xomap );
  }
};
