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

#ifndef PF_PATH_MASK_H
#define PF_PATH_MASK_H

#include <math.h>

#include <iostream>

#include "../base/format_info.hh"
#include "../base/processor.hh"
#include "../base/splinecurve.hh"

#include "curves.hh"

namespace PF 
{


typedef struct _falloff_segment
{
  int x1, y1, x2, y2, xmin, xmax, lmin, lmax, l;
  float fl;
} falloff_segment;

typedef struct _path_point
{
  int x, y;
  bool state_changing;
} path_point;

void init_path_segment( falloff_segment& seg, int x1, int y1, int x2, int y2, int ymin, int ymax );

class PathMaskPar: public OpParBase
{
  Property<bool> invert;
  Property<bool> enable_falloff;

  Property<SplineCurve> falloff_curve;

  Property<ClosedSplineCurve> smod;

  Property<float> border_size;

  std::vector< std::pair<float,float> > spoints;
  std::vector< std::pair<float,float> > spoints2;

  ProcessorBase* curve;

public:
  float* modvec;
  float falloff_vec[65536];

  std::vector<path_point>* edgevec;
  //std::vector<float>* edgevec;
  std::vector< falloff_segment >* segvec;
  std::vector< std::pair<int,int> > ptvec;

  PathMaskPar();

  bool get_invert() { return invert.get(); }

  bool get_falloff_enabled() { return enable_falloff.get(); }

  void path_modified() { smod.modified(); }
  void path_reset() { smod.reset(); }

  SplineCurve& get_falloff_curve() { return falloff_curve.get(); }
  ClosedSplineCurve& get_smod() { return smod.get(); }

  float get_border_size() { return border_size.get(); }
  void set_border_size( float sz ) { border_size.update(sz); }

  bool needs_input() { return false; }
  bool has_intensity() { return false; }

  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level);
};


void get_line_points(int x1, int y1, int x2, int y2, std::vector< std::pair<int,int> >& points);


template<class T>
void get_falloff_curve( float* vec, float val, T& out ) {
  out = 0;
}



template < OP_TEMPLATE_DEF >
class PathMask: public IntensityProc<T, has_imap>
{
public:
  void render(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, PathMaskPar* par);

  void render_spline(VipsRegion** in, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* out, PathMaskPar* par);

  void draw_segment(VipsRegion* oreg, const falloff_segment& seg, float* vec);
};


template< OP_TEMPLATE_DEF >
void PathMask< OP_TEMPLATE_IMP >::
render(VipsRegion** ir, int n, int in_first,
    VipsRegion* imap, VipsRegion* omap,
    VipsRegion* oreg, PathMaskPar* par)
    {
  //BLENDER blender( par->get_blend_mode(), par->get_opacity() );

    render_spline( ir, n, in_first, imap, omap, oreg, par );
};



#define SWAP_PX( X, Y ) {                         \
  pout = (T*)VIPS_REGION_ADDR( oreg, X, Y );      \
  if( pout[0] == PF::FormatInfo<T>::MIN ) {       \
    for( b = 0; b < bands; b++ )                  \
      pout[b] = PF::FormatInfo<T>::MAX;           \
  } else {                                        \
    for( b = 0; b < bands; b++ )                  \
      pout[b] = PF::FormatInfo<T>::MIN;           \
  }                                               \
  xlast = X; ylast = Y;                           \
}


#define DRAW_OUTLINE_PX( X, Y ) {                           \
  if( vips_rect_includespoint(r,X,Y) ) {                           \
    pout = (T*)VIPS_REGION_ADDR( oreg, X, Y );                           \
    for( b = 0; b < bands; b++ ) pout[b] = FormatInfo<T>::MAX;                           \
    xlast = X; ylast = Y;                           \
  } else {                           \
    if( (X == xlast) && (Y == ylast) ) continue;                           \
    if( (Y < top) && (X >= left) && (X <= right) ) {                           \
      SWAP_PX( X, top );                           \
    }                           \
    if( (Y > bottom) && (X >= left) && (X <= right) ) {                           \
      SWAP_PX( X, bottom );                           \
    }                           \
    if( (X < left) && (Y >= top) && (Y <= bottom) ) {                           \
      SWAP_PX( left, Y );                           \
    }                           \
    if( (X > right) && (Y >= top) && (Y <= bottom) ) {                           \
      SWAP_PX( right, Y );                           \
    }                           \
  }                          \
}

template< OP_TEMPLATE_DEF >
void PathMask< OP_TEMPLATE_IMP >::
render_spline(VipsRegion** ir, int n, int in_first,
    VipsRegion* imap, VipsRegion* omap,
    VipsRegion* oreg, PathMaskPar* par)
    {
  //BLENDER blender( par->get_blend_mode(), par->get_opacity() );

  Rect *r = &oreg->valid;
  int bands = oreg->im->Bands;
  int line_size = r->width * bands; //layer->in_all[0]->Bands;

  //vips_region_black( oreg );

  T* pout;
  float* vec = par->falloff_vec;
  bool empty = ( par->get_smod().get_npoints() == 0 );

  int width = oreg->im->Xsize - oreg->im->Xoffset;
  int height = oreg->im->Ysize - oreg->im->Yoffset;
  float scale = 2;

  int x, y, i, b;
  int left = r->left;
  int top = r->top;
  int right = r->left + r->width - 1;
  int bottom = r->top + r->height - 1;

  //std::cout<<"left="<<r->left<<"  right="<<right<<std::endl;
  //std::cout<<"drawing region @"<<r->left<<","<<r->top<<" -> "<<r->left+r->width-1<<","<<r->top+r->height-1<<std::endl;
  for( y = 0; y < r->height; y++ ) {
    pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top+y );

    if( empty ) {
      for( x = 0; x < r->width; x++, pout += bands ) {
        for( b = 0; b < bands; b++ )
          pout[b] = FormatInfo<T>::MAX;
      }
      continue;
    }

    int state = 0;
    bool is_outline = false;
    x = r->left;
    // draw left region
    int xstart = par->edgevec[r->top+y][0].x, xend = -1;
    for( unsigned int pi = 0; pi < par->edgevec[r->top+y].size()-1; pi++ ) {
      xstart = par->edgevec[r->top+y][pi].x;
      xend = par->edgevec[r->top+y][pi+1].x;
      if( xstart == xend ) continue;

      for( ; x < xstart; x++, pout += bands ) {
        if( x > right ) break;
        for( b = 0; b < bands; b++ )
          //pout[b] = (state != 0) ? FormatInfo<T>::MAX : FormatInfo<T>::MIN;
          get_falloff_curve( vec, (state != 0)?0.0f:1.0f, pout[b] );
      }

      //if( (xend-xstart) > 1 ) {
      //std::cout<<"par->edgevec["<<r->top+y<<"]["<<pi<<"].state_changing="
      //    <<par->edgevec[r->top+y][pi].state_changing<<std::endl;
      if( par->edgevec[r->top+y][pi].state_changing ) {
        // gap found, we change the state
        state = 1 - state;
      }

      // check if segment is at least partly contained in the region
      bool crossing = false;
      if( xstart <= right && xend >= r->left ) crossing = true;
      if( !crossing ) continue;
      //std::cout<<"crossing1="<<crossing1<<"  crossing2="<<crossing2<<std::endl;
      if( false && r->top+y < 64 )
        std::cout<<"par->edgevec["<<r->top+y<<"]["<<pi<<"]="
        <<par->edgevec[r->top+y][pi].x<<"  xstart="<<xstart
        <<"  xend="<<xend<<"  state="<<state<<std::endl;

      if( xstart >= r->left && xstart <= right ) {
        if( false && r->top+y < 64 )
          std::cout<<"  drawing outline at x="<<x<<std::endl;
        for( b = 0; b < bands; b++ )
          //pout[b] = FormatInfo<T>::MAX;
          get_falloff_curve( vec, 0.0f, pout[b] );
        x++; pout += bands;
      }

      for( ; x < xend; x++, pout += bands ) {
        if( x > right ) break;
        for( b = 0; b < bands; b++ )
          //pout[b] = (state != 0) ? FormatInfo<T>::MAX : FormatInfo<T>::MIN;
          get_falloff_curve( vec, (state != 0)?0.0f:1.0f, pout[b] );
      }
      /**/
      if( xend >= r->left && xend <= right ) {
        if( false && r->top+y < 64 )
          std::cout<<"  drawing outline at x="<<x<<std::endl;
        for( b = 0; b < bands; b++ )
          //pout[b] = FormatInfo<T>::MAX;
          get_falloff_curve( vec, 0.0f, pout[b] );
        //x++; pout += bands;
      }
      /**/
    }
    /*
    if( xend >= r->left && xend <= right ) {
      // there still one border point that needs drawing
      if( r->top+y < 64 )
        std::cout<<"  drawing outline at x="<<xend<<std::endl;
      pout = (T*)VIPS_REGION_ADDR( oreg, xend, r->top+y );
      for( b = 0; b < bands; b++ ) pout[b] = FormatInfo<T>::MAX;
      x++; pout += bands;
    }
    */
    // draw right region
    x = MAX(r->left,xend+1);
    pout = (T*)VIPS_REGION_ADDR( oreg, x, r->top+y );
    for( ; x <= right; x++, pout += bands ) {
      for( b = 0; b < bands; b++ )
        //pout[b] = FormatInfo<T>::MIN;
        get_falloff_curve( vec, 1.0f, pout[b] );
    }
  }

  if( par->get_falloff_enabled() == false )
    return;

  /*
    for( unsigned int pi = 0; pi < par->ptvec.size(); pi++ ) {
      if( !vips_rect_includespoint(r, par->ptvec[pi].first, par->ptvec[pi].second) )
        continue;
      pout = (T*)VIPS_REGION_ADDR( oreg, par->ptvec[pi].first, par->ptvec[pi].second );
      for( b = 0; b < bands; b++ ) pout[b] = FormatInfo<T>::MAX/(b+1);
    }
    return;
  */

  std::pair<float,float> outline_scaling =
      par->get_smod().get_outline_scaling( width, height );

  std::pair<float,float> outline_center = par->get_smod().get_center();
  float cx = outline_center.first * width;
  float cy = outline_center.second * height;

  const std::vector< std::pair<int,int> >& spline_points = par->get_smod().get_outline();
  std::pair<float,float> pt;

  //-----------------
  // draw falloff
  /**/
  int si = r->top/64;
  //std::cout<<"r->top="<<r->top<<"  par->segvec["<<si<<"].size()="<<par->segvec[si].size()<<std::endl;
  for( unsigned int pi = 0; pi < par->segvec[si].size(); pi++ ) {
    int pi2 = pi + 1;
    if( pi2 >= (int)par->segvec[si].size() ) pi2 = 0;

    const falloff_segment& seg = par->segvec[si][pi];
    const falloff_segment& seg2 = par->segvec[si][pi2];

    if( (seg.x1==seg2.x1) && (seg.y1==seg2.y1) &&
        (seg.x2==seg2.x2) && (seg.y2==seg2.y2) )
      continue;

    if( (seg.xmin<left) && (seg.xmax<left) )
      continue;
    if( (seg.xmin>right) && (seg.xmax>right) )
      continue;

    draw_segment( oreg, seg, vec );
    /*
    int ibpx = par->segvec[si][pi].first;
    int ibpy = par->segvec[si][pi].second;
    int ibpx2 = par->segvec[si][pi2].first;
    int ibpy2 = par->segvec[si][pi2].second;

    // get corresponding point on path
    int ipx = (ibpx-cx)/scale + cx;
    int ipy = (ibpy-cy)/scale + cy;
    int ipx2 = (ibpx2-cx)/scale + cx;
    int ipy2 = (ibpy2-cy)/scale + cy;

    bool crossing1 = true;
    if( (ipx < r->left) && (ibpx < r->left) ) crossing1 = false;
    if( (ipx > right) && (ibpx > right) ) crossing1 = false;
    bool crossing2 = true;
    if( (ipx2 < r->left) && (ibpx2 < r->left) ) crossing2 = false;
    if( (ipx2 > right) && (ibpx2 > right) ) crossing2 = false;

    if( !crossing1 && !crossing2 ) continue;

    //std::cout<<"drawing border segment "<<ibpx<<","<<ibpy
    //    <<" -> "<<ibpx2<<","<<ibpy2<<std::endl;

    std::vector< std::pair<int,int> > ptvec2;
    get_line_points( ibpx, ibpy, ibpx2, ibpy2, ptvec2 );
    for( unsigned int i = 0; i < ptvec2.size(); i++ ) {
      draw_segment( oreg, scale, ptvec2[i].first, ptvec2[i].second, cx, cy );
    }
    */
  }
  /**/
  /*
  std::cout<<"spline_points.size(): "<<spline_points.size()<<std::endl;
  int xlast=-1, ylast=-1;
  int N = 0;
  for( unsigned int pi = 0; pi < spline_points.size(); pi++, N++ ) {
    //std::cout<<"point #"<<pi<<": "<<spline_points[pi].first<<" "<<spline_points[pi].second<<std::endl;

    int pi2 = pi - 1;
    if( pi2 < 0 ) pi2 = spline_points.size() - 1;

    double px = spline_points[pi].first*outline_scaling.first,
        py = spline_points[pi].second*outline_scaling.second;
    double px2 = spline_points[pi2].first*outline_scaling.first,
        py2 = spline_points[pi2].second*outline_scaling.second;
    int ipx = (int)px, ipy = (int)py;
    int ipx2 = (int)px2, ipy2 = (int)py2;

    double bpx = (px-cx) * scale + cx;
    double bpy = (py-cy) * scale + cy;
    double bpx2 = (px2-cx) * scale + cx;
    double bpy2 = (py2-cy) * scale + cy;
    int ibpx = (int)bpx, ibpy = (int)bpy;
    int ibpx2 = (int)bpx, ibpy2 = (int)bpy2;

    std::vector< std::pair<int,int> > ptvec2;
    get_line_points( ibpx, ibpy, ibpx2, ibpy2, ptvec2 );
    for( unsigned int i = 0; i < ptvec2.size(); i++ ) {
      draw_segment( oreg, scale, ptvec2[i].first, ptvec2[i].second, cx, cy );
    }
  }
  */

  return;
/*
  //std::cout<<"PathMask::render: height="<<height<<std::endl;

  std::pair<float,float> outline_scaling =
      par->get_smod().get_outline_scaling( width, height );

  std::pair<float,float> outline_center = par->get_smod().get_center();
  float cx = outline_center.first * width;
  float cy = outline_center.second * height;

  //const std::vector< std::pair<float,float> >& spline_points = par->get_smod().get_outline();
  std::vector< std::pair<float,float> > spline_points;
  std::pair<float,float> pt;

  pt.first = 0.2f*width/outline_scaling.first;
  pt.second = 0.2f*height/outline_scaling.second;
  spline_points.push_back(pt);
  pt.first = 0.8f*width/outline_scaling.first;
  pt.second = 0.1f*height/outline_scaling.second;
  spline_points.push_back(pt);
  pt.first = 0.8f*width/outline_scaling.first;
  pt.second = 0.8f*height/outline_scaling.second;
  spline_points.push_back(pt);
  pt.first = 0.2f*width/outline_scaling.first;
  pt.second = 0.8f*height/outline_scaling.second;
  spline_points.push_back(pt);

  //-----------------
  // draw outline
  std::cout<<"spline_points.size(): "<<spline_points.size()<<std::endl;
  int xlast=-1, ylast=-1;
  int N = 0;
  for( unsigned int pi = 0; pi < spline_points.size(); pi++, N++ ) {
    //std::cout<<"point #"<<pi<<": "<<spline_points[pi].first<<" "<<spline_points[pi].second<<std::endl;

    int pi2 = pi - 1;
    if( pi2 < 0 ) pi2 = spline_points.size() - 1;

    double px = spline_points[pi].first*outline_scaling.first,
        py = spline_points[pi].second*outline_scaling.second;
    double px2 = spline_points[pi2].first*outline_scaling.first,
        py2 = spline_points[pi2].second*outline_scaling.second;
    int ipx = (int)px, ipy = (int)py;
    int ipx2 = (int)px2, ipy2 = (int)py2;

    double bpx = (px-cx) * scale + cx;
    double bpy = (py-cy) * scale + cy;
    double bpx2 = (px2-cx) * scale + cx;
    double bpy2 = (py2-cy) * scale + cy;
    int ibpx = (int)bpx, ibpy = (int)bpy;
    int ibpx2 = (int)bpx, ibpy2 = (int)bpy2;

    //if( ibpx==xlast && ibpy==ylast ) continue;
    //xlast = ipx; ylast = ipy;

    /
    if( vips_rect_includespoint(r,ipx,ipy) ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, ipx, ipy );
      //pout[1] = FormatInfo<T>::MAX; pout[0] = 0; pout[2] = 0;
    }
    if( vips_rect_includespoint(r,ibpx,ibpy) ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, ibpx, ibpy );
      pout[2] = FormatInfo<T>::MAX; pout[0] = 0; pout[1] = 0;
    }
    if( vips_rect_includespoint(r,ipx_,ipy_) ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, ipx_, ipy_ );
      pout[0] = FormatInfo<T>::MAX; pout[1] = 0; pout[2] = 0;
    }
    //continue;
    /

    int left = r->left;
    int right = r->left + r->width - 1;
    int top = r->top;
    int bottom = r->top + r->height - 1;

    int dx = ipx2 - ipx;
    int dy = ipy2 - ipy;
    int xlast = -1, ylast = -1;
    if( (dx != 0) || (dy != 0) ) {
      if( dy == 0 ) {
        int ipxmin = MIN(ipx,ipx2);
        int ipxmax = MAX(ipx,ipx2);
        for( int x2 = ipxmin; x2 < ipxmax; x2++) {
          if( x2 == ipx2 ) continue;
          DRAW_OUTLINE_PX( x2, ipy );
        }
      } else {
        // walk along border segment
        int ipymin = MIN(ipy,ipy2);
        int ipymax = MAX(ipy,ipy2);
        for( int y2 = ipymin; y2 <= ipymax; y2++) {
          int x2 = ipx + (y2-ipy)*dx/dy;
          //int x2 = ipx + (y2-ipy)*dx/dy;
          DRAW_OUTLINE_PX( x2, y2 );
        }
      }
    }

    int bdx = ibpx2 - ibpx;
    int bdy = ibpy2 - ibpy;
    if( bdy == 0 ) {
      int ibpxmin = MIN(ibpx,ibpx2);
      int ibpxmax = MAX(ibpx,ibpx2);
      for( int x2 = ibpxmin; x2 <= ibpxmax; x2++) {
        //draw_segment( oreg, scale, x2, ibpy, cx, cy );
      }
    } else {
      // walk along border segment
      int ibpymin = MIN(ibpy,ibpy2);
      int ibpymax = MAX(ibpy,ibpy2);
      for( int y2 = ibpymin; y2 <= ibpymax; y2++) {
        int x2 = ibpx + (y2-ibpy)*bdx/bdy;
        //draw_segment( oreg, scale, x2, y2, cx, cy );
      }
    }
    //if( N>100 )break;
  }
*/
}




template< OP_TEMPLATE_DEF >
void PathMask< OP_TEMPLATE_IMP >::
draw_segment(VipsRegion* oreg, const falloff_segment& seg, float* vec)
{
  Rect *r = &oreg->valid;
  int bands = oreg->im->Bands;
  T* pout;
  int i, x, y, b;

  int left = r->left;
  int top = r->top;
  int right = r->left + r->width - 1;
  int bottom = r->top + r->height - 1;


  //std::cout<<"border: "<<x2<<","<<y2<<"  path: "<<x1<<","<<y1<<std::endl;

  //--------------------------
  // draw falloff segment

  float lx = seg.x2 - seg.x1;
  float ly = seg.y2 - seg.y1;

  for( i = seg.lmin; i <= seg.lmax; i++ ) {
    // position
    x = (int)((float)i * lx / seg.fl) + seg.x1;
    y = (int)((float)i * ly / seg.fl) + seg.y1;
    //std::cout<<"  x="<<x<<"  y="<<y<<std::endl;
    if( !vips_rect_includespoint(r,x,y) )
      continue;
    /*
    float op = 1.0 - (float)i / seg.fl;
    pout = (T*)VIPS_REGION_ADDR( oreg, x, y );
    //pout[0] = pout[2] = FormatInfo<T>::MIN; pout[1] = FormatInfo<T>::MAX;
    T val = (T)((float)FormatInfo<T>::RANGE*op + FormatInfo<T>::MIN);
    for( b = 0; b < bands; ++b) {
      //if(op>0.5)
        pout[b] = val;
    }
    */
    float op = (float)i / seg.fl;
    pout = (T*)VIPS_REGION_ADDR( oreg, x, y );
    T val; get_falloff_curve( vec, op, val );
    for( b = 0; b < bands; ++b)
      pout[b] = val;

    if( x > left ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, x-1, y );
      for( b = 0; b < bands; ++b) {
            pout[b] = val;
      }
    }
    if( y > top ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, x, y-1 );
      for( b = 0; b < bands; ++b) {
            pout[b] = val;
      }
    }
    /**/
    if( x < right ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, x+1, y );
      for( b = 0; b < bands; ++b) {
            pout[b] = val;
      }
    }
    if( y < bottom ) {
      pout = (T*)VIPS_REGION_ADDR( oreg, x, y+1 );
      for( b = 0; b < bands; ++b) {
            pout[b] = val;
      }
    }
    /**/
  }
}



  ProcessorBase* new_path_mask();
}

#endif 


