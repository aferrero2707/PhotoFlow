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


#ifndef BLENDER_HH
#define BLENDER_HH


#include "pftypes.hh"


namespace PF 
{


#define BLEND_LOOP( theblender ) {                                \
    theblender.init_line( omap, r->left, y0 );                    \
    for( x=0, xomap=0; x < line_size; ) {                         \
      for( ch=0; ch<CHMIN; ch++, x++ ) pout[x] = pbottom[x];      \
      theblender.blend( opacity, pbottom, ptop, pout, x, xomap );       \
      x += dx;                                                         \
      for( ch=CHMAX+1; ch<PF::ColorspaceInfo<colorspace>::NCH; ch++, x++ ) pout[x] = pbottom[x]; \
    }                                                                   \
  }


#define BLEND_LOOP2( theblender ) {                               \
    theblender.init_line( omap, r->left, y0 );                    \
    for( x=0, xomap=0; x < line_size; ) {                         \
      theblender.blend( opacity, pbottom, ptop, pout, x, xomap ); \
      x += PF::ColorspaceInfo<colorspace>::NCH;                   \
    }                                                             \
  }


  
  template<typename T, colorspace_t colorspace, int CHMIN, int CHMAX, bool has_omap>
  class Blender
  {
    blendmode_t mode;
    float opacity;

  public:
    Blender( blendmode_t m, float o ):
      mode( m ), opacity( o )
    {
    }
    
    void blend(VipsRegion* bottom, VipsRegion* top, VipsRegion* oreg, VipsRegion* omap) 
    {
      if( !bottom || !top ) return;
      BlendNormal<T,colorspace,CHMIN,CHMAX,has_omap> blend_normal;
      BlendGrainExtract<T,colorspace,CHMIN,CHMAX,has_omap> blend_grain_extract;
      BlendGrainMerge<T,colorspace,CHMIN,CHMAX,has_omap> blend_grain_merge;
      BlendMultiply<T,colorspace,CHMIN,CHMAX,has_omap> blend_multiply;
      BlendScreen<T,colorspace,CHMIN,CHMAX,has_omap> blend_screen;
      BlendLighten<T,colorspace,CHMIN,CHMAX,has_omap> blend_lighten;
      BlendDarken<T,colorspace,CHMIN,CHMAX,has_omap> blend_darken;
      BlendOverlay<T,colorspace,CHMIN,CHMAX,has_omap> blend_overlay;
      BlendSoftLight<T,colorspace,CHMIN,CHMAX,has_omap> blend_soft_light;
      BlendHardLight<T,colorspace,CHMIN,CHMAX,has_omap> blend_hard_light;
      BlendVividLight<T,colorspace,CHMIN,CHMAX,has_omap> blend_vivid_light;
      BlendLuminosity<T,colorspace,CHMIN,CHMAX,has_omap> blend_lumi;
      BlendColor<T,colorspace,CHMIN,CHMAX,has_omap> blend_color;
      Rect *r = &oreg->valid;
      //int x, y, xomap, y0, dx1=CHMIN, dx2=PF::ColorspaceInfo<colorspace>::NCH-CHMIN, ch, CHMAXplus1=CHMAX+1;
      int x, y, xomap, y0, dx=CHMAX-CHMIN+1, ch;
      int line_size = r->width * oreg->im->Bands;
      T* pbottom;
      T* ptop;
      T* pout;
      for( y = 0; y < r->height; y++ ) {      
        y0 = r->top + y;
        pbottom = (T*)VIPS_REGION_ADDR( bottom, r->left, y0 ); 
        ptop = (T*)VIPS_REGION_ADDR( top, r->left, y0 ); 
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, y0 ); 
        switch(mode) {
        case PF_BLEND_PASSTHROUGH:
          break;
        case PF_BLEND_NORMAL:
          BLEND_LOOP(blend_normal);
          break;
        case PF_BLEND_GRAIN_EXTRACT:
          BLEND_LOOP(blend_grain_extract);
          break;
        case PF_BLEND_GRAIN_MERGE:
          BLEND_LOOP(blend_grain_merge);
          break;
        case PF_BLEND_OVERLAY:
          BLEND_LOOP(blend_overlay);
          break;
        case PF_BLEND_SOFT_LIGHT:
          BLEND_LOOP(blend_soft_light);
          break;
        case PF_BLEND_HARD_LIGHT:
          BLEND_LOOP(blend_hard_light);
          break;
        case PF_BLEND_VIVID_LIGHT:
          BLEND_LOOP(blend_vivid_light);
          break;
        case PF_BLEND_MULTIPLY:
          BLEND_LOOP(blend_multiply);
          break;
        case PF_BLEND_SCREEN:
          BLEND_LOOP(blend_screen);
          break;
        case PF_BLEND_LIGHTEN:
          BLEND_LOOP(blend_lighten);
          break;
        case PF_BLEND_DARKEN:
          BLEND_LOOP(blend_darken);
          break;
        case PF_BLEND_LUMI:
          BLEND_LOOP2(blend_lumi);
          break;
        case PF_BLEND_COLOR:
          BLEND_LOOP2(blend_color);
          break;
        case PF_BLEND_UNKNOWN:
          break;
        }
      }  
    }
  };



}


#endif
