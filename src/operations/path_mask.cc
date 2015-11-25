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

#include "path_mask.hh"
#include "../base/processor.hh"

template<>
void PF::get_falloff_curve<unsigned short int>( float* vec, float val, unsigned short int& out ) {
  out = static_cast<unsigned short int>( vec[static_cast<unsigned short int>(val*65535)] * 65535 );
}


template<>
void PF::get_falloff_curve<float>( float* vec, float val, float& out ) {
  out = vec[static_cast<unsigned short int>(val*65535)];
}


void PF::get_line_points(int x1, int y1, int x2, int y2, std::vector< std::pair<int,int> >& points)
{
  float xdiff = (x2 - x1);
  float ydiff = (y2 - y1);

  if( false && y1<20 )
    std::cout<<"get_line_points("<<x1<<","<<y1<<","<<x2<<","<<y2<<")"<<std::endl;

  if(xdiff == 0.0f && ydiff == 0.0f) {
    points.push_back( std::make_pair(x1, y1) );
    return;
  }
  if(fabs(xdiff) > fabs(ydiff)) {
    //std::cout<<"ydiff="<<ydiff<<std::endl;
    float xmin, xmax;

    // set xmin to the lower x value given
    // and xmax to the higher value
    if(x1 < x2) {
      xmin = x1;
      xmax = x2;
    } else {
      xmin = x2;
      xmax = x1;
    }

    // draw line in terms of y slope
    float slope = ydiff / xdiff;
    float x = x1;
    float incr = (xdiff>0) ? 1.0f : -1.0f;
    while( x != x2) {
      float y = y1 + ((x - x1) * slope);
      points.push_back( std::make_pair((int)rint(x), (int)rint(y)) );
      if( false && y1<20 )
        std::cout<<"  added point "<<(int)rint(x)<<","<<(int)rint(y)<<std::endl;
      x += incr;
    }
    //points.push_back( std::make_pair(x2, y2) );
  } else {
    float ymin, ymax;

    // set ymin to the lower y value given
    // and ymax to the higher value
    if(y1 < y2) {
      ymin = y1;
      ymax = y2;
    } else {
      ymin = y2;
      ymax = y1;
    }

    // draw line in terms of x slope
    float slope = xdiff / ydiff;
    for(int y = ymin; y <= ymax; y += 1) {
      float x = x1 + ((y - y1) * slope);
      points.push_back( std::make_pair((int)rint(x), (int)rint(y)) );
    }
    //points.push_back( std::make_pair(x2, y2) );
  }
}


void PF::init_path_segment( PF::falloff_segment& seg, int x1, int y1, int x2, int y2, int ymin, int ymax )
{
  seg.x1 = x1; seg.y1 = y1; seg.x2 = x2; seg.y2 = y2;
  seg.fl = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) + 1;
  seg.l = (int)seg.fl;
  float lx = x2-x1;
  float ly = y2-y1;

  if( ly == 0 ) {
    seg.lmin = 0;
    seg.lmax = seg.l;
  } else {
    if( (y1 < ymin) ) {
      int x = x1 + ( lx * (ymin - y1) ) / ly;
      seg.xmin = x;
      seg.lmin = sqrt((x - x1) * (x - x1) + (ymin - y1) * (ymin - y1)) - 1;
    } else if( y1 > ymax ) {
      int x = x1 + ( lx * (ymax - y1) ) / ly;
      seg.xmin = x;
      seg.lmin = sqrt((x - x1) * (x - x1) + (ymax - y1) * (ymax - y1)) - 1;
    } else {
      seg.xmin = x1;
      seg.lmin = 0;
    }

    if( (y2 < ymin) ) {
      int x = x1 + ( lx * (ymin - y1) ) / ly;
      seg.xmax = x;
      seg.lmax = sqrt((x - x1) * (x - x1) + (ymin - y1) * (ymin - y1)) + 1;
    } else if( y2 > ymax ) {
      int x = x1 + ( lx * (ymax - y1) ) / ly;
      seg.xmax = x;
      seg.lmax = sqrt((x - x1) * (x - x1) + (ymax - y1) * (ymax - y1)) + 1;
    } else {
      seg.xmax = x2;
      seg.lmax = seg.l;
    }
  }
}



static void get_falloff_segments( const std::vector< std::pair<int,int> >& points,
    const std::vector< std::pair<int,int> >& border,
    std::vector< PF::falloff_segment >& segments )
{
  int p0[2], p1[2];
  int last0[2] = { -100, -100 }, last1[2] = { -100, -100 };
  //nbp = 0;
  int next = 0;
  for(int i = 0; i < border.size(); i++) {
    p0[0] = points[i].first, p0[1] = points[i].second;
    if(next > 0)
      p1[0] = border[next].first, p1[1] = border[next].second;
    else
      p1[0] = border[i].first, p1[1] = border[i].second;

    // now we check p1 value to know if we have to skip a part
    if(next == i) next = 0;
    while(p1[0] == -999999) {
      if(p1[1] == -999999)
        next = i - 1;
      else
        next = p1[1];
      p1[0] = border[next].first, p1[1] = border[next].second;
    }

    // and we add the segment
    if(last0[0] != p0[0] || last0[1] != p0[1] || last1[0] != p1[0] || last1[1] != p1[1]) {
      //std::cout<<"adding segment: "<<p0[0]<<","<<p0[1]<<" -> "<<p1[0]<<","<<p1[1]<<std::endl;
      //_path_falloff(buffer, p0, p1, *posx, *posy, *width);
      PF::falloff_segment s;
      s.x1 = p0[0]; s.y1 = p0[1]; s.x2 = p1[0]; s.y2 = p1[1];
      segments.push_back( s );
      last0[0] = p0[0], last0[1] = p0[1];
      last1[0] = p1[0], last1[1] = p1[1];
    }
  }
}


PF::PathMaskPar::PathMaskPar():
                OpParBase(),
                invert("invert",this,false),
                enable_falloff("enable_falloff",this,true),
                falloff_curve( "falloff_curve", this ), // 0
                smod( "smod", this ),
                border_size( "border_size", this, 0.05 ),
                modvec( NULL ), edgevec( NULL ), segvec( NULL )
{
  float x1 = 0.0, y1 = 1.0, x2 = 1.0, y2 = 0.0;
  falloff_curve.get().set_point( 0, x1, y1 );
  falloff_curve.get().set_point( 1, x2, y2 );
  falloff_curve.store_default();

  smod.store_default();

  //const std::vector< std::pair<float,float> >& points = smod.get().get_points();
  //std::pair<float,float> center = smod.get().get_center();

  set_type( "path_mask" );
  set_default_name( _("path mask") );
}



VipsImage* PF::PathMaskPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* out = PF::OpParBase::build( in, first, imap, omap, level );

  int tw = 64, th = 64, nt = out->Xsize*2/tw;

  falloff_curve.get().lock();
  if( falloff_curve.is_modified() || invert.is_modified() ) {
    std::cout<<"PathMaskPar::build(): updating falloff LUT"<<std::endl;
    for(int i = 0; i <= FormatInfo<unsigned short int>::RANGE; i++) {
      float x = ((float)i)/FormatInfo<unsigned short int>::RANGE;
      float y = falloff_curve.get().get_value( x );
      falloff_vec[i] = invert.get() ? 1.0f-y : y;
    }
  }
  falloff_curve.get().unlock();

  bool path_modified = smod.is_modified();
  if( get_smod().get_border_size() != border_size.get() ) {
    get_smod().set_border_size( border_size.get() );
    path_modified = true;
  }
  if( path_modified || (out->Xsize != get_smod().get_wd_last()) ||
      (out->Ysize != get_smod().get_ht_last()) ) {
    get_smod().update_outline( out->Xsize, out->Ysize );
    path_modified = true;
  }

  const std::vector< std::pair<float,float> >& points = smod.get().get_points();
  int ps = points.size();

  if( path_modified ) {
    std::cout<<"PathMaskPar::build(): updating path"<<std::endl;
    if( edgevec ) delete[] edgevec;
    edgevec = new std::vector<float>[out->Ysize];

    const std::vector< std::pair<int,int> >& spline_points  = get_smod().get_outline();
    const std::vector< std::pair<int,int> >& spline_points2 = get_smod().get_border();

    std::pair<float,float> outline_center = get_smod().get_center();
    float bx = outline_center.first * out->Xsize;
    float by = outline_center.second * out->Ysize;

    ptvec.clear();
    //std::cout<<"spline_points.size():  "<<spline_points.size()<<std::endl;
    //std::cout<<"spline_points2.size(): "<<spline_points2.size()<<std::endl;
    int xlast=-1, ylast=-1;
    for( int pi = 0; pi < spline_points.size(); pi++ ) {
      double px = spline_points[pi].first, py = spline_points[pi].second, pw = 1, ph = 1;

      int ipx = px, ipy = py;
      //if( ipx==xlast && ipy==ylast ) continue;
      xlast = ipx; ylast = ipy;

      int pi2 = pi + 1;
      if( pi2 >= spline_points.size() ) pi2 = 0;

      double px2 = spline_points[pi2].first, py2 = spline_points[pi2].second, pw2 = 1, ph2 = 1;

      int ipx2 = px2, ipy2 = py2;

      if( (ipx==ipx2) && (ipy==ipy2) ) continue;

      get_line_points( ipx, ipy, ipx2, ipy2, ptvec );
      std::vector< std::pair<int,int> > ptvec2;
      get_line_points( ipx, ipy, ipx2, ipy2, ptvec2 );
      for( unsigned int i = 0; i < ptvec2.size(); i++ ) {
        int x = ptvec2[i].first;
        int y = ptvec2[i].second;
        if( (y<0) || (y>=out->Ysize) ) continue;
        if( ipy < 20 ) {
          //std::cout<<"spline point "<<x<<","<<y<<std::endl;
        }
        int idx = -1;
        for( unsigned int j = 0; j < edgevec[y].size(); j++ ) {
          if( edgevec[y][j] >= x ) {
            idx = j; break;
          }
        }
        if( idx < 0 ) edgevec[y].push_back( x );
        else {
          if( edgevec[y][idx] > x )
            edgevec[y].insert( edgevec[y].begin()+idx, x );
        }
      }
    }

    /*
  for( int y = 0; y < 64; y++ ) {
    std::cout<<"y="<<y;
    for(int j=0; j<edgevec[y].size(); j++) {
      std::cout<<"  "<<edgevec[y][j];
    }
    std::cout<<std::endl;
  }
     */

    for( int y = 0; y < out->Ysize; y++ ) {
      if( edgevec[y].empty() ) edgevec[y].push_back( out->Xsize );
    }

    std::vector< PF::falloff_segment > segments;
    get_falloff_segments( spline_points, spline_points2, segments );

    if( segvec ) delete[] segvec;
    segvec = new std::vector< PF::falloff_segment >[out->Ysize/tw+1];

    for( int y = 0, si = 0; y < out->Ysize; y+=tw, si++ ) {
      //std::cout<<"Filling segment "<<si<<"  ("<<y<<" -> "<<y+tw<<")"<<std::endl;
      for( int i = 0; i < segments.size(); i++ ) {
        int ipx = segments[i].x1;
        int ipy = segments[i].y1;
        int ibpx = segments[i].x2;
        int ibpy = segments[i].y2;

        bool crossing = true;
        if( (ipy < y) && (ibpy < y) ) crossing = false;
        if( (ipy >= (y+tw)) && (ibpy >= (y+tw)) ) crossing = false;

        //std::cout<<"y="<<y<<"->"<<y+tw-1<<"  ipy="<<ipy<<"  ibpy="<<ibpy<<"  crossing="<<crossing<<std::endl;

        if( crossing ) {
          //std::cout<<"  crossing segment found: "<<ipx<<","<<ipy<<" -> "<<ibpx<<","<<ibpy<<std::endl;
          PF::falloff_segment& seg = segments[i];
          PF::init_path_segment( seg, ipx, ipy, ibpx, ibpy, y, y+tw-1 );
          segvec[si].push_back( seg );
        }
      }
      //std::cout<<"segvec["<<si<<"].size()="<<segvec[si].size()<<std::endl;
    }
  }

  VipsImage* cached;
  VipsAccess acc = VIPS_ACCESS_RANDOM;
  bool threaded = true, persistent = false;
  if( vips_tilecache(out, &cached,
      "tile_width", tw, "tile_height", th, "max_tiles", nt,
      "access", acc, "threaded", threaded, "persistent", persistent, NULL) ) {
    std::cout<<"PathMaskPar::build(): vips_tilecache() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( out, "GaussBlurPar::build(): iter_in unref" );

  return cached;
}


PF::ProcessorBase* PF::new_path_mask()
{
  return( new PF::Processor<PF::PathMaskPar,PF::PathMask>() );
}


