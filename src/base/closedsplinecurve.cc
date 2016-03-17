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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vips/vips.h>

#include "../dt/develop/masks.h"
#include "splinecurve.hh"

PF::ClosedSplineCurve::ClosedSplineCurve():
PF::Curve(),
Dmax(0), border_size(0),
wd_last(0),
ht_last(0),
ypp( NULL ),
ypp_size( 0 )
{
  points_mutex = vips_g_mutex_new();
  //points.push_back( std::make_pair(float(0.5),float(0.2)) );
  //points.push_back( std::make_pair(float(0.8),float(0.8)) );
  //points.push_back( std::make_pair(float(0.2),float(0.8)) );
  center.first = 0.5; center.second = 0.5;
  //update_polar();
  //update_spline();
}



PF::ClosedSplineCurve::~ClosedSplineCurve()
{
}



void PF::ClosedSplineCurve::update_polar()
{
  points_polar.clear();
  for( unsigned int i = 0; i < points.size(); i++ ) {
    float dx = points[i].first - center.first;
    float dy = points[i].second - center.second;
    float r = hypotf(dx,dy);
    float phi = atan2f(dy,dx);
    points_polar.push_back( std::make_pair(r,phi) );
  }
}



int PF::ClosedSplineCurve::add_point( unsigned int id, float x, float y )
{
  //if( (get_npoints()>0) && (x<=points[0].first) ) return -1;
  //if( (get_npoints()>1) && (x >= points[get_npoints()-1].first) ) return -1;
  //return -1;
  lock();
  if( id >= 0 ) {

    if( id < points.size() ) {
      points.insert( points.begin()+id, std::make_pair(x,y) );
#ifndef NDEBUG
      std::cout<<"PF::ClosedSplineCurve::add_point( "<<x<<", "<<y<<" ): point added before "<<points[i].first<<std::endl;
#endif
    } else {
      points.push_back( std::make_pair(x,y) );
    }
    update_polar();
    unlock();
    return id;
  } else {
    points.push_back( std::make_pair(x,y) );
  }
  unlock();
  return( points.size()-1 );


//#ifndef NDEBUG
  std::cout<<"PF::ClosedSplineCurve::add_point( "<<x<<", "<<y<<" ): points.size()="<<points.size()<<std::endl;
//#endif
  float dx = x - center.first;
  float dy = y - center.second;
  float r = hypotf(dx,dy);
  float phi = atan2f(dy,dx);
  std::cout<<"PF::ClosedSplineCurve::add_point( "<<x<<", "<<y<<" ): r="<<r<<" phi="<<phi<<std::endl;
  std::cout<<" phi[0]="<<points_polar[0].second<<std::endl;
  std::cout<<" phi["<<points_polar.size()-1<<"]="<<points_polar[points_polar.size()-1].second<<std::endl;
  int dest_id = -1;
  if( points_polar.size() == 0 || phi < points_polar[0].second )
    dest_id = 0;
  else if( phi >= points_polar[points_polar.size()-1].second )
    dest_id = points_polar.size();
  else {
    for( unsigned int i = 1; i < points_polar.size(); i++ ) {
      std::cout<<" phi["<<i<<"]="<<points_polar[i].second<<std::endl;
      if( (phi >= points_polar[i-1].second) && (phi < points_polar[i].second) ) {
        dest_id = i;
        break;
      }
    }
  }
  std::cout<<"PF::ClosedSplineCurve::add_point( "<<x<<", "<<y<<" ): dest_id="<<dest_id<<std::endl;

  if( dest_id >= 0 ) {

    if( dest_id < points.size() ) {
      points.insert( points.begin()+dest_id, std::make_pair(x,y) );
#ifndef NDEBUG
      std::cout<<"PF::ClosedSplineCurve::add_point( "<<x<<", "<<y<<" ): point added before "<<points[i].first<<std::endl;
#endif
    } else {
      points.push_back( std::make_pair(x,y) );
    }
    update_polar();
    unlock();
    return dest_id;
  }
  unlock();
  return( points.size()-1 );
}


bool PF::ClosedSplineCurve::remove_point( unsigned int id )
{
  if( id == 0 ) return false;
  if( id >= (get_npoints()-1) ) return false;

  points.erase( points.begin() + id );
  //update_spline();
  return true;
}


int PF::ClosedSplineCurve::set_point( unsigned int id, float x, float y )
{
  //return 0;
#ifndef NDEBUG
  std::cout<<"PF::ClosedSplineCurve::set_point( "<<x<<", "<<y<<" ): points.size()="<<points.size()<<std::endl;
#endif
  if( id >= points.size() ) return -1;
  points[id].first = x;
  points[id].second = y;
  return id;

  float dx = x - center.first;
  float dy = y - center.second;
  float r = hypotf(dx,dy);
  float phi = atan2f(dy,dx);
  std::cout<<"PF::ClosedSplineCurve::set_point( "<<x<<", "<<y<<" ): r="<<r<<" phi="<<phi<<std::endl;
  std::cout<<" phi[0]="<<points_polar[0].second<<std::endl;
  std::cout<<" phi["<<points_polar.size()-1<<"]="<<points_polar[points_polar.size()-1].second<<std::endl;
  int dest_id = -1;
  if( points_polar.size() == 0 || phi < points_polar[0].second )
    dest_id = 0;
  else if( phi >= points_polar[points_polar.size()-1].second )
    dest_id = points_polar.size();
  else {
    for( unsigned int i = 1; i < points_polar.size(); i++ ) {
      std::cout<<" phi["<<i<<"]="<<points_polar[i].second<<std::endl;
      if( (phi >= points_polar[i-1].second) && (phi < points_polar[i].second) ) {
        dest_id = i;
        break;
      }
    }
  }
  std::cout<<"PF::ClosedSplineCurve::set_point( "<<x<<", "<<y<<" ): dest_id="<<dest_id<<std::endl;

  if( dest_id < 0 ) return -1;

  int result_id = -1;

  std::vector< std::pair<float,float> > points2;
  std::vector< std::pair<float,float> > points_polar2;

  bool added = false;
  for( unsigned int i = 0; i < points_polar.size(); i++ ) {
    if( static_cast<int>(i) == dest_id ) {
      points2.push_back( std::make_pair(x,y) );
      points_polar2.push_back( std::make_pair(r,phi) );
      result_id = points2.size() - 1;
      added = true;
    }
    if( static_cast<int>(i) != id ) {
      points2.push_back( points[i] );
      points_polar2.push_back( points_polar[i] );
    }
  }
  if( !added ) {
    points2.push_back( std::make_pair(x,y) );
    points_polar2.push_back( std::make_pair(r,phi) );
    result_id = points2.size() - 1;
  }

  points_polar = points_polar2;
  points = points2;
  return result_id;
}



void PF::ClosedSplineCurve::scale( float s )
{
  update_center();
  for( unsigned int i = 0; i < points.size(); i++ ) {
    float dx = points[i].first - center.first;
    float dy = points[i].second - center.second;
    double bpx = dx * s + center.first;
    double bpy = dy * s + center.second;
    points[i].first = bpx;
    points[i].second = bpy;
  }
}



void PF::ClosedSplineCurve::update_spline()
{
}


void PF::ClosedSplineCurve::update_center()
{
  // get the center of gravity of the form (like if it was a simple polygon)
  float bx = 0.0f;
  float by = 0.0f;
  float surf = 0.0f;

  int nb = points.size();
  if( nb > 0 ) {
    for(int k = 0; k < nb; k++)
    {
      int k2 = (k + 1) % nb;
      surf += points[k].first * points[k2].second - points[k2].first * points[k].second;

      bx += (points[k].first + points[k2].first)
              * (points[k].first * points[k2].second - points[k2].first * points[k].second);
      by += (points[k].second + points[k2].second)
              * (points[k].first * points[k2].second - points[k2].first * points[k].second);
    }
    bx /= 3.0 * surf;
    by /= 3.0 * surf;
  }

  center.first = bx; center.second = by;
  Dmax = 0;
  for( unsigned int i = 0; i < points.size(); i++ ) {
    float px = points[i].first;
    float py = points[i].second;
    float dx = px - center.first;
    float dy = py - center.second;
    float D = hypotf(dx,dy);
    if( D > Dmax ) Dmax = D;
  }
}



#define CLIPD(a) ((a)>0.0?((a)<1.0?(a):1.0):0.0)

float PF::ClosedSplineCurve::get_value( float x )
{
}


float PF::ClosedSplineCurve::get_delta( float x )
{ 
  return( get_value(x) - x );
}


void PF::ClosedSplineCurve::get_values( std::vector< std::pair<float,float> >& vec )
{ 
  for (unsigned int i=0; i<vec.size(); i++)
    vec[i].second = get_value( vec[i].first );
}


void PF::ClosedSplineCurve::get_deltas( std::vector< std::pair<float,float> >& vec )
{ 
  for (unsigned int i=0; i<vec.size(); i++) {
    float val = get_value( vec[i].first );
    vec[i].second = val - vec[i].first;
  }
}


void PF::ClosedSplineCurve::update_outline( float wd, float ht )
{
  update_center();
  //float wd = 1000.0f/Dmax;
  //float ht = wd;
  //std::cout<<"ClosedSplineCurve::update_outline(): Dmax="<<Dmax<<"  wd="<<wd<<"  ht="<<ht<<"  border_size="<<border_size<<std::endl;
  dt_masks_form_t *form = (dt_masks_form_t *)malloc(sizeof(dt_masks_form_t));
  form->type = DT_MASKS_PATH;
  form->version = DEVELOP_MASKS_VERSION;
  form->formid = time(NULL);

  form->points = NULL;
  float masks_border = border_size;

  //printf("ClosedSplineCurve::update_outline(): points.size()=%d\n", (int)points.size());
  for( unsigned int pi = 0; pi < points.size(); pi++ ) {
    dt_masks_point_path_t *bzpt = (dt_masks_point_path_t *)(malloc(sizeof(dt_masks_point_path_t)));
    int nb = g_list_length(form->points);
    //printf("ClosedSplineCurve::get_points(): nb(1)=%d\n", nb);
    // change the values
    bzpt->corner[0] = points[pi].first;
    bzpt->corner[1] = points[pi].second;
    bzpt->ctrl1[0] = bzpt->ctrl1[1] = bzpt->ctrl2[0] = bzpt->ctrl2[1] = -1.0;
    bzpt->state = DT_MASKS_POINT_STATE_NORMAL;

    bzpt->border[0] = bzpt->border[1] = MAX(0.005f, masks_border);

    if( pi==0 ) {
      form->source[0] = bzpt->corner[0] + 0.02f;
      form->source[1] = bzpt->corner[1] + 0.02f;
    }

    // if that's the first point we should add another one as base point
    if(false && nb == 0)
    {
      dt_masks_point_path_t *bzpt2 = (dt_masks_point_path_t *)(malloc(sizeof(dt_masks_point_path_t)));
      bzpt2->corner[0] = points[pi].first;
      bzpt2->corner[1] = points[pi].second;
      bzpt2->ctrl1[0] = bzpt2->ctrl1[1] = bzpt2->ctrl2[0] = bzpt2->ctrl2[1] = -1.0;
      bzpt2->border[0] = bzpt2->border[1] = MAX(0.005f, masks_border);
      bzpt2->state = DT_MASKS_POINT_STATE_NORMAL;
      form->points = g_list_append(form->points, bzpt2);
      guint nb = g_list_length(form->points);
      //printf("ClosedSplineCurve::get_points(): nb(2)=%d\n", nb);
      form->source[0] = bzpt->corner[0] + 0.02f;
      form->source[1] = bzpt->corner[1] + 0.02f;
      nb++;
    }
    form->points = g_list_append(form->points, bzpt);
  }

  _path_init_ctrl_points(form);
  ctrl_points.clear();
  guint nb = g_list_length(form->points);
  for(int k = 0; k < nb; k++) {
    dt_masks_point_path_t *pt = (dt_masks_point_path_t *)g_list_nth_data(form->points, k);
    std::pair<float,float> ctrl1 = std::make_pair( (float)pt->ctrl1[0], (float)pt->ctrl1[1] );
    std::pair<float,float> ctrl2 = std::make_pair( (float)pt->ctrl2[0], (float)pt->ctrl2[1] );
    ctrl_points.push_back( std::make_pair(ctrl1, ctrl2) );
  }

  float* out_points = NULL;
  int out_points_count = 0;
  float* out_border = NULL;
  int out_border_count = 0;
  int result = _path_get_points_border(form, 999, wd, ht,
      &out_points, &out_points_count,
      &out_border, &out_border_count, 0);
  //std::cout<<"_path_get_points_border(): result="<<result<<std::endl;
  //std::cout<<"update_outline(): out_points_count="<<out_points_count
  //    <<"  out_border_count="<<out_border_count<<std::endl;

  outline.clear();
  float* ptr = out_points;
  int xlast = -999999, ylast = -999999;
  for( int pi = 0; pi < out_points_count; pi++ ) {
    if( pi >= points.size()*3 ) {
      int ix = (int)ptr[0];
      int iy = (int)ptr[1];
      //std::cout<<"path point "<<ix<<","<<iy<<"  (last "<<xlast<<","<<ylast<<")"<<std::endl;
      //if( ix!=xlast || iy!=ylast) {
        outline.push_back( std::make_pair(ptr[0], ptr[1]) );
        xlast = ix; ylast = iy;
        //std::cout<<"added path point "<<ptr[0]<<","<<ptr[1]<<std::endl;
      //}
    }
    ptr += 2;
  }
  if( out_points ) free( out_points );

  border.clear();
  ptr = out_border;
  xlast = -999999; ylast = -999999;
  for( int pi = 0; pi < out_border_count; pi++ ) {
    if( pi >= points.size()*3 ) {
      int ix = (int)ptr[0];
      int iy = (int)ptr[1];
      //std::cout<<"path point "<<ix<<","<<iy<<"  (last "<<xlast<<","<<ylast<<")"<<std::endl;
      //if( ix!=xlast || iy!=ylast) {
        border.push_back( std::make_pair(ptr[0], ptr[1]) );
        xlast = ix; ylast = iy;
        //std::cout<<"added path point "<<ptr[0]<<","<<ptr[1]<<std::endl;
      //}
    }
    ptr += 2;
  }
  if( out_border ) free( out_border );

  wd_last = wd; ht_last = ht;
}
